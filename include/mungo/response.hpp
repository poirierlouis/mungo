#ifndef MUNGO_RESPONSE_HPP
#define MUNGO_RESPONSE_HPP

#include <mgxx/mgxx.hpp>

namespace mungo {
class response {
  mgxx::http::async_response m_response;

 public:
  explicit response(mgxx::http::async_response&& response);

  response& header(std::string name, std::string value);

  void send(mgxx::http::status_code code);
  void send(mgxx::http::status_code code, std::string body);

  void ok();
  void ok(std::string body);

  void created(std::string body);
  void no_content();

  void bad_request();
  void bad_request(std::string body);

  void not_found();
};
}  // namespace mungo

#endif  // MUNGO_RESPONSE_HPP
