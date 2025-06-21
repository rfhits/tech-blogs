## cpp 协程（coroutine）

```
v2
created, 2024-06-27T11:04+08:00
modified, 2025-06-06T12:07+08:00
modified, 2025-06-21T20:50+08:00, 看到了 Asymmetric Transfer 但是看不懂
published, 2025-06-21T20:58+08:00
category: c-cpp
```

互联网上大家看的应该都是这几篇博客，我觉得 2 和 3 两篇顺序要调换一下：

1. [Coroutine Theory](https://lewissbaker.github.io/2017/09/25/coroutine-theory)
   协程的原理，从内存、寄存器的角度介绍如何实现协程
2. [Understanding the promise type](https://lewissbaker.github.io/2018/09/05/understanding-the-promise-type)
   主要是说 promise_type 如何改写 `co_await`、`co_yield` 和 `co_return`，像异常的传播我暂时也看不懂
3. [Understanding operator co_await](https://lewissbaker.github.io/2017/11/17/understanding-operator-co-await)
4. [Asymmetric Transfer](https://lewissbaker.github.io/2017/09/25/coroutine-theory)
   我理解为就是两个协程相互 resume，而 resume 即函数调用，虽然 rbp 调整为协程帧，但是栈指针一直下压，导致溢出

## `promise_type`: 协程状态存储

下面是一个简单的协程例子：

```cpp
#include <coroutine>
#include <iostream>

struct task {
    struct promise_type {
        task get_return_object() {
            std::cout << "get_return_object()" << std::endl;
            return task{
                std::coroutine_handle<promise_type>::from_promise(*this)};
        }

        std::suspend_always initial_suspend() noexcept {
            return {};
        }

        std::suspend_always final_suspend() noexcept {
            return {};
        }

        void unhandled_exception() {}
        void return_void() noexcept {}
    };
    std::coroutine_handle<promise_type> handle;
};

task func() {
    std::cout << "start" << std::endl;
    co_await std::suspend_always{};
    std::cout << "resume" << std::endl;
}

int main() {
    auto c = func();
    c.handle.resume();
    c.handle.resume();
    return 0;
}
```

协程无非就是有一段代码，我们称它为 callee 好了，caller 调用 callee，callee 有自己的上下文（context），具体来说就是寄存器那些。

callee 发起了一个非常耗时的调用，他不想阻塞在这里，就需要把自己执行的状态保存到一个对象中，比如执行到 callee 的第几行了，现在上下文是什么，然后把执行流转移给 caller。

并且 callee 要向外暴露一个接口（handle），如果 callee 中那个耗时的调用执行完了，可以通过 `handle.resume()` 回来。

上文的 `func` 就是这个 callee 中的指令内容，`task` 就是要把上下文保存到的对象，`task::promise_type` 用于定义协程创建、结束时候的行为，如是否挂起。

## `co_await`

`co_await` 是协程最关键的地方，它的语法是 `co_await awaitable`

这里直接抄 [C++ Coroutines: Understanding operator co_await](https://lewissbaker.github.io/2017/11/17/understanding-operator-co-await)，`co_await` 会被展开成有 `await_ready` `await_suspend` 和 `await_resume` 的调用块：

```cpp
{
  auto&& value = <expr>;
  auto&& awaitable = get_awaitable(promise, static_cast<decltype(value)>(value));
  auto&& awaiter = get_awaiter(static_cast<decltype(awaitable)>(awaitable));
  if (!awaiter.await_ready())
  {
    using handle_t = std::experimental::coroutine_handle<P>;

    using await_suspend_result_t =
      decltype(awaiter.await_suspend(handle_t::from_promise(promise)));

    <suspend-coroutine>

    if constexpr (std::is_void_v<await_suspend_result_t>)
    {
      awaiter.await_suspend(handle_t::from_promise(promise));
      <return-to-caller-or-resumer>
    }
    else
    {
      static_assert(
         std::is_same_v<await_suspend_result_t, bool>,
         "await_suspend() must return 'void' or 'bool'.");

      if (awaiter.await_suspend(handle_t::from_promise(promise)))
      {
        <return-to-caller-or-resumer>
      }
    }

    <resume-point>
  }

  return awaiter.await_resume();
}
```

这里强调下 `await_suspend()`，协程调度的关键就在这里。
博客原文给了一个例子，通过 `await_suspend(handle)` 把 `handle` 注册到 event 对象中，然后立刻返回。

因为 event 是异步的，当 event 在某个线程中被完成后，可以检查挂在自己身上的 handles，逐个 resume。
