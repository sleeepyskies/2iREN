#include "thread_pool.hpp"

#include "2iREN/utility/log.hpp"

namespace siren {

ThreadPool::ThreadPool(const u32 workercount) {
    log::info("ThreadPool initialized with {} threads.", workercount);

    m_inner.run([&](Inner& inner){
        for (const auto _ : range(workercount)) {
            inner.threads.emplace_back(std::jthread{&ThreadPool::worker, this});
        }
    });
}

ThreadPool::~ThreadPool() {
    m_terminate.store(true);
    m_cv.notify_all();
}

auto ThreadPool::worker() -> void {
    Job job;

    while (true) {
        job = m_inner.run_guarded([&](UniqueGuard<Inner>& inner) -> Job {
            m_cv.wait(inner, [&inner, this] {
                return m_terminate || !inner->jobs.empty();
            });
            if (m_terminate && inner->jobs.empty()) {
                return nullptr;
            }
            job = std::move(inner->jobs.front());
            inner->jobs.pop();
            return std::move(job);
        });

        if (!job && m_terminate) {
            break;
        }

        // avoids stalling other threads while doing something heavy
        if (job) {
            job();
        }
    }
}

} // namespace siren
