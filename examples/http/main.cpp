#include <BS_thread_pool.hpp>
#include <atomic>
#include <csignal>
#include <iostream>
#include <mungo/mungo.hpp>
#include <thread>

constexpr auto html = R"(<!DOCTYPE html>
<html lang="en">
<head>
  <meta charset="utf-8">
  <title>mungo &middot; HTTP example</title>
  <style>
    html, body {{
      width: 100%;
      height: 100%;
      margin: 0;
      padding: 0;
    }}

    body {{
      display: flex;
      flex-flow: column;
      justify-content: center;
      align-items: center;

      font-family: monospace, sans-serif;
      color: #d0d0d0;
      background-color: #212121;
    }}

    a {{
      color: unset;
      text-decoration: none;
    }}

    ul {{
      padding-left: 16px;
      list-style-type: none;
    }}

    li {{
      margin: 4px 0;
      padding: 4px;

      border: 1px solid #616161;
      border-radius: 4px;
    }}

    span.method {{
      display: inline-block;
      min-width: 58px;
      padding: 0 6px;

      font-weight: bold;
      border-radius: 4px;

      &.get {{ color: #5da4ff; }}
      &.post {{ color: #ffd75d; }}
      &.delete {{ color: #ff5d5d; }}
    }}
  </style>
</head>

<body>
  <h1>Hello {}!</h1>

  <p>You can access the following API endpoints:</p>

  <ul>
    <li><a href="/api/users"><span class="method get">GET</span> <span>/api/users</span></a></li>
    <li><a href="/api/users/42"><span class="method get">GET</span> <span>/api/users/:id</span></a></li>
    <li><span class="method post">POST</span> <span>/api/users</span></li>
    <li><span class="method delete">DELETE</span> <span>/api/users/:id</span></li>
  </ul>
</body>
</html>)";

std::atomic_bool is_running = true;

void signal_handler(int) { is_running = false; }

int main(int, char**) {
  std::signal(SIGINT, signal_handler);

  mungo::app server;

  constexpr auto host = "localhost:4200";
  const auto is_listening = server.setup(
      {.unsafe_host = host},
      [](const std::string_view msg) { std::cout << "[mungo] " << msg; });
  if (!is_listening) {
    std::cerr << "[mungo] Failed to listen on http://" << host << '\n';
    return 1;
  }

  BS::thread_pool pool(std::thread::hardware_concurrency());
  server.use_pool([&pool](auto task) {
    pool.detach_task(std::move(task));
  });

  server
      .get("/",
           [](const mungo::request& req, const mungo::response& res) {
             std::string body = std::format(html, req.remote_ip());

             res.header("Content-Type", "text/html").ok(std::move(body));
           })
      .get("/api/users",
           [](const mungo::request&, const mungo::response& res) {
             res.header("Content-Type", "application/json")
                 .ok(R"([{"id": 42, "username": "Mungo"}])");
           })
      .get("/api/users/:id",
           [](const mungo::request& req, const mungo::response& res) {
             const auto id = req.param<uint64_t>("id");
             if (!id) {
               res.bad_request("Invalid user ID");
               return;
             }

             if (id != 42) {
               res.not_found();
               return;
             }

             res.header("Content-Type", "application/json")
                 .ok(R"({"id": 42, "username": "Mungo"})");
           })
      .post("/api/users",
            [](const mungo::request&, const mungo::response& res) {
              res.created("I'm a fake!");
            })
      .del("/api/users/:id",
           [](const mungo::request& req, const mungo::response& res) {
             const auto id = req.param<uint64_t>("id");
             if (!id) {
               res.bad_request("Invalid user ID");
               return;
             }

             if (id != 42) {
               res.not_found();
               return;
             }

             res.no_content();
           });

  // server.ws("/ws", [](const mungo::websocket& ws) {});

  while (is_running) {
    server.poll(1);
  }

  return 0;
}