#ifndef MUNGO_INTERNAL_MIDDLEWARE_HPP
#define MUNGO_INTERNAL_MIDDLEWARE_HPP

#include <mgxx/mgxx.hpp>

#include "mungo/internal/route.hpp"
#include "mungo/internal/task.hpp"

namespace mungo {
class response;

template <class T>
concept is_mw = std::is_class_v<T> && std::is_empty_v<T>;

template <typename F>
concept middleware_handler = std::is_invocable_v<F, request&, response&, internal::middleware_task>;
}  // namespace mungo

namespace mungo::internal {
using middleware_listener =
    mgxx::listener<request&, response&, middleware_task>;
template <typename F>
using lambda_middleware_listener =
    mgxx::lambda_listener<F, request&, response&, middleware_task>;
}  // namespace mungo::internal

#endif  // MUNGO_INTERNAL_MIDDLEWARE_HPP
