#include <memory>
#include <cassert>
#include <type_traits>
#include <iostream>
#include <chrono>
#include <map>
#include<thread>

// move only function
template <typename... Args>
struct callback {
    callback() = default;
    callback(std::nullptr_t) noexcept {}
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

struct stop_token {
    stop_token() = default;
    explicit stop_token(std::in_place_t) : m_control(std::make_shared<_control_block>()) {}

    static stop_token make() {
        return stop_token(std::in_place);
    }

    void request_stop() const {
        if (!m_control) return;
        m_control->m_stop = true;
        if (m_control->m_cb) {
            m_control->m_cb();
            m_control->m_cb = nullptr;
        }
    }

    void set_stop_callback(callback<> cb) const noexcept {
        if (!m_control) return;
        m_control->m_cb = std::move(cb);
    }

    void clear_stop_callback() const noexcept {
        if (!m_control) return;
        m_control->m_cb = nullptr;
    }
 
    explicit operator bool() const noexcept {
        return m_control != nullptr;
    }

private:
    struct _control_block {
        bool m_stop = false;
        callback<> m_cb;
    };

    std::shared_ptr<_control_block> m_control;
};

struct timer_context {
    timer_context() = default;

    void set_timeout(std::chrono::steady_clock::duration dt, 
                     callback<> cb, 
                     stop_token stop = {}) {
        auto expire_time = std::chrono::steady_clock::now() + dt;
        auto it = m_timer_heap.insert({expire_time, _timer_entry{std::move(cb), stop}});
        stop.set_stop_callback([this, it] {
            auto cb = std::move(it->second.m_cb);
            m_timer_heap.erase(it);
            cb();
        });
    }

    std::chrono::steady_clock::duration duration_to_next_timer() {
        while (!m_timer_heap.empty()) {
            auto it = m_timer_heap.begin();
            auto now = std::chrono::steady_clock::now();
            if (it->first <= now) {
                it->second.m_stop.clear_stop_callback();
                auto cb = std::move(it->second.m_cb);
                m_timer_heap.erase(it);
                cb();
            } else {
                return it->first - now;
            }
        }
        return std::chrono::nanoseconds(-1);
    }

    bool is_empty() const {
        return m_timer_heap.empty();
    }
private:
    struct _timer_entry {
        callback<> m_cb;
        stop_token m_stop;
    };
    std::multimap<std::chrono::steady_clock::time_point, _timer_entry> m_timer_heap;
};


int main() {
    // test stop token
    // stop_token st(std::in_place);

    // st.set_stop_callback([]() {
    //     std::cout << "set stop callback\n";
    // });

    // st.request_stop();

    // test timer context
    timer_context tc;
    stop_token st(std::in_place);
    tc.set_timeout(std::chrono::seconds(1), 
        []() { std::cout << "set timer stop callback"; },
    st);


    std::this_thread::sleep_for(std::chrono::milliseconds(200));
    tc.duration_to_next_timer();

    return 0;
}