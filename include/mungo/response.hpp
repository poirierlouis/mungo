#ifndef MUNGO_RESPONSE_HPP
#define MUNGO_RESPONSE_HPP

#include <mgxx/mgxx.hpp>

#include "mungo/status_code.hpp"

namespace mungo {
class app;

class response {
  mgxx::http::async_response m_response;
  status_code m_status_code{status_code::internal_server_error};
  std::string m_body;

  void commit();

  friend class app;

 public:
  explicit response(mgxx::http::async_response&& response);

  response& header(std::string name, std::string value);

  void send(status_code code);
  void send(status_code code, std::string body);

  void ok();
  void ok(std::string body);

  void created(std::string body);
  void no_content();

  void bad_request();
  void bad_request(std::string body);

  void unauthorized();
  void unauthorized(std::string body);

  void not_found();
};
}  // namespace mungo

#endif  // MUNGO_RESPONSE_HPP
