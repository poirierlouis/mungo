#include "mungo/response.hpp"

namespace mungo {
response::response(std::shared_ptr<mgxx::http::async_response> response)
    : m_response(std::move(response)) {}

response& response::header(std::string name, std::string value) {
  m_response->get_headers().set(std::move(name), std::move(value));
  return *this;
}

void response::send(const status_code code) { m_status_code = code; }

void response::send(const status_code code, const std::string& body) {
  m_status_code = code;
  m_body = body;
}

void response::ok() { send(status_code::ok); }
void response::ok(const std::string& body) { send(status_code::ok, body); }

void response::created(const std::string& body) {
  send(status_code::created, body);
}

void response::no_content() { send(status_code::no_content); }

void response::bad_request() { send(status_code::bad_request); }
void response::bad_request(const std::string& body) {
  send(status_code::bad_request, body);
}

void response::unauthorized() { send(status_code::unauthorized); }

void response::unauthorized(const std::string& body) {
  send(status_code::unauthorized, body);
}

void response::not_found() { send(status_code::not_found); }

void response::commit() const { m_response->send(m_status_code, m_body); }
}  // namespace mungo