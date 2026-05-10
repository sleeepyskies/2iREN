#pragma once

#include <condition_variable>

#include "2iren/util/concepts.hpp"
#include "mutex.hpp"


namespace siren {

/**
 * @class ConditionVariable
 * @brief A synchronization primitive used to block a thread until a particular condition is met.
 * @details The ConditionVariable works in together with @ref Mutex and @ref UniqueGuard.
 * It allows a thread to sleep while waiting for another thread to modify shared
 * data and signal the change.
 */
class ConditionVariable {
public:
    ConditionVariable()                                    = default;
    ConditionVariable(const ConditionVariable&)            = delete;
    ConditionVariable& operator=(const ConditionVariable&) = delete;

    /**
     * @brief Blocks the current thread until the predicate is satisfied.
     * @details This function atomcially unlocks the mutex and puts the thread to
     * sleep. When notified, the thread reacquires the lock and evaluates the
     * predicate. If the predicate returns false, the thread resumes sleeping.
     * @tparam T The resource type managed by the Mutex.
     * @tparam P A callable type that returns a value convertible to bool.
     * @param guard The UniqueGuard providing exclusive access to the resource.
     * @param pred A predicate function invoked to check if the thread should stop waiting.
     */
    template <typename T, IsPredicate P>
    auto wait(UniqueGuard<T>& guard, P&& pred) const -> void {
        m_condition.wait(guard.m_lock, std::forward<P>(pred));
    }

    /** @brief Wakes up one thread currently waiting on this condition. */
    auto notify_one() const -> void { m_condition.notify_one(); }

    /** @brief Wakes up all threads currently waiting on this condition. */
    auto notify_all() const -> void { m_condition.notify_all(); }

private:
    mutable std::condition_variable m_condition;
};

} // namespace siren
