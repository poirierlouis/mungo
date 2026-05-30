#ifndef MUNGO_ROUTER_HPP
#define MUNGO_ROUTER_HPP

#include "mungo/internal/cti.hpp"
#include "mungo/internal/middleware.hpp"
#include "mungo/internal/route.hpp"

namespace mungo {
class app;

template <internal::fixed_string UriBase, is_mw... MiddlewaresBase>
class basic_router {
  app& m_app;

 public:
  explicit basic_router(app& app) : m_app(app) {}

  template <internal::fixed_string Uri, is_mw... Middlewares>
  auto router() const {
    constexpr auto path = UriBase + internal::strip_end<Uri>();
    return basic_router<path, MiddlewaresBase..., Middlewares...>{m_app};
  }

  template <internal::fixed_string Uri, is_mw... Middlewares,
            internal::route_handler F>
  basic_router& get(F&& handler);

  template <internal::fixed_string Uri, is_mw... Middlewares,
            internal::route_handler F>
  basic_router& post(F&& handler);

  template <internal::fixed_string Uri, is_mw... Middlewares,
            internal::route_handler F>
  basic_router& put(F&& handler);

  template <internal::fixed_string Uri, is_mw... Middlewares,
            internal::route_handler F>
  basic_router& patch(F&& handler);

  template <internal::fixed_string Uri, is_mw... Middlewares,
            internal::route_handler F>
  basic_router& del(F&& handler);
};
}  // namespace mungo

#endif  // MUNGO_ROUTER_HPP
