#include "mungo/response.hpp"

namespace mungo {
response::response(std::shared_ptr<mgxx::http::async_response> response)
    : m_response(std::move(response)) {}

const response& response::header(std::string name, std::string value) const {
  m_response->get_headers().set(std::move(name), std::move(value));
  return *this;
}

void response::send(const mgxx::http::status_code code) const {
  m_response->send(code);
}

void response::send(const mgxx::http::status_code code,
                    std::string body) const {
  m_response->send(code, std::move(body));
}

void response::ok() const { m_response->send(mgxx::http::status_code::ok); }
void response::ok(std::string body) const {
  m_response->send(mgxx::http::status_code::ok, std::move(body));
}

void response::created(std::string body) const {
  m_response->send(mgxx::http::status_code::created, std::move(body));
}

void response::no_content() const {
  m_response->send(mgxx::http::status_code::no_content);
}

void response::bad_request() const {
  m_response->send(mgxx::http::status_code::bad_request);
}
void response::bad_request(std::string body) const {
  m_response->send(mgxx::http::status_code::bad_request, std::move(body));
}

void response::not_found() const {
  m_response->send(mgxx::http::status_code::not_found);
}
}  // namespace mungo