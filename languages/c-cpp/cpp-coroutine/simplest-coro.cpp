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

