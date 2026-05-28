#ifndef MUNGO_CTI_HPP
#define MUNGO_CTI_HPP

#include <cstdint>
#include <string_view>

#include "mungo/internal/hash.hpp"

namespace mungo::internal {
using type_id = uint64_t;

template <typename T>
consteval std::uint64_t get_type_id() {
#if defined(__clang__) || defined(__GNUC__)
  constexpr auto name = std::string_view{__PRETTY_FUNCTION__};
#elif defined(_MSC_VER)
  constexpr auto name = std::string_view{__FUNCSIG__};
#else
#error "mungo: compiler not supported for mungo::internal::get_type_id<T>()."
#endif
  return fnv1a(name);
}

template <size_t N>
struct fixed_string {
  char buffer[N]{};

  constexpr fixed_string() = default;

  // ReSharper disable once CppNonExplicitConvertingConstructor
  constexpr fixed_string(const char (&str)[N]) { std::copy_n(str, N, buffer); }

  template <size_t N2>
  constexpr fixed_string(const fixed_string<N2>& src, const size_t count) {
    const size_t to_copy = count < N ? count : N - 1;
    std::copy_n(src.buffer, to_copy, buffer);
    buffer[to_copy] = '\0';
  }

  [[nodiscard]] constexpr char* data() const { return buffer; }

  [[nodiscard]] constexpr std::string_view view() const {
    return {buffer, N > 0 ? N - 1 : 0};
  }
};

template <size_t N>
fixed_string(const char (&)[N]) -> fixed_string<N>;

template <fixed_string str>
constexpr size_t strip_end_length() {
  size_t length = str.view().size();
  while (length > 0 && str.buffer[length - 1] == '/') {
    --length;
  }
  return length;
}

template <fixed_string str>
constexpr auto strip_end() {
  constexpr size_t length = strip_end_length<str>();
  return fixed_string<length + 1>(str, length);
}

template <size_t N1, size_t N2>
consteval fixed_string<N1 + N2 - 1> operator+(const fixed_string<N1>& lhs,
                                              const fixed_string<N2>& rhs) {
  fixed_string<N1 + N2 - 1> result{};
  std::copy_n(lhs.buffer, N1 - 1, result.buffer);
  std::copy_n(rhs.buffer, N2, result.buffer + (N1 - 1));
  return result;
}
}  // namespace mungo::internal

#endif  // MUNGO_CTI_HPP
