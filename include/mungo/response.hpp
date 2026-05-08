#ifndef MUNGO_RESPONSE_HPP
#define MUNGO_RESPONSE_HPP

#include <mgxx/mgxx.hpp>

namespace mungo {
class response {
  std::shared_ptr<mgxx::http::async_response> m_response;

 public:
  explicit response(std::shared_ptr<mgxx::http::async_response> response);

  const response& header(std::string name, std::string value) const;

  void send(mgxx::http::status_code code) const;
  void send(mgxx::http::status_code code, std::string body) const;

  void ok() const;
  void ok(std::string body) const;

  void created(std::string body) const;
  void no_content() const;

  void bad_request() const;
  void bad_request(std::string body) const;

  void not_found() const;
};
}  // namespace mungo

#endif  // MUNGO_RESPONSE_HPP
