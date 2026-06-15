#include "thread_pool.hpp"

#include "2iren/util/log.hpp"


namespace siren {

ThreadPool::ThreadPool(const i32 thread_count) {
    if constexpr (!single_threaded) {
        i32 count;
        if (thread_count < 0) {
            count = std::max(std::jthread::hardware_concurrency() + thread_count, 1u);
        } else {
            count = std::max(1, thread_count);
        }

        log::info("New ThreadPool initialized with {} threads", count);

        auto inner = m_inner.lock();
        for (i32 i = 0; i < count; i++) {
            inner->threads.emplace_back(std::jthread{ &ThreadPool::run, this });
        }
    }
}

ThreadPool::~ThreadPool() {
    m_terminate.store(true);
    m_condition.notify_all();
}

auto ThreadPool::run() -> void {
    Task task;

    while (true) {
        task = m_inner.run_guarded(
            [&] (UniqueGuard<Inner>& inner) -> Task{
                m_condition.wait(
                    inner,
                    [&inner, this]{
                        return m_terminate || !inner->tasks.empty();
                    }
                );
                if (m_terminate && inner->tasks.empty()) {
                    return nullptr;
                }
                task = std::move(inner->tasks.front());
                inner->tasks.pop();
                return std::move(task);
            }
        );

        if (!task && m_terminate) {
            break;
        }

        // avoids stalling other threads while doing something heavy
        if (task) {
            task();
        }
    }
}

} // namespace siren
