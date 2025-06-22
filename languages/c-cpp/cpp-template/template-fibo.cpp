using ull = unsigned long long;

template <ull x> consteval ull fib() { return fib<x - 1>() + fib<x - 2>(); }

template <> consteval ull fib<0>() { return 1; }

template <> consteval ull fib<1>() { return 1; }

template <ull x> struct fib_t {
    static ull const value = fib_t<x - 1>::value + fib_t<x - 2>::value;
};

template <> struct fib_t<0> {
    static ull const value = 1;
};

template <> struct fib_t<1> {
    static ull const value = 1;
};

#include <iostream>

int main() {
    // if we use constexpr to specify function, compile and run will be slow
    constexpr ull x = fib<100>();
    std::cout << x << std::endl;

    // struct: compile fast and run fast
    std::cout << fib_t<100>::value << std::endl;
    return 0;
}