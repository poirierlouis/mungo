#ifndef MUNGO_ROUTE_HPP
#define MUNGO_ROUTE_HPP

#include <mgxx/mgxx.hpp>
#include <string>
#include <string_view>
#include <unordered_map>
#include <vector>

namespace mungo {
class request;
class response;

template <typename T>
concept route_parsable =
    std::is_integral_v<T> || std::is_same_v<T, std::string_view>;

struct route {
  using handler = mgxx::listener<const request&, const response&>;
  template <typename F>
  using lambda_handler =
      mgxx::lambda_listener<F, const request&, const response&>;

  std::string path;
  std::vector<std::string> params;

  static uint64_t hash(std::string_view path);
  static uint64_t hash(std::string_view method, std::string_view path);

  static route compile(std::string_view path);
};

using routes = std::unordered_map<uint64_t, std::vector<route>>;
}  // namespace mungo

#endif  // MUNGO_ROUTE_HPP
