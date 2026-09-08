#pragma once

#include <atomic>
#include <functional>
#include <future>
#include <queue>
#include <thread>
#include <vector>

#include "2iREN/concurrency/condition_variable.hpp"
#include "2iREN/concurrency/mutex.hpp"
#include "2iREN/core/assert.hpp"

namespace siren {

class ThreadPool {
    using Job = std::function<void()>;

public:
    explicit ThreadPool(u32 workercount);
    ~ThreadPool();

    ThreadPool(const ThreadPool&)            = delete;
    ThreadPool(ThreadPool&&)                 = delete;
    ThreadPool& operator=(const ThreadPool&) = delete;
    ThreadPool& operator=(ThreadPool&&)      = delete;

    /// @brief Retrieves the singleton instance of this ThreadPool.
    static auto get() -> ThreadPool& {
        ASSERT(
            s_instance != nullptr, "must call ThreadPool::init() before calling ThreadPool::get()."
        );
        return *s_instance;
    }

    /// @brief Initializes the global singleton instance.
    static auto initialize(
        const i32 thread_count = static_cast<i32>(std::jthread::hardware_concurrency())
    ) -> void {
        s_instance = new ThreadPool(thread_count);
    }

    /// @brief Shutsdown the ThreadPool.
    static auto shutdown() -> void;

    /// @brief Runs the provided task on a worker thread.
    template <typename Func, typename... Args>
        requires(std::is_invocable_v<Func, Args...>)
    auto submit(Func&& func, Args&&... args) -> void {
        auto work = std::bind(std::forward<Func>(func), std::forward<Args>(args)...);

        auto job = std::make_shared<std::packaged_task<decltype(func(args...))()>>(std::move(work));

        m_inner.run([job](Inner& inner) { inner.jobs.push([job] { (*job)(); }); });
        m_cv.notify_one();
    }

private:
    void worker();

    struct Inner {
        std::vector<std::jthread> workers;
        std::queue<Job> jobs;
    };

    std::atomic_bool m_terminate = false;
    ConditionVariable m_cv;
    Mutex<Inner> m_inner;

    static inline ThreadPool* s_instance;
};

} // namespace siren
