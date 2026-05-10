#pragma once

#include <expected>

#include "guard.hpp"


namespace siren {

/** @brief Enum listing all error codes within the @ref RwLock. */
enum class RwlockError {
    ResourceLocked,
};

/** @brief The guard type returned by a @ref RwLock for reading. */
template <typename T>
using ReadGuard = Guard<T, std::shared_lock<std::shared_mutex>>;

/** @brief The guard type returned by a @ref RwLock for writing. */
template <typename T>
using WriteGuard = Guard<T, std::unique_lock<std::shared_mutex>>;

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
    RwLock() : m_data(T()) { }
    template <typename... Args>
    explicit RwLock(Args&&... args) : m_data(T(std::move(args...))) { }
    explicit RwLock(T&& t) : m_data(std::move(t)) { }

    RwLock(const RwLock&)            = delete;
    RwLock(RwLock&&)                 = default;
    RwLock& operator=(const RwLock&) = delete;
    RwLock& operator=(RwLock&&)      = default;

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
        if (!lock.owns_lock()) { return RwlockError::ResourceLocked; }
        return ReadGuard<T>{ std::move(lock), m_data };
    }

    /**
     * @brief Perform a blocking write. If the resource is
     * currently locked, the thread will wait until it is free.
     */
    [[nodiscard]]
    auto write() -> WriteGuard<T> {
        typename WriteGuard<T>::LockType lock{ m_mutex }; // blocking
        return WriteGuard<T>{ std::move(lock), m_data };
    }

    /** @brief Attempts to obtain a @ref WriteGuard. Returns std::unexpected on failure. */
    [[nodiscard]]
    auto try_write() -> std::expected<WriteGuard<T>, RwlockError> {
        typename WriteGuard<T>::LockType lock{ m_mutex, std::try_to_lock };
        if (!lock.owns_lock()) { return std::unexpected(RwlockError::ResourceLocked); }
        return WriteGuard<T>{ std::move(lock), m_data };
    }

private:
    T m_data;
    mutable std::shared_mutex m_mutex;
};

} // namespace siren
