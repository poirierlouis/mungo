#ifndef MUNGO_INTERNAL_HASH_HPP
#define MUNGO_INTERNAL_HASH_HPP

#include <cstdint>
#include <string_view>

namespace mungo::internal {
constexpr uint64_t fnv1a(const std::string_view str) {
  uint64_t hash = 0xCBF29CE484222325ULL;
  for (const char c : str) {
    hash ^= static_cast<uint64_t>(c);
    hash *= 0x100000001B3ULL;
  }
  return hash;
}

constexpr uint64_t fnv1a(const char* str) {
  uint64_t hash = 0xCBF29CE484222325ULL;
  while (*str) {
    hash ^= static_cast<uint64_t>(*str++);
    hash *= 0x100000001B3ULL;
  }
  return hash;
}

constexpr std::string base64_encode(const std::string_view input) {
  constexpr std::string_view alphabet =
      "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

  const size_t length = input.length();
  std::string out;
  out.reserve(((length + 2) / 3) * 4);

  size_t i = 0;
  for (; i + 2 < length; i += 3) {
    out.push_back(alphabet[(input[i] >> 2) & 0x3F]);
    out.push_back(
        alphabet[((input[i] & 0x3) << 4) | ((input[i + 1] >> 4) & 0xF)]);
    out.push_back(
        alphabet[((input[i + 1] & 0xF) << 2) | ((input[i + 2] >> 6) & 0x3)]);
    out.push_back(alphabet[input[i + 2] & 0x3F]);
  }

  if (i < length) {
    out.push_back(alphabet[(input[i] >> 2) & 0x3F]);
    if (i + 1 == length) {
      out.push_back(alphabet[(input[i] & 0x3) << 4]);
      out.push_back('=');
      out.push_back('=');
    } else {
      out.push_back(
          alphabet[((input[i] & 0x3) << 4) | ((input[i + 1] >> 4) & 0xF)]);
      out.push_back(alphabet[(input[i + 1] & 0xF) << 2]);
      out.push_back('=');
    }
  }
  return out;
}

constexpr std::string base64_decode(const std::string_view input) {
  constexpr auto get_value = [](const char c) -> signed char {
    if (c >= 'A' && c <= 'Z') return c - 'A';
    if (c >= 'a' && c <= 'z') return c - 'a' + 26;
    if (c >= '0' && c <= '9') return c - '0' + 52;
    if (c == '+') return 62;
    if (c == '/') return 63;
    return -1;
  };

  if (input.empty()) return {};
  if (input.length() % 4 != 0) {
    return {};
  }

  const size_t length = input.length();
  std::string out;
  out.reserve((length / 4) * 3);

  for (size_t i = 0; i < length; i += 4) {
    const signed char v1 = get_value(input[i]);
    const signed char v2 = get_value(input[i + 1]);
    const signed char v3 = get_value(input[i + 2]);
    const signed char v4 = get_value(input[i + 3]);
    if (v1 < 0 || v2 < 0) {
      return {};
    }

    unsigned int n = (static_cast<unsigned int>(v1) << 18) |
                     (static_cast<unsigned int>(v2) << 12);
    out.push_back(static_cast<char>((n >> 16) & 0xFF));

    if (v3 >= 0) {
      n |= (static_cast<unsigned int>(v3) << 6);
      out.push_back(static_cast<char>((n >> 8) & 0xFF));
    }

    if (v4 >= 0) {
      n |= static_cast<unsigned int>(v4);
      out.push_back(static_cast<char>(n & 0xFF));
    }
  }

  return out;
}
}  // namespace mungo::internal

#endif  // MUNGO_INTERNAL_HASH_HPP
