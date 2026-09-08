#pragma once

#include <atomic>
#include <functional>
#include <future>
#include <queue>
#include <thread>
#include <vector>

#include "2iREN/core/assert.hpp"
#include "2iREN/concurrency/condition_variable.hpp"
#include "2iREN/concurrency/mutex.hpp"

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
        ASSERT(s_instance != nullptr, "must call ThreadPool::init() before calling ThreadPool::get().");
        return *s_instance;
    }

    /// @brief Initializes the global singleton instance.
    static auto init(
        const i32 thread_count = static_cast<i32>(std::jthread::hardware_concurrency())
    ) -> void {
        s_instance = new ThreadPool(thread_count);
    }

    /// @brief Handles cleanup of the singleton instance.
    static auto shutdown() -> void {
        delete s_instance;
        s_instance = nullptr;
    }

    /**
     * @brief Runs a provided task asynchronously (if siren::single_threaded is false).
     * @note This function returns nothing, so the called must handle results of the function.
     * To receive a future, see ThreadPool::spawn().
     * @tparam Func The function type.
     * @tparam Args The argument types of the function.
     * @param func The function to run.
     * @param args The arguments to provide to the function.
     */
    template <typename Func, typename... Args>
        requires(std::is_invocable_v<Func, Args...>)
    auto submit(Func&& func, Args&&... args) -> void {
        auto work = std::bind(
            std::forward<Func>(func),
            std::forward<Args>(args)...
        );

        auto job = std::make_shared<std::packaged_task<decltype(func(args...))()>>(std::move(work));

        m_inner.run([job](Inner& inner) { 
            inner.jobs.push([job] { 
                (*job)(); 
            }); 
        });
        m_cv.notify_one();
    }

private:
    void worker();

    struct Inner {
        std::vector<std::jthread> threads;
        std::queue<Job> jobs;          
    };

    std::atomic_bool m_terminate = false; 
    ConditionVariable m_cv;
    Mutex<Inner> m_inner;

    static inline ThreadPool* s_instance;
};

} // namespace siren
