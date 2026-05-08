#include <atomic>
#include <csignal>
#include <iostream>
#include <mungo/mungo.hpp>
#include <thread>

std::atomic_bool is_running = true;

void signal_handler(int) { is_running = false; }

int main(int, char**) {
  std::signal(SIGINT, signal_handler);

  mungo::app server;
  server.setup(
      {
          .unsafe_host = "localhost:80",
          /*
            .safe_host = "localhost:443",
            .cert = "cert.pem",
            .key = "key.pem",
          */
      },
      [](const std::string_view msg) { std::cout << "[mungo] " << msg; });

  server.use_pool([](auto task) {
    std::thread([task = std::move(task)] { task(); }).detach();
  });

  server
      .get("/",
           [](const mungo::request& req, const mungo::response& res) {
             const std::string body = std::format(
                 "Hello {}!\n"
                 "API endpoints you can try:\n\n"
                 "- GET /api/users\n"
                 "- GET /api/users/:id\n"
                 "- POST /api/users\n"
                 "- DELETE /api/users/:id",
                 req.remote_ip());

             res.ok(body);
           })
      .get("/api/users",
           [](const mungo::request&, const mungo::response& res) {
             res.ok("[{id: 42, username: \"Mungo\"}]");
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

             res.ok("{id: 42, username: \"Mungo\"}");
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
    server.poll(100);
  }

  return 0;
}