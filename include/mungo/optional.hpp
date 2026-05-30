#ifndef MUNGO_OPTIONAL_HPP
#define MUNGO_OPTIONAL_HPP

#include <type_traits>

namespace mungo {
template <typename T>
class optional {
  static_assert(!std::is_reference_v<T> && !std::is_pointer_v<T>);
  T* m_ptr = nullptr;

 public:
  constexpr optional() noexcept = default;
  constexpr explicit optional(T* ptr) noexcept : m_ptr(ptr) {}

  [[nodiscard]] constexpr bool has_value() const noexcept {
    return m_ptr != nullptr;
  }
  [[nodiscard]] constexpr explicit operator bool() const noexcept {
    return m_ptr != nullptr;
  }

  [[nodiscard]] constexpr T* operator->() const noexcept { return m_ptr; }
  [[nodiscard]] constexpr T& operator*() const noexcept { return *m_ptr; }

  [[nodiscard]] constexpr T& value() const { return *m_ptr; }

  template <typename U>
  [[nodiscard]] constexpr T value_or(U&& value) const {
    return m_ptr ? *m_ptr : static_cast<T>(std::forward<U>(value));
  }
};
}  // namespace mungo

#endif  // MUNGO_OPTIONAL_HPP
