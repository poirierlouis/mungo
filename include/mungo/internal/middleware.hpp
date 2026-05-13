#ifndef MUNGO_INTERNAL_MIDDLEWARE_HPP
#define MUNGO_INTERNAL_MIDDLEWARE_HPP

#include <mgxx/mgxx.hpp>

#include "mungo/internal/route.hpp"
#include "mungo/internal/task.hpp"

namespace mungo {
class request;
class response;
}  // namespace mungo

namespace mungo::internal {
using middleware = mgxx::listener<const request&, response&, middleware_task>;
template <typename F>
using lambda_middleware =
    mgxx::lambda_listener<F, const request&, response&, middleware_task>;
}  // namespace mungo::internal

#endif  // MUNGO_INTERNAL_MIDDLEWARE_HPP
