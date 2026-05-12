#include "mungo/internal/route.hpp"

#include <regex>

namespace mungo::internal {
uint64_t route::hash(const std::string_view path) {
  return std::hash<std::string_view>{}(path);
}

uint64_t route::hash(const std::string_view method,
                     const std::string_view path) {
  return std::hash<std::string_view>{}(method) ^
         std::hash<std::string_view>{}(path);
}

route route::compile(std::string_view path) {
  const std::regex rule(R"(:([a-zA-Z_-]+))");
  route route;

  auto words_begin =
      std::cregex_iterator(path.data(), path.data() + path.size(), rule);
  auto words_end = std::cregex_iterator();

  for (std::cregex_iterator i = words_begin; i != words_end; ++i) {
    const std::cmatch& match = *i;
    route.params.emplace_back(match[1].str());
  }

  route.path = std::regex_replace(std::string(path), rule, "*");
  return route;
}
}  // namespace mungo
