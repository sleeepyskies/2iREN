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

/// TODO:
///     the thread pool is currently disabled. this is because until the asset system
///     is rewritten, this will cause crashed by calling opengl functions from worker threads.
///     really, the asset system needs to be simplified and just needs to load intermediary data
///     that can then be used to create GPU resources, but i suspect this may be a large undertaking.

class ThreadPool {
    using Job = std::function<void()>;

public:
    explicit ThreadPool(usize workercount);
    ~ThreadPool();

    ThreadPool(const ThreadPool&)            = delete;
    ThreadPool(ThreadPool&&)                 = delete;
    ThreadPool& operator=(const ThreadPool&) = delete;
    ThreadPool& operator=(ThreadPool&&)      = delete;

    /// @brief Retrieves the singleton instance of this ThreadPool.
    static auto get() -> ThreadPool&;

    /// @brief Initializes the global singleton instance.
    static auto initialize(usize thread_count = std::jthread::hardware_concurrency()) -> void;

    /// @brief Shuts down the ThreadPool.
    static auto shutdown() -> void;

    /// @brief Runs the provided task on a worker thread.
    template <typename Func, typename... Args>
        requires(std::is_invocable_v<Func, Args...>)
    auto submit(Func&& func, Args&&... args) -> void {
        auto work = std::bind(std::forward<Func>(func), std::forward<Args>(args)...);

        work();
        return;

        auto job = std::make_shared<std::packaged_task<decltype(func(args...))()>>(std::move(work));

        m_inner.run([job](Inner& inner) { inner.jobs.push([job] { (*job)(); }); });
        m_cv.notify_one();
    }

private:
    void worker() const;

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
