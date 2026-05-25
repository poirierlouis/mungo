#include <BS_thread_pool.hpp>
#include <atomic>
#include <chrono>
#include <csignal>
#include <iostream>
#include <mungo/mungo.hpp>

constexpr auto k_serial_no = "541D7C457C0F15BDC2A48A2B81CA106AA8D667B3";
constexpr auto html_anonymous = R"(<!DOCTYPE html>
<html lang="en">
<head>
  <meta charset="utf-8">
  <title>mungo &middot; HTTPs example</title>
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
  </style>
</head>

<body>
  <h1>Hello {}!</h1>
</body>
</html>)";
constexpr auto html_auth = R"(<!DOCTYPE html>
<html lang="en">
<head>
  <meta charset="utf-8">
  <title>mungo &middot; HTTPs example</title>
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
  </style>
</head>

<body>
  <h1>Hello {} :)</h1>
  <h2>Serial Number: {}</h2>
  <h3>You have root access!</h3>
</body>
</html>)";

std::atomic_bool is_running = true;

void handle_signal(int) { is_running = false; }

struct mw_auth_mtls {};

int main(int, char**) {
  std::signal(SIGINT, handle_signal);

  const auto build = std::filesystem::absolute("../../../");

  mungo::app server;
  const auto is_listening = server.setup(
      {.unsafe_host = "localhost:4080",
       .safe_host = "localhost:4443",
       .ca = build / "root.pem",
       .cert = build / "server.crt",
       .key = build / "server.key"},
      [](const std::string_view msg) { std::cout << "[mungo] " << msg; });
  if (!is_listening) {
    std::cerr << "[mungo] Failed to listen on https://localhost:4443\n";
    return 1;
  }

  BS::thread_pool pool(std::thread::hardware_concurrency());
  server.use_pool([&pool](auto task) { pool.detach_task(std::move(task)); });

  server.use_middleware<mw_auth_mtls>(
      [](const mungo::request& req, mungo::response& res, auto next) {
        if (!req.is_mtls()) {
          res.unauthorized("Missing client certificate");
          return;
        }

        const auto& cert = req.tls_cert_info();
        if (cert.get_subject_name() != "CN=mgxx-client" ||
            cert.get_serial_number() != k_serial_no) {
          res.unauthorized("Bad credentials");
          return;
        }

        next(req, res);
      });

  server.get("/", [](const mungo::request& req, mungo::response& res) {
    const auto& cert = req.tls_cert_info();
    if (cert.get_subject_name() == "CN=mgxx-client" &&
        cert.get_serial_number() == k_serial_no) {
      res.header("Location", "/app")
          .send(mungo::status_code::temporary_redirect);
      return;
    }

    res.header("Content-Type", "text/plain")
        .ok(std::format(html_anonymous, req.remote_ip()));
  });

  server.get<mw_auth_mtls>(
      "/app", [](const mungo::request& req, mungo::response& res) {
        const auto& cert = req.tls_cert_info();
        const auto subject = cert.get_subject_name().substr(3);
        res.header("Content-Type", "text/html")
            .ok(std::format(html_auth, subject, cert.get_serial_number()));
      });

  while (is_running) {
    server.poll(100);
  }

  return 0;
}