#ifndef MUNGO_INTERNAL_META_HPP
#define MUNGO_INTERNAL_META_HPP

#include <type_traits>
#include <variant>

namespace mungo::internal {
template <typename F, typename U>
struct merge;
template <typename... F, typename... U>
struct merge<std::variant<F...>, std::variant<U...>> {
  using type = std::variant<F..., U...>;
};

template <typename T, typename Variant>
struct type_index;
template <typename T, typename... Types>
struct type_index<T, std::variant<Types...>> {
  static constexpr std::size_t get() {
    constexpr bool matches[] = {std::is_same_v<T, Types>...};
    for (std::size_t i = 0; i < sizeof...(Types); ++i) {
      if (matches[i]) return i;
    }
    return -1;
  }
};

template <typename T, typename Variant>
inline constexpr std::size_t type_index_v = type_index<T, Variant>::get();

template <typename T, typename Variant>
concept is_attribute_valid =
    type_index_v<T, Variant> != static_cast<std::size_t>(-1);
}  // namespace mungo::internal

#endif  // MUNGO_INTERNAL_META_HPP
