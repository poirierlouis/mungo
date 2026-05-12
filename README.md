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

## Quick Start

The following example shows how to set up a basic server with a fake thread pool
and route handlers.

```cpp
#include <iostream>

#include <BS_thread_pool.hpp>
#include <mungo/mungo.hpp>

std::atomic_bool is_running;

void signal_handler(int) {
  std::cout << "[mungo] shutting down...\n";
  is_running = false;
}

int main() {
  std::signal(SIGINT, &signal_handler);

  mungo::app server;

  // 1. Setup the server with HTTP only
  constexpr auto host = "localhost:4200";
  const bool is_listening = server.setup(
    {.unsafe_host = host},
    [](const std::string_view msg) { std::cout << "[mungo] " << msg; }
  );

  if (!is_listening) {
    std::cerr << "Failed to listen on " << host << std::endl;
    return 1;
  }

  // 2. Tell mungo how to dispatch tasks using a thread pool
  BS::thread_pool pool(std::thread::hardware_concurrency());
  server.use_pool([&pool](auto task) {
    pool.detach_task(std::move(task));
  });

  // 3. Define routes
  server.get("/",
             // lambda handler is executed in the thread pool.
             [](const mungo::request& req, const mungo::response& res) {
    res.header("Content-Type", "text/plain")
       .ok(std::format("Hello {}!", req.get_remote_ip()));
  });

  server.get("/api/users/:id",
             [](const mungo::request& req, const mungo::response& res) {
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

  // 4. Run the polling loop
  is_running = true;
  while (is_running) {
    server.poll(1);
  }

  return 0;
}
```

For more detailed usage, see `examples/` directory.