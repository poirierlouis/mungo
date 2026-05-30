// Include customization point header.
#include <mungo/attributes.hpp>
#include <string>
#include <variant>

enum class role : uint8_t { none, user, admin };

struct user_session {
  std::string email;
  role role;
};

// Inject types into mungo's attribute system BEFORE including
// <mungo/mungo.hpp>. This specialization is picked up when the `mungo::request`
// alias is resolved.
template <>
struct mungo::custom_attrs<> {
  using type = std::variant<user_session>;
};

#include <BS_thread_pool.hpp>
#include <atomic>
#include <csignal>
#include <iostream>
#include <mungo/mungo.hpp>

struct mw_auth {};
struct mw_auth_admin {};

std::atomic_bool is_running = true;

void handle_signal(int) { is_running = false; }

std::vector<std::string> split(const std::string_view str,
                               const char delimiter) {
  std::vector<std::string> tokens;
  std::string token;
  for (const auto c : str) {
    if (c == delimiter) {
      tokens.push_back(std::move(token));
      token.clear();
    } else {
      token.push_back(c);
    }
  }
  if (!token.empty()) {
    tokens.push_back(std::move(token));
  }
  return tokens;
}

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

  const auto auth_middleware = [](role role) {
    return [role](mungo::request& req, mungo::response& res, auto next) {
      const auto auth = req.header("Authorization").value_or("");
      if (!auth.starts_with("Basic ")) {
        res.header("WWW-Authenticate", "Basic realm=\"Restricted Area\"")
            .unauthorized();
        return;
      }

      const auto credentials = mungo::internal::base64_decode(auth.substr(6));
      const auto tokens = split(credentials, ':');
      if (tokens.size() != 2) {
        res.header("WWW-Authenticate", "Basic realm=\"Restricted Area\"")
            .unauthorized();
        return;
      }

      const auto& email = tokens[0];
      const auto& pwd = tokens[1];
      auto user_role = role::none;
      if (email == "admin@admin.com" && pwd == "admin") {
        user_role = role::admin;
      } else if (email == "user@user.com" && pwd == "user") {
        user_role = role::user;
      } else {
        res.header("WWW-Authenticate", "Basic realm=\"Restricted Area\"")
            .unauthorized();
        return;
      }

      if (role == role::admin && user_role != role::admin) {
        res.header("WWW-Authenticate", "Basic realm=\"Restricted Area\"")
            .forbidden();
        return;
      }

      req.attr<user_session>(email, user_role);
      next(req, res);
    };
  };
  server.use_middleware<mw_auth>(auth_middleware(role::none));
  server.use_middleware<mw_auth_admin>(auth_middleware(role::admin));

  server.get<"/">([](const mungo::request& req, mungo::response& res) {
    const auto user = req.attr<user_session>();
    if (user) {
      res.header("Location", user->role == role::admin ? "/admin" : "/app")
          .send(mungo::status_code::temporary_redirect);
      return;
    }

    res.ok(std::format("Welcome visitor!"));
  });

  server.get<"/app", mw_auth>(
      [](const mungo::request& req, mungo::response& res) {
        const auto user = req.attr<user_session>();
        res.ok(std::format(R"({{"user": "{}", "is_admin": {}}})", user->email,
                           user->role == role::admin));
      });

  server.get<"/admin", mw_auth_admin>(
      [](const mungo::request& req, mungo::response& res) {
        const auto user = req.attr<user_session>();
        res.ok(std::format(R"({{"user": "{}", "is_admin": {}}})", user->email,
                           user->role == role::admin));
      });

  while (is_running) {
    server.poll(100);
  }

  return 0;
}
