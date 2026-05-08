#ifndef MUNGO_FILESYSTEM_HPP
#define MUNGO_FILESYSTEM_HPP

#include <filesystem>
#include <optional>
#include <string>

namespace mungo {
class filesystem {
 public:
  static std::optional<std::string> read(const std::filesystem::path& path);
};
}  // namespace mungo

#endif  // MUNGO_FILESYSTEM_HPP
