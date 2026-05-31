#ifndef MUNGO_REQUEST_HPP
#define MUNGO_REQUEST_HPP

#include <array>
#include <mgxx/mgxx.hpp>
#include <optional>
#include <string>
#include <string_view>
#include <variant>

#include "mungo/internal/meta.hpp"
#include "mungo/internal/request_fwd.hpp"
#include "mungo/internal/route.hpp"
#include "mungo/optional.hpp"

namespace mungo {
template <typename Attrs>
class basic_request {
  mgxx::http::async_request m_request;
  internal::route m_route;
  std::array<Attrs, std::variant_size_v<Attrs>> m_attributes;

  template <typename T>
    requires std::is_integral_v<T>
  [[nodiscard]] static std::optional<T> get_number(const std::string_view str) {
    T value;
    const auto [ptr, err] =
        std::from_chars(str.data(), str.data() + str.size(), value);
    if (err != std::errc{}) {
      return std::nullopt;
    }

    return value;
  }

  [[nodiscard]] std::optional<std::string_view> param_view(
      const std::string_view name) const {
    for (std::size_t i = 0; i < m_route.params.size(); ++i) {
      if (m_route.params[i] == name) {
        return m_request.get_param(i);
      }
    }
    return std::nullopt;
  }

 public:
  explicit basic_request(mgxx::http::async_request&& request,
                         internal::route route)
      : m_request(std::move(request)), m_route(std::move(route)) {}

  [[nodiscard]] std::string_view remote_ip() const {
    return m_request.get_remote_ip();
  }

  [[nodiscard]] bool is_mtls() const { return m_request.is_mtls(); }

  [[nodiscard]] const mgxx::tls_cert_info& tls_cert_info() const {
    return m_request.get_tls_cert_info();
  }

  [[nodiscard]] std::string_view method() const { return m_request.method(); }

  [[nodiscard]] std::string_view path() const {
    return {m_request.uri().data(),
            m_request.uri().size() + m_request.query().size()};
  }

  template <internal::route_param_parsable T>
  [[nodiscard]] std::optional<T> param(const std::string_view name) const {
    const auto param = param_view(name);
    if (!param) {
      return std::nullopt;
    }

    if constexpr (std::same_as<T, std::string_view>) {
      return param.value();
    } else {
      return basic_request::get_number<T>(param.value());
    }
  }

  template <internal::route_query_param_parsable T, size_t N = 1024>
  [[nodiscard]] std::optional<T> query(const std::string_view name) const {
    const auto param = m_request.get_query_param(name);
    if (!param) {
      return std::nullopt;
    }

    if constexpr (std::same_as<T, std::string>) {
      return mgxx::http::decode_url<N>(param.value());
    } else if constexpr (std::same_as<T, bool>) {
      return param.value() == "true" || param.value() == "1" ||
             param.value() == "on" || param.value() == "yes" ||
             param.value() == "enabled" || param.value() == "y" ||
             param.value() == "t";
    } else {
      return basic_request::get_number<T>(param.value());
    }
  }

  [[nodiscard]] std::optional<std::string_view> header(
      const std::string& name) const {
    return m_request.get_header(name);
  }

  [[nodiscard]] std::string_view body() const { return m_request.body(); }

  template <typename T, typename... Args>
    requires internal::is_attribute_valid<T, Attrs>
  void attr(Args&&... args) {
    constexpr std::size_t idx = internal::type_index_v<T, Attrs>;
    m_attributes[idx].template emplace<T>(std::forward<Args>(args)...);
  }

  template <typename T>
    requires internal::is_attribute_valid<T, Attrs>
  [[nodiscard]] optional<T> attr() noexcept {
    constexpr std::size_t idx = internal::type_index_v<T, Attrs>;
    return mungo::optional<T>(std::get_if<T>(&m_attributes[idx]));
  }

  template <typename T>
    requires internal::is_attribute_valid<T, Attrs>
  [[nodiscard]] optional<const T> attr() const noexcept {
    constexpr std::size_t idx = internal::type_index_v<T, Attrs>;
    return mungo::optional<const T>(std::get_if<T>(&m_attributes[idx]));
  }
};
}  // namespace mungo

#endif  // MUNGO_REQUEST_HPP
