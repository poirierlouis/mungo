#ifndef MUNGO_INTERNAL_TASK_HPP
#define MUNGO_INTERNAL_TASK_HPP

#include <memory>
#include <mgxx/mgxx.hpp>
#include <type_traits>
#include <utility>

namespace mungo {
class request;
class response;
}  // namespace mungo

namespace mungo::internal {

template <typename... Args>
class basic_task {
  std::unique_ptr<mgxx::listener<Args...>> m_ptr;

 public:
  template <typename F>
    requires(!std::is_same_v<std::remove_cvref_t<F>, basic_task>)
  explicit basic_task(F&& callback)
      : m_ptr(std::make_unique<mgxx::lambda_listener<std::decay_t<F>, Args...>>(
            std::forward<F>(callback))) {}

  basic_task(const basic_task&) = delete;
  basic_task& operator=(const basic_task&) = delete;

  basic_task(basic_task&&) noexcept = default;
  basic_task& operator=(basic_task&&) = default;

  void operator()(Args... args) const {
    m_ptr->invoke(std::forward<Args>(args)...);
  }
};

using task = basic_task<>;
using middleware_task = basic_task<const request&, response&>;

}  // namespace mungo::internal

#endif  // MUNGO_INTERNAL_TASK_HPP
