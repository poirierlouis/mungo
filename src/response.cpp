#include "mungo/response.hpp"

namespace mungo {
response::response(mgxx::http::async_response&& response)
    : m_response(std::move(response)) {}

response& response::header(std::string name, std::string value) {
  m_response.get_headers().set(std::move(name), std::move(value));
  return *this;
}

void response::send(const status_code code) { m_status_code = code; }

void response::send(const status_code code, std::string body) {
  m_status_code = code;
  m_body = std::move(body);
}

void response::ok() { send(status_code::ok); }
void response::ok(std::string body) { send(status_code::ok, std::move(body)); }

void response::created(std::string body) {
  send(status_code::created, std::move(body));
}

void response::no_content() { send(status_code::no_content); }

void response::bad_request() { send(status_code::bad_request); }
void response::bad_request(std::string body) {
  send(status_code::bad_request, std::move(body));
}

void response::unauthorized() { send(status_code::unauthorized); }
void response::unauthorized(std::string body) {
  send(status_code::unauthorized, std::move(body));
}

void response::forbidden() { send(status_code::forbidden); }
void response::forbidden(std::string body) {
  send(status_code::forbidden, std::move(body));
}

void response::not_found() { send(status_code::not_found); }

void response::commit() { m_response.send(m_status_code, m_body); }
}  // namespace mungo