#include "mungo/app.hpp"

#include <regex>

namespace mungo {
void app::poll(const int ms) const { m_server->poll(ms); }

std::optional<route> app::register_route(const std::string_view path) {
  const auto hash = route::hash(path);
  if (const auto it = m_routes.find(hash); it != m_routes.end()) {
    return std::nullopt;
  }

  return m_routes.try_emplace(hash, route::compile(path)).first->second;
}
}  // namespace mungo