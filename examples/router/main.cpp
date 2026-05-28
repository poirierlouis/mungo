#include <BS_thread_pool.hpp>
#include <atomic>
#include <chrono>
#include <csignal>
#include <iostream>
#include <mungo/mungo.hpp>

struct mw_admin {};

std::atomic_bool is_running = true;

void handle_signal(int) { is_running = false; }

int main(int, char**) {
  std::signal(SIGINT, handle_signal);

  mungo::app server;
  const auto is_listening = server.setup(
      {.unsafe_host = "localhost:4200"},
      [](const std::string_view msg) { std::cout << "[mungo] " << msg; });
  if (!is_listening) {
    std::cerr << "[mungo] Failed to listen on http://localhost:4200\n";
    return 1;
  }

  BS::thread_pool pool(std::thread::hardware_concurrency());
  server.use_pool([&pool](auto task) { pool.detach_task(std::move(task)); });

  server.use_middleware<mw_admin>(
      [](const mungo::request& req, mungo::response& res, auto next) {
        const auto auth = req.header("Authorization");
        if (!auth) {
          res.header("WWW-Authenticate",
                     R"(Basic realm="mungo server", charset="UTF-8")")
              .unauthorized();
          return;
        }

        const auto credentials = mungo::internal::base64_encode("mungo:secret");
        if (*auth != std::format("Basic {}", credentials)) {
          res.header("WWW-Authenticate",
                     R"(Basic realm="mungo server", charset="UTF-8")")
              .unauthorized();
          return;
        }

        next(req, res);
      });

  const auto api = server.router<"/api", mw_admin>();

  auto users = api.router<"/users">();
  users
      .get<"/">([](const mungo::request&, mungo::response& res) {
        res.ok(R"([{"id": 42, "username": "test"}])");
      })
      .get<"/:id">([](const mungo::request& req, mungo::response& res) {
        res.ok(std::format(R"({{"id": {}, "username": "test"}})",
                           req.param<uint32_t>("id").value_or(0)));
      });

  auto health = server.router<"/health">();
  health
      .get<"/">([](const mungo::request&, mungo::response& res) {
        res.header("Content-Type", "application/json")
            .ok(R"({"service": "all", "status": "ok"})");
      })
      .get<"/api">([](const mungo::request&, mungo::response& res) {
        res.header("Content-Type", "application/json")
            .ok(R"({"service": "api", "status": "ok"})");
      })
      .get<"/llm">([](const mungo::request&, mungo::response& res) {
        res.header("Content-Type", "application/json")
            .ok(R"({"service": "llm", "status": "ok"})");
      });

  while (is_running) {
    server.poll(100);
  }

  return 0;
}