#pragma once
#include <rtmidi17/message.hpp>
#include <coroutine>

namespace rtmidi
{
struct suspend
{
  bool await_ready() const noexcept {
    return false;
  }
  void await_suspend(std::coroutine_handle<>) const noexcept { }
  void await_resume() const noexcept { }

};

class task
{
public:
  struct promise_type
  {
    promise_type() = default;
    ~promise_type() = default;

    message m_bytes;

    static auto get_return_object_on_allocation_failure() -> task
    {
      throw std::bad_alloc();
    }

    auto get_return_object()
        -> std::coroutine_handle<promise_type>
    {
      return std::coroutine_handle<promise_type>::from_promise(*this);
    }

    auto initial_suspend()
    {
      return suspend{};
    }

    auto yield_value(message v)
    {
      m_bytes = std::move(v);
      return suspend{};
    }

    void return_value(message v)
    {
      m_bytes = std::move(v);
    }

    auto final_suspend()
    {
      return suspend{};
    }

    void unhandled_exception()
    {
      std::terminate();
    }
  };

  task(std::coroutine_handle<promise_type> handle)
      : m_handle{std::move(handle)}
  {
  }

  task(const task&) = delete;

  task(task&& other)
    : m_handle{std::exchange(other.m_handle, {})} 
  {
  }

  ~task() 
  { 
    if(m_handle)
      m_handle.destroy(); 
  }

  bool resume()
  {
    if (!m_handle.done())
      m_handle.resume();
    return !m_handle.done();
  }

  const rtmidi::message& value() { return m_handle.promise().m_bytes; }

private:
  std::coroutine_handle<promise_type> m_handle;
};

}