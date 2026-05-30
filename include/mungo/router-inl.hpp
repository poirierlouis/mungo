#ifndef MUNGO_ROUTER_INL_HPP
#define MUNGO_ROUTER_INL_HPP

#include "mungo/internal/cti.hpp"
#include "mungo/internal/middleware.hpp"

namespace mungo {

template <internal::fixed_string UriBase, is_mw... MiddlewaresBase>
template <internal::fixed_string Uri, is_mw... Middlewares,
          internal::route_handler F>
basic_router<UriBase, MiddlewaresBase...>&
basic_router<UriBase, MiddlewaresBase...>::get(F&& handler) {
  constexpr auto path = UriBase + internal::strip_end<Uri>();
  m_app.get<path, MiddlewaresBase..., Middlewares...>(std::forward<F>(handler));
  return *this;
}

template <internal::fixed_string UriBase, is_mw... MiddlewaresBase>
template <internal::fixed_string Uri, is_mw... Middlewares,
          internal::route_handler F>
basic_router<UriBase, MiddlewaresBase...>&
basic_router<UriBase, MiddlewaresBase...>::post(F&& handler) {
  constexpr auto path = UriBase + internal::strip_end<Uri>();
  m_app.post<path, MiddlewaresBase..., Middlewares...>(
      std::forward<F>(handler));
  return *this;
}

template <internal::fixed_string UriBase, is_mw... MiddlewaresBase>
template <internal::fixed_string Uri, is_mw... Middlewares,
          internal::route_handler F>
basic_router<UriBase, MiddlewaresBase...>&
basic_router<UriBase, MiddlewaresBase...>::put(F&& handler) {
  constexpr auto path = UriBase + internal::strip_end<Uri>();
  m_app.put<path, MiddlewaresBase..., Middlewares...>(std::forward<F>(handler));
  return *this;
}

template <internal::fixed_string UriBase, is_mw... MiddlewaresBase>
template <internal::fixed_string Uri, is_mw... Middlewares,
          internal::route_handler F>
basic_router<UriBase, MiddlewaresBase...>&
basic_router<UriBase, MiddlewaresBase...>::patch(F&& handler) {
  constexpr auto path = UriBase + internal::strip_end<Uri>();
  m_app.patch<path, MiddlewaresBase..., Middlewares...>(
      std::forward<F>(handler));
  return *this;
}

template <internal::fixed_string UriBase, is_mw... MiddlewaresBase>
template <internal::fixed_string Uri, is_mw... Middlewares,
          internal::route_handler F>
basic_router<UriBase, MiddlewaresBase...>&
basic_router<UriBase, MiddlewaresBase...>::del(F&& handler) {
  constexpr auto path = UriBase + internal::strip_end<Uri>();
  m_app.del<path, MiddlewaresBase..., Middlewares...>(std::forward<F>(handler));
  return *this;
}

}  // namespace mungo

#endif  // MUNGO_ROUTER_INL_HPP