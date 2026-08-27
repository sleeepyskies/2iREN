#pragma once

#include <expected>
#include <functional>
#include <mutex>

#include "guard.hpp"


namespace siren {

/** @brief The guard type returned by a @ref RwLock for reading. */
template <typename T>
using ReadGuard = Guard<T, std::shared_lock<std::shared_mutex>>;

/** @brief The guard type returned by a @ref RwLock for writing. */
template <typename T>
using WriteGuard = Guard<T, std::unique_lock<std::shared_mutex>>;

/** @brief Enum listing all error codes within the @ref RwLock. */
enum class RwlockError {
    /** @brief The @ref RwLock was already held by another thread. */
    ResourceLocked,
};

/**
 * @class RwLock
 * @brief A thread safe container allowing multiple threads read and write
 * access to some shared resource.
 * @details This lock allows any number of readers to acquire a lock on
 * the wrapped resource, but only one writer to have a lock.
 * @tparam T The wrapped resource.
 */
template <typename T>
class RwLock {
public:
    /** @brief Constructs T using its default constructor. */
    RwLock()
        requires(std::is_default_constructible_v<T>)
        : m_data(T()) { }

    /**
     * @brief Constructs T in-place using provided arguments.
     * @param args Arguments forwarded to the constructor of T.
     */
    template <typename... Args>
        requires (!std::is_same_v<std::remove_cvref_t<Args>, RwLock> && ...)
    explicit RwLock(Args... args) : m_data(std::forward<Args>(args)...) { }

    /** @brief Moves an existing T into the protected container. */
    explicit RwLock(T&& t) : m_data(std::move(t)) { }

    /** @brief Copies an existing T into the protected container. */
    explicit RwLock(const T& t) : m_data(t) { }

    RwLock(const RwLock&)            = delete;
    RwLock(RwLock&&)                 = delete;
    RwLock& operator=(const RwLock&) = delete;
    RwLock& operator=(RwLock&&)      = delete;

    /**
     * @brief Perform a blocking read. If the resource is currently
     * locked with a Write, the thread will wait until it is freed.
     */
    [[nodiscard]]
    auto read() const -> ReadGuard<T> {
        typename ReadGuard<T>::LockType lock{ m_mutex }; // blocking
        return ReadGuard<T>{ std::move(lock), m_data };
    }

    /** @brief Attempts to obtain a @ref ReadGuard. Returns std::unexpected on failure. */
    [[nodiscard]]
    auto try_read() const -> std::expected<ReadGuard<T>, RwlockError> {
        typename ReadGuard<T>::LockType lock{ m_mutex, std::try_to_lock };
        if (!lock.owns_lock()) { return std::unexpected(RwlockError::ResourceLocked); }
        return ReadGuard<T>{ std::move(lock), m_data };
    }

    /**
     * @brief Perform a blocking write. If the resource is
     * currently locked, the thread will wait until it is free.
     */
    [[nodiscard]] auto write() -> WriteGuard<T> {
        typename WriteGuard<T>::LockType lock{ m_mutex }; // blocking
        return WriteGuard<T>{ std::move(lock), m_data };
    }

    /** @brief Attempts to obtain a @ref WriteGuard. Returns std::unexpected on failure. */
    [[nodiscard]] auto try_write() -> std::expected<WriteGuard<T>, RwlockError> {
        typename WriteGuard<T>::LockType lock{ m_mutex, std::try_to_lock };
        if (!lock.owns_lock()) { return std::unexpected(RwlockError::ResourceLocked); }
        return WriteGuard<T>{ std::move(lock), m_data };
    }

    /**
     * @brief Runs the given lambda with a non-exclusive guard.
     * @tparam Function A lambda that takes the guard as an argument.
     * @return The result of calling function with a non-exclusive guard.
     */
    template <typename Function>
        requires(std::is_invocable_v<Function, const T&>)
    auto run(Function&& function) const -> std::invoke_result_t<Function, const T&> {
        auto guard = read();
        return std::invoke(std::forward<Function>(function), *guard);
    }

    /**
     * @brief Runs the given lambda with an exclusive guard.
     * @tparam Function A lambda that takes the guard as an argument.
     * @return The result of calling function with an exclusive guard.
     */
    template <typename Function>
        requires(std::is_invocable_v<Function, T&>)
    auto run_exclusive(Function&& function) -> std::invoke_result_t<Function, T&> {
        auto guard = write();
        return std::invoke(std::forward<Function>(function), *guard);
    }

private:
    T m_data;
    mutable std::shared_mutex m_mutex;
};

} // namespace siren
