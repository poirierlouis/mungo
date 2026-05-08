#ifndef MUNGO_TASK_HPP
#define MUNGO_TASK_HPP

#include <memory>

namespace mungo {
class task {
  struct base {
    virtual ~base() = default;
    virtual void invoke() = 0;
  };

  template <typename F>
  struct impl : base {
   private:
    F f;

   public:
    explicit impl(F&& func) : f(std::forward<F>(func)) {}

    void invoke() override { f(); }
  };

  std::unique_ptr<base> m_ptr;

 public:
  template <typename F>
  explicit task(F&& f)
      : m_ptr(std::make_unique<impl<std::decay_t<F>>>(std::forward<F>(f))) {}

  task(const task&) = delete;

  task(task&&) = default;
  task& operator=(task&&) = default;

  void operator()() const { m_ptr->invoke(); }
};
}  // namespace mungo

#endif  // MUNGO_TASK_HPP
