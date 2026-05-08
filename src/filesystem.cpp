#include "../include/mungo/internal/filesystem.hpp"

#include <fstream>

namespace mungo {
std::optional<std::string> filesystem::read(const std::filesystem::path& path) {
  std::ifstream file(path);
  if (!file) {
    return std::nullopt;
  }

  return std::string(std::istreambuf_iterator(file),
                     std::istreambuf_iterator<char>());
}
}  // namespace mungo