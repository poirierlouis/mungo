#ifndef MUNGO_REQUEST_HPP
#define MUNGO_REQUEST_HPP

#include <memory>
#include <mgxx/mgxx.hpp>

#include "mungo/internal/route.hpp"

namespace mungo {
class request {
  std::unique_ptr<mgxx::http::async_request> m_request;
  internal::route m_route;

  [[nodiscard]] std::optional<std::string_view> param_view(
      std::string_view name) const;

 public:
  explicit request(std::unique_ptr<mgxx::http::async_request> request,
                   internal::route route);

  [[nodiscard]] std::string_view remote_ip() const;

  [[nodiscard]] std::string_view method() const;
  [[nodiscard]] std::string_view path() const;

  template <internal::route_parsable T>
  [[nodiscard]] std::optional<T> param(const std::string_view name) const {
    const auto param = param_view(name);
    if (!param) {
      return std::nullopt;
    }

    if constexpr (std::same_as<T, std::string_view>) {
      return param.value();
    } else {
      T value;
      const auto [ptr, err] =
          std::from_chars(param.value().data(),
                          param.value().data() + param.value().size(), value);
      if (err == std::errc{}) {
        return value;
      }

      return std::nullopt;
    }
  }

  [[nodiscard]] std::optional<std::string_view> header(
      const std::string& name) const;
  [[nodiscard]] std::string_view body() const;
};
}  // namespace mungo

#endif  // MUNGO_REQUEST_HPP
