#ifndef MUNGO_ATTRIBUTES_HPP
#define MUNGO_ATTRIBUTES_HPP

#include <variant>

#include "internal/meta.hpp"

namespace mungo {
using FrameworkAttrs = std::variant<std::monostate>;

template <typename = void>
struct custom_attrs {
  using type = std::variant<std::monostate>;
};

template <typename Tag = void>
using AppAttrs =
    typename internal::merge<FrameworkAttrs,
                             typename custom_attrs<Tag>::type>::type;
}  // namespace mungo

#endif  // MUNGO_ATTRIBUTES_HPP
