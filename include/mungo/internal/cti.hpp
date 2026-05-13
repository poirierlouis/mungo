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
}

#endif  // MUNGO_CTI_HPP
