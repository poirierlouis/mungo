# mungo

![version](https://img.shields.io/badge/version-work%20in%20progress-orange)

**mungo** is a modern C++20 HTTP framework designed to be as agnostic as
possible. It leverages [mgxx](https://github.com/poirierlouis/mgxx) (a C++
wrapper for [mongoose](https://github.com/cesanta/mongoose)) as a robust backend
for handling HTTP and HTTPS communication, providing a clean and expressive API
for web development.

## Features

- **Express-like Routing**: intuitive API for defining routes (`get`, `post`, 
  `put`, `patch`, `del`).
- **Dynamic Route Parameters**: support for named parameters in routes (e.g., 
  `/api/users/:id`).
- **Middleware Support**: flexible mechanism to handle requests before and after
  route handlers.
- **Compile-time Routers**: group endpoints together with automatic middleware
  inheritance and URI resolution.
- **Custom attributes**: attach custom attributes to requests using middlewares.
- **Agnostic Backend**: minimal abstractions over the underlying `mgxx` backend
  while maintaining high flexibility.
- **Async Execution**: built-in support for asynchronous request handling,
  easily integrable with external thread pools (like `BS::thread_pool`).
- **TLS/mTLS Support**: simplified configuration for secure HTTPS connections
  and mutual TLS authentication.
- **Automatic HTTPS Redirection**: built-in redirection from insecure HTTP to
  secure HTTPS.
- **Modern C++ API**: uses C++20 features for a type-safe and efficient
  developer experience.

## Usage

The following example shows how to set up a basic server with
[BS::thread_pool](https://github.com/bshoshany/thread-pool) and route handlers.

### Quick start

You can setup an HTTP only server and provide a callback to handle log messages:
```cpp
#include <iostream>

#include <BS_thread_pool.hpp>
#include <mungo/mungo.hpp>

int main() {
  mungo::app server;

  constexpr auto host = "localhost:4200";
  const auto is_listening = server.setup(
    {.unsafe_host = host},
    [](const std::string_view msg) { std::cout << "[mungo] " << msg; }
  );

  if (!is_listening) {
    std::cerr << "Failed to listen on " << host << std::endl;
    return 1;
  }
  
  // ...
}
```

#### Asynchronous thread-safe execution

You can configure the server to use a thread pool for asynchronous request
handling:
```cpp
  // ...

  BS::thread_pool pool(std::thread::hardware_concurrency());
  server.use_pool([&pool](auto task) {
    pool.detach_task(std::move(task));
  });

  // ...
```

#### Route handlers

You can define routes and handle requests:
```cpp
  server.get<"/">(// lambda is executed as a task of the thread pool.
                  [](const mungo::request& req, mungo::response& res) {
    res.ok(std::format("Hello {}!", req.get_remote_ip()));
  });

  server.post<"/">([](const mungo::request& req, mungo::response& res) {
    res.header("Content-Type", "application/json")
       .created(R"({"dummy": "fake"})");
  });
```

#### Named parameters

You can declare named parameters to quickly access values from the URI:
```cpp
  // ...

  server.get<"/api/users/:id">([](const mungo::request& req,
                                  mungo::response& res) {
    const auto id = req.param<uint64_t>("id");
    if (!id) {
      res.bad_request("Invalid user ID");
      return;
    }

    if (*id == 0) {
      res.not_found();
      return;
    }

    res.header("Content-Type", "application/json")
       .ok(std::format(R"({{"id": {}, "username": "mungo"}})", *id));
  });

  // ...
```

#### Polling loop

You must run the server in a loop as it is event-driven:
```cpp
  // ...

  while (true) {
    server.poll(100);
  }

  return 0;
}
```

### HTTPS

You can configure the server to use TLS by providing:
- paths of public certificate and private key files
- unsafe host of the server to listen on (HTTP)
- safe host of the server to listen on (HTTPS)
```cpp
  // ...

  const auto is_listening = server.setup(
    {
      .unsafe_host = "localhost:80",
      .safe_host = "localhost:443",
      .cert = "path/to/server.crt",
      .key = "path/to/server.key",
    },
    [](const std::string_view msg) { std::cout << "[mungo] " << msg; }
  );

  // ...
```

It will automatically redirect HTTP requests to HTTPS using a 
`301 Moved Permanently` status code.

### mTLS

You can configure the server to use two-way TLS by providing:
- path of a certificate authority file
- same as HTTPS above
```cpp
  // ...

  const auto is_listening = server.setup(
    {
      .unsafe_host = "0.0.0.0:80",
      .safe_host = "0.0.0.0:443",
      .ca = "path/to/root.pem",
      .cert = "path/to/server.crt",
      .key = "path/to/server.key",
    },
    [](const std::string_view msg) { std::cout << "[mungo] " << msg; }
  );

  // ...
```

You can access the client's certificate info from the request:
```cpp
  // ...

  server.get<"/">([](const mungo::request& req, mungo::response& res) {
    if (!req.is_mtls()) {
      res.unauthorized("Missing client certificate");
      return;
    }

    const auto& cert = req.tls_cert_info();
    res.header("Content-Type", "application/json")
       .ok(std::format(R"({{"subject": "{}", "serial_no": "{}"}})",
                       cert.get_subject_name(), cert.get_serial_number()));
  });

  // ...
```

### Middlewares / Interceptors / Filters
You can create middlewares to handle a request before and after a route handler.

In your middleware, you can call `next` with `req` and `res` objects to run the
next handler in the chain.
```cpp
// Add type ids to declare and use your middlewares (compile-time).
struct mw_logger {};
struct mw_auth {};

int main() {
  // ...

  server.use_middleware<mw_logger>([](mungo::request& req,
                                      mungo::response& res, auto next) {
    std::cout << "[mungo] " << req.method() << " " << req.path() << std::endl;
    next(req, res);
  });

  server.use_middleware<mw_auth>([](mungo::request& req,
                                    mungo::response& res, auto next) {
    const auto auth = req.header("Authorization").value_or("");
    if (auth != "Bearer c2VjcmV0") {
      res.unauthorized("You must login");
      return;
    }

    next(req, res);
  });

  server.get<"/", mw_logger>([](const mungo::request&, mungo::response& res) {
    res.ok();
  }

  server.post<"/api", mw_logger, mw_auth>([](const mungo::request&,
                                             mungo::response& res) {
    res.ok("Access granted");
  }

  // ...
```

### Routers

You can create routers to group endpoints together. It will automatically 
include middlewares of the parent router. URI is resolved at compile-time.

```cpp
  // ...

  const auto api = server.router<"/api", mw_auth>();

  auto users = api.router<"/users", mw_auth_users>();
  users
      .get<"/">([](const mungo::request& req, mungo::response& res) {
        res.ok();
      })
      .get<"/:id">([](const mungo::request& req, mungo::response& res) {
        res.ok();
      })
      .post<"/">([](const mungo::request& req, mungo::response& res) {
        res.created("R({"id": null})");
      })
      .del<"/:id">([](const mungo::request& req, mungo::response& res) {
        res.no_content();
      });

  // ...
```

It will always strip slashes at the end of a URI. It means that `/api//` will
become `/api`. This rule is applied when using `router<>` and HTTP methods
`get<>`, `post<>`, `put<>`, `patch<>`, `del<>`. Finally, when you only define a
single slash like `get<"/">` it will be treated as `get<"">` under the hood.

> [!NOTE]
> It is currently impossible to disable parent middlewares. This might be
> introduced later using a `mungo::not<>` template type.

### Attributes

You can attach custom attributes to requests using middlewares. You can declare
as many attributes as you want. You need to provide a specialization of your
attributes to be used by the framework. Access to an attribute is `O(1)` thanks
to compile-time type-safe lookup. A request will only grow in memory as you
attach attributes.
```cpp
// Include customization point header.
#include <mungo/attributes.hpp>

struct user_session {
  std::string email;
};

// Inject types into mungo's attribute system BEFORE including
// <mungo/mungo.hpp>. This specialization is picked up when the `mungo::request`
// alias is resolved.
template <>
struct mungo::custom_attrs<> {
  using type = std::variant<user_session>;
};

// Include mungo header.
#include <mungo/mungo.hpp>

struct mw_auth {};

int main() {
  // ...

  server.use_middleware<mw_auth>([](mungo::request& req,
                                    mungo::response& res, auto next) {
    req.attr<user_session>("mungo@mungo.com");
    next(req, res);

    // You would normally extract some data from a header, cross-check with a
    // database, validate credentials, set the attribute and keep on.
  });

  server.get<"/", mw_auth>([](const mungo::request& req,
                              mungo::response& res) {
    const auto user = req.attr<user_session>();
    if (!user) {
      // You should not even need to check for the attribute presence as long as
      // your middleware cover all cases.
      res.unauthorized("Missing user session");
      return;
    }

    res.ok(std::format("User is {}", user->email));
  });

  // ...
```

For more, see `examples/` directory.