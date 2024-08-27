#include <memory>
#include <cassert>
#include <type_traits>
#include <iostream>

// move only function
template <typename... Args>
struct callback {
    callback() = default;
    callback(callback const &) = delete;
    callback& operator=(callback const &) = delete;
    callback(callback &&) = default;
    callback& operator=(callback &&) = default;

    template <typename F, 
              class = std::enable_if_t<
                std::is_invocable_v<F, Args...> && 
                !std::is_same_v<std::decay_t<F>, callback>>>
    callback(F &&f) 
        : m_base(std::make_unique<_callback_impl<std::decay_t<F>>>(
            std::forward<F>(f))) {}

    // 显式调用
    void operator()(Args... args) {
        assert(m_base);
        m_base->_call(std::forward<Args>(args)...);
        m_base = nullptr;
    }

    explicit operator bool() const noexcept {
        return m_base != nullptr;
    }
private:
    struct _callback_base {
        virtual void _call(Args... args) = 0;
        virtual ~_callback_base() = default;
    };

    template <typename F>
    struct _callback_impl final : _callback_base {
        F m_func;

        template <typename... Ts, 
                  class = std::enable_if_t<std::is_constructible_v<F, Ts...>>>
        _callback_impl(Ts&&... ts) : m_func(std::forward<Ts>(ts)...) {}

        void _call(Args... args) {
            m_func(std::forward<Args>(args)...);
        }
    };

    std::unique_ptr<_callback_base> m_base;
};

int main() {
    callback<int, int> cb([](int a, int b) {
        std::cout << a + b << '\n';
    });

    cb(1, 2);
    return 0;
}