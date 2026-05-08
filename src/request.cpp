#include "mungo/request.hpp"

#include "mungo/internal/route.hpp"

namespace mungo {
request::request(std::shared_ptr<mgxx::http::async_request> request,
                 route route)
    : m_request(std::move(request)), m_route(std::move(route)) {}

std::string_view request::remote_ip() const {
  return m_request->get_remote_ip();
}

std::string_view request::method() const { return m_request->method(); }

std::string_view request::path() const {
  return {m_request->uri().data(),
          m_request->uri().size() + m_request->query().size()};
}

std::optional<std::string_view> request::param_view(
    const std::string_view name) const {
  for (size_t i = 0; i < m_route.params.size(); ++i) {
    const auto& param = m_route.params[i];
    if (param == name) {
      return m_request->get_param(i);
    }
  }

  return std::nullopt;
}

std::optional<std::string_view> request::header(const std::string& name) const {
  return m_request->get_header(name);
}

std::string_view request::body() const { return m_request->body(); }
}  // namespace mungo