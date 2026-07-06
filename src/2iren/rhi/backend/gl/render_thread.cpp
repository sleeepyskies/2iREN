#include "render_thread.hpp"

#include <glad/gl.h>
#include <GLFW/glfw3.h>

#include "2iren/window.hpp"


namespace siren {

static auto load_gl_function_pointers(GLFWwindow* window) -> void {
    glfwMakeContextCurrent(window);
    gladLoadGL(glfwGetProcAddress);
}

RenderThread::RenderThread(const Window& window) {
    if constexpr (single_threaded) {
        load_gl_function_pointers(window.handle());
        return;
    }

    auto inner    = m_inner.lock();
    inner->thread = std::thread{ &RenderThread::run, this, window.handle()};
}

RenderThread::~RenderThread() {
    m_terminate = true;
    m_condition.notify_all();

    std::thread render_thread;
    m_inner.run([&] (Inner& inner) {
        render_thread = std::move(inner.thread);
    });

    if (render_thread.joinable()) {
        render_thread.join();
    }
}

auto RenderThread::spawn(RenderTask&& task) const -> void {
    if constexpr (single_threaded) {
        task();
        return;
    }

    m_task_count.fetch_add(1);
    // unlock before notifying so the thread doesn't have to wait
    m_inner.run([&] (Inner& inner) {
        inner.tasks.push(std::move(task));
    });
    m_condition.notify_one();
}

auto RenderThread::wait_until_idle() const noexcept -> void {
    usize remaining = m_task_count.load();

    while (remaining > 0) {
        m_task_count.wait(remaining);
        remaining = m_task_count.load();
    }
}

auto RenderThread::run(GLFWwindow* window) const -> void {
    load_gl_function_pointers(window);

    while (true) {
        std::queue<RenderTask> local_tasks;
        bool stop = false;

        m_inner.run_guarded([&] (UniqueGuard<Inner>& inner) {
            m_condition.wait(inner, [&inner, this] {
                return m_terminate || !inner->tasks.empty();
            });

            if (m_terminate && inner->tasks.empty()) {
                stop = true;
                return;
            }

            // grab all tasks since is only one thread anyway and avoid multiple locks then
            std::swap(local_tasks, inner->tasks);
        });

        if (stop) {
            break;
        }

        while (!local_tasks.empty()) {
            local_tasks.front()(); // <-- we call the fn here to incase u didn't see ()()
            local_tasks.pop();
            m_task_count.fetch_sub(1);
            m_task_count.notify_all();
        }
    }
}

} // namespace siren
