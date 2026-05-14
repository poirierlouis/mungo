#ifndef MUNGO_APP_HPP
#define MUNGO_APP_HPP

#include <filesystem>
#include <format>
#include <memory>
#include <optional>
#include <string>
#include <string_view>
#include <unordered_map>

#include "mungo/internal/cti.hpp"
#include "mungo/internal/filesystem.hpp"
#include "mungo/internal/middleware.hpp"
#include "mungo/internal/route.hpp"
#include "mungo/internal/task.hpp"
#include "mungo/request.hpp"
#include "mungo/response.hpp"
#include "mungo/status_code.hpp"

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
  using executor = mgxx::listener<internal::task>;
  template <typename F>
  using lambda_executor = mgxx::lambda_listener<F, internal::task>;

 private:
  using routes = std::unordered_map<uint64_t, internal::route>;
  using handlers =
      std::unordered_map<uint64_t, std::unique_ptr<internal::route::handler>>;
  using middlewares = std::unordered_map<internal::type_id,
                                         std::unique_ptr<internal::middleware>>;

  std::unique_ptr<mgxx::server> m_server;
  std::unique_ptr<executor> m_executor;
  std::shared_ptr<mgxx::http::endpoint> m_http;
  std::shared_ptr<mgxx::http::endpoint> m_https;

  std::string m_redirect;
  routes m_routes;
  handlers m_handlers;
  middlewares m_middlewares;

  std::optional<internal::route> register_route(std::string_view path);

  template <typename... M, typename F>
  auto make_chain(F&& handler) {
    if constexpr (sizeof...(M) == 0) {
      return [l_handler = std::forward<F>(handler)](const request& req,
                                                    response& res) mutable {
        l_handler(req, res);
      };
    } else {
      return make_chain_recursive<M...>(std::forward<F>(handler));
    }
  }

  template <typename M0, typename... M, typename F>
  auto make_chain_recursive(F&& handler) {
    auto next = make_chain<M...>(std::forward<F>(handler));
    return [this, next = std::move(next)](const request& req,
                                          response& res) mutable {
      constexpr auto id = internal::get_type_id<M0>();
      if (const auto it = m_middlewares.find(id); it != m_middlewares.end()) {
        it->second->invoke(
            req, res,
            internal::middleware_task(
                [&next](const request& l_req, response& l_res) mutable {
                  next(l_req, l_res);
                }));
      } else {
        next(req, res);
      }
    };
  }

  template <typename... M, typename F>
  void dispatch(const std::string_view method, const std::string& path,
                F&& handler) {
    const auto hash = internal::route::hash(method, path);
    auto chain = make_chain<M...>(std::forward<F>(handler));
    m_handlers[hash] =
        std::make_unique<internal::route::lambda_handler<decltype(chain)>>(
            std::move(chain));

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
          const auto it = m_routes.find(internal::route::hash(path));
          if (it == m_routes.end()) {
            mg_res->send(status_code::internal_server_error);
            return;
          }

          const auto it_handler =
              m_handlers.find(internal::route::hash(mg_req.method(), path));
          if (it_handler == m_handlers.end()) {
            mg_res->send(status_code::not_found);
            return;
          }

          m_executor->invoke(internal::task(
              [route = it->second, handler = it_handler->second.get(),
               l_req = mg_req.to_async(), l_res = mg_res]() mutable {
                const request req(std::move(l_req), std::move(route));
                response res(l_res);

                handler->invoke(req, res);

                res.commit();
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

    const auto ca = internal::filesystem::read(config.ca);
    const auto cert = internal::filesystem::read(config.cert);
    const auto key = internal::filesystem::read(config.key);
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
        res.send(status_code::moved_permanently);
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

  template <typename T, typename F>
  uint64_t use_middleware(F&& handler) {
    constexpr auto id = internal::get_type_id<T>();
    m_middlewares[id] =
        std::make_unique<internal::lambda_middleware<std::decay_t<F>>>(
            std::forward<F>(handler));
    return id;
  }

  template <typename... M, typename F>
  app& get(const std::string& path, F&& handler) {
    dispatch<M...>("GET", path, std::forward<F>(handler));
    return *this;
  }

  template <typename... M, typename F>
  app& post(const std::string& path, F&& handler) {
    dispatch<M...>("POST", path, std::forward<F>(handler));
    return *this;
  }

  template <typename... M, typename F>
  app& put(const std::string& path, F&& handler) {
    dispatch<M...>("PUT", path, std::forward<F>(handler));
    return *this;
  }

  template <typename... M, typename F>
  app& patch(const std::string& path, F&& handler) {
    dispatch<M...>("PATCH", path, std::forward<F>(handler));
    return *this;
  }

  template <typename... M, typename F>
  app& del(const std::string& path, F&& handler) {
    dispatch<M...>("DELETE", path, std::forward<F>(handler));
    return *this;
  }

  void poll(int ms) const;
};
}  // namespace mungo

#endif  // MUNGO_APP_HPP
