#ifndef MUNGO_INTERNAL_ROUTE_HPP
#define MUNGO_INTERNAL_ROUTE_HPP

#include <mgxx/mgxx.hpp>
#include <string>
#include <string_view>
#include <unordered_map>
#include <vector>

#include "mungo/internal/request_fwd.hpp"

namespace mungo {
class response;
}  // namespace mungo

namespace mungo::internal {
template <typename T>
concept route_param_parsable =
    std::is_integral_v<T> || std::is_same_v<T, std::string_view>;

template <typename T>
concept route_query_param_parsable =
    std::is_same_v<T, bool> || std::is_integral_v<T> ||
    std::is_same_v<T, std::string>;

template <typename F>
concept route_handler = std::is_invocable_v<F, const request&, response&>;

struct route {
  using handler = mgxx::listener<request&, response&>;
  template <typename F>
  using lambda_handler = mgxx::lambda_listener<F, request&, response&>;

  std::string path;
  std::vector<std::string> params;

  static uint64_t hash(std::string_view path);
  static uint64_t hash(std::string_view method, std::string_view path);

  static route compile(std::string_view path);
};

using routes = std::unordered_map<uint64_t, std::vector<route>>;
}  // namespace mungo::internal

#endif  // MUNGO_INTERNAL_ROUTE_HPP
