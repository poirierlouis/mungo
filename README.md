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
  server.get("/",
             // lambda is executed as a task of the thread pool.
             [](const mungo::request& req, mungo::response& res) {
    res.header("Content-Type", "text/plain")
       .ok(std::format("Hello {}!", req.get_remote_ip()));
  });

  server.post("/",
              [](const mungo::request& req, mungo::response& res) {
    res.header("Content-Type", "application/json")
       .created(R"({"dummy": "fake"})");
  }
```

#### Named parameters

You can declare named parameters to quickly access values from the URI:
```cpp
  // ...

  server.get("/api/users/:id",
             [](const mungo::request& req, mungo::response& res) {
    const auto id = req.param<uint64_t>("id");
    if (!id) {
      res.bad_request("Missing or invalid user ID");
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

**TODO:** add getter to client's certificate info in `mungo::request`.

For more, see `examples/` directory.