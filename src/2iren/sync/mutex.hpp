#pragma once

#include <expected>

#include "guard.hpp"


namespace siren {

/**
 * @brief Type alias for a lock held on a mutex.
 */
using UniqueMutexLock = std::unique_lock<std::mutex>;

/**
 * @brief Handle to a locked @ref Mutex resource.
 * @details Uses RAII to ensure the underlying mutex is released when the guard
 * goes out of scope. Provides pointer-like access to the wrapped data.
 */
template <typename T>
using UniqueGuard = Guard<T, UniqueMutexLock>;

/** @brief Lists all possible error codes possible for the @ref Mutex. */
enum class MutexError {
    /** @brief The @ref Mutex was already held by another thread. */
    ResourceLocked,
};

/**
 * @brief A thread-safe Monitor container wrapping a resource of type T.
 * @tparam T The type of the resource to protect.
 * @threadsafe This class is safe for concurrent access from multiple threads.
 */
template <typename T>
class Mutex {
public:
    using GuardType    = UniqueGuard<T>;
    using LockType     = GuardType::LockType;
    using ExpectedType = std::expected<GuardType, MutexError>;

    /** @brief Constructs T using its default constructor. */
    Mutex()
        requires(std::is_default_constructible_v<T>)
        : m_data() { }

    /**
     * @brief Constructs T in-place using provided arguments.
     * @param args Arguments forwarded to the constructor of T.
     */
    template <typename... Args>
        requires (!std::is_same_v<std::remove_cvref_t<Args>, Mutex> && ...)
    explicit Mutex(Args... args) : m_data(std::forward<Args>(args)...) { }

    /** @brief Moves an existing T into the protected container. */
    explicit Mutex(T&& t) : m_data(std::move(t)) { }

    /** @brief Copies an existing T into the protected container. */
    explicit Mutex(const T& t) : m_data(t) { }

    Mutex(const Mutex&)             = delete;
    Mutex(Mutex&&)                  = delete;
    Mutex& operator=(const Mutex&)  = delete;
    Mutex& operator=(const Mutex&&) = delete;

    /**
     * @brief Blocks current thread until access is acquired.
     * @return A @ref UniqueGuard providing access to the data.
     * @note This is a blocking operation.
     */
    [[nodiscard]]
    auto lock() const -> GuardType {
        LockType lock{ m_mutex };
        // have to const_cast here since m_data is const, Guard expects a non const value however
        return GuardType{ std::move(lock), const_cast<T&>(m_data) };
    }

    /**
     * @brief Attempts to acquire exclusive access without blocking.
     * @return A @ref UniqueGuard on success, or @ref MutexError if locked.
     */
    [[nodiscard]]
    auto try_lock() const noexcept -> ExpectedType {
        LockType lock{ m_mutex, std::try_to_lock };
        if (!lock.owns_lock()) { return std::unexpected(MutexError::ResourceLocked); }
        return GuardType{ std::move(lock), const_cast<T&>(m_data) };
    }

    /**
     * @brief Executes a callable with a reference to the protected data.
     * @return The result of the callable.
     */
    template <typename Function>
    auto run(Function&& func) const noexcept -> std::invoke_result_t<Function, T&> {
        auto guard = this->lock();
        return func(*guard);
    }

    /**
     * @brief Executes a callable with a guard to the protected data.
     * @return The result of the callable.
     */
    template <typename Function>
    auto guarded(Function&& func) const noexcept -> std::invoke_result_t<Function, UniqueGuard<T>&> {
        auto guard = this->lock();
        return func(guard);
    }

    /**
     * @brief Atomically updates the wrapped value.
     * @param val The new value to assign.
     * @note Thread will block until assignment is complete.
     */
    template <typename U>
    auto set(U&& val) const noexcept -> void { *lock() = std::forward<U>(val); }

    /**
     * @brief Retrieves a copy of the wrapped value.
     * @return A copy of the internal data.
     * @note Thread will block during the copy operation.
     */
    auto get() const noexcept -> T { return *lock(); }

private:
    T m_data;
    mutable std::mutex m_mutex;
};

/**
 * @brief Template specialization for a void @ref Mutex.
 * Provides unique access to void. May be useful in situations where
 * no resource guarding is needed, by synchronization is still a requirement.
 */
template <>
class Mutex<void> {
public:
    using LockType     = UniqueMutexLock;
    using ExpectedType = std::expected<LockType, MutexError>;

    /** @brief Constructs a new void mutex. */
    Mutex() = default;

    Mutex(const Mutex&)             = delete;
    Mutex(Mutex&&)                  = delete;
    Mutex& operator=(const Mutex&)  = delete;
    Mutex& operator=(const Mutex&&) = delete;

    /**
     * @brief Blocks current thread until access is acquired.
     * @return A @ref UniqueGuard providing access to the data.
     * @note This is a blocking operation.
     */
    [[nodiscard]]
    auto lock() const -> LockType { return LockType{ m_mutex }; }

    /**
     * @brief Attempts to acquire exclusive access without blocking.
     * @return A @ref UniqueGuard on success, or @ref MutexError if locked.
     */
    [[nodiscard]]
    auto try_lock() const noexcept -> ExpectedType {
        LockType lock{ m_mutex, std::try_to_lock };
        if (!lock.owns_lock()) { return std::unexpected(MutexError::ResourceLocked); }
        return lock;
    }

    /**
     * @brief Executes a callable with a reference to the protected data.
     * @return The result of the callable.
     */
    template <typename Function>
    auto run(Function&& func) const noexcept -> std::invoke_result_t<Function> {
        auto lock = this->lock();
        return func();
    }

private:
    mutable std::mutex m_mutex;
};

} // namespace siren
