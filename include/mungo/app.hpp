#ifndef MUNGO_APP_HPP
#define MUNGO_APP_HPP

#include <filesystem>
#include <functional>
#include <memory>
#include <string>
#include <string_view>

#include "mungo/internal/filesystem.hpp"
#include "mungo/internal/route.hpp"
#include "mungo/internal/task.hpp"
#include "mungo/request.hpp"
#include "mungo/response.hpp"

namespace mungo {
struct config {
  std::string unsafe_host;
  std::string safe_host;

  std::filesystem::path ca;
  std::filesystem::path cert;
  std::filesystem::path key;
};

class app {
 public:
  using executor = mgxx::listener<task>;
  template <typename F>
  using lambda_executor = mgxx::lambda_listener<F, task>;

 private:
  using routes = std::unordered_map<uint64_t, route>;
  using handlers =
      std::unordered_map<uint64_t, std::unique_ptr<route::handler>>;

  std::unique_ptr<mgxx::server> m_server;
  std::unique_ptr<executor> m_executor;
  std::shared_ptr<mgxx::http::endpoint> m_http;
  std::shared_ptr<mgxx::http::endpoint> m_https;

  std::string m_redirect;
  routes m_routes;
  handlers m_handlers;

  std::optional<route> register_route(std::string_view path);

  template <typename F>
  void dispatch(const std::string_view method, const std::string& path,
                F&& handler) {
    const auto hash = route::hash(method, path);
    m_handlers[hash] =
        std::make_unique<route::lambda_handler<F>>(std::forward<F>(handler));

    auto route = register_route(path);
    if (!route) {
      return;
    }

    if (!m_https) {
      MG_ERROR(("HTTPS endpoint not initialized, cannot register route: %s",
                path.c_str()));
      return;
    }

    m_https->on_async_request(
        route->path,
        [this, path](
            const mgxx::http::request& mg_req,
            const std::shared_ptr<mgxx::http::async_response>& mg_res) mutable {
          const auto it = m_routes.find(route::hash(path));
          if (it == m_routes.end()) {
            mg_res->send(mgxx::http::status_code::internal_server_error);
            return;
          }

          const auto it_handler =
              m_handlers.find(route::hash(mg_req.method(), path));
          if (it_handler == m_handlers.end()) {
            mg_res->send(mgxx::http::status_code::not_found);
            return;
          }

          // TODO: add middlewares

          m_executor->invoke(
              task([route = it->second, handler = it_handler->second.get(),
                    l_req = mg_req.to_async(), l_res = mg_res]() mutable {
                const request req(std::move(l_req), std::move(route));
                const response res(l_res);
                handler->invoke(req, res);
              }));
        });
  }

 public:
  template <typename F>
  bool setup(const config& config, F&& logger, const int level = MG_LL_ERROR,
             const size_t size = 8192) {
    m_server =
        std::make_unique<mgxx::server>(std::forward<F>(logger), level, size);
    if (!m_server) {
      return false;
    }

    const auto ca = filesystem::read(config.ca);
    const auto cert = filesystem::read(config.cert);
    const auto key = filesystem::read(config.key);
    const auto is_tls = !config.safe_host.empty() && cert && key;
    const auto is_mtls = is_tls && ca;

    if (is_tls) {
      m_https = m_server->listen_http("https://" + config.safe_host);
      if (!m_https) {
        return false;
      }

      if (is_mtls) {
        m_https->use_tls(ca.value(), cert.value(), key.value());
      } else {
        m_https->use_tls(cert.value(), key.value());
      }

      m_http = m_server->listen_http("http://" + config.unsafe_host);
      if (!m_http) {
        return false;
      }

      m_redirect = std::format("https://{}", config.safe_host);
      m_http->on_fallback([redirect = std::string_view(m_redirect)](
                              const mgxx::http::request& req,
                              mgxx::http::response& res) {
        res.get_headers().set("Location", std::format("{}{}{}", redirect,
                                                      req.uri(), req.query()));
        res.send(mgxx::http::status_code::moved_permanently);
      });
    } else {
      m_https = m_server->listen_http("http://" + config.unsafe_host);
      if (!m_https) {
        return false;
      }
    }

    return true;
  }

  template <typename F>
  void use_pool(F&& task) {
    m_executor = std::make_unique<lambda_executor<std::decay_t<F>>>(
        std::forward<F>(task));
  }

  template <typename F>
  app& get(const std::string& path, F&& handler) {
    dispatch("GET", path, std::forward<F>(handler));
    return *this;
  }

  template <typename F>
  app& post(const std::string& path, F&& handler) {
    dispatch("POST", path, std::forward<F>(handler));
    return *this;
  }

  template <typename F>
  app& put(const std::string& path, F&& handler) {
    dispatch("PUT", path, std::forward<F>(handler));
    return *this;
  }

  template <typename F>
  app& patch(const std::string& path, F&& handler) {
    dispatch("PATCH", path, std::forward<F>(handler));
    return *this;
  }

  template <typename F>
  app& del(const std::string& path, F&& handler) {
    dispatch("DELETE", path, std::forward<F>(handler));
    return *this;
  }

  void poll(int ms) const;
};
}  // namespace mungo

#endif  // MUNGO_APP_HPP
