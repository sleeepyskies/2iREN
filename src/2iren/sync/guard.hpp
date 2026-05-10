#pragma once

#include <shared_mutex>


namespace siren {

namespace detail {
template <typename L>
struct IsSharedLock : std::false_type { };

template <typename M>
struct IsSharedLock<std::shared_lock<M>> : std::true_type { };
}


/**
 * @class Guard
 * @brief A RAII container for accessing shared data in a thread safe manner.
 * The lock/guard is held until this object is dropped.
 * @todo Guard poisoning?
 * @tparam T The type of the data behind the guard.
 * @tparam Lock The type of the lock. Maybe either a shared or a unique lock.
 */
template <typename T, typename Lock>
class Guard {
public:
    static constexpr bool IsReadonly = detail::IsSharedLock<Lock>::value;

    using Resource  = std::remove_cvref_t<T>;
    using Pointer   = std::conditional_t<IsReadonly, const Resource*, Resource*>;
    using Reference = std::conditional_t<IsReadonly, const Resource&, Resource&>;
    using LockType  = Lock;

    explicit Guard(
        Lock&& lock,
        Reference data
    ) : m_lock(std::move(lock)), m_ptr(&data) { }

    Guard(const Guard&)            = delete;
    Guard(Guard&&)                 = default;
    Guard& operator=(const Guard&) = delete;
    Guard& operator=(Guard&&)      = default;

    [[nodiscard]] constexpr auto operator->() noexcept -> Pointer { return m_ptr; }
    [[nodiscard]] constexpr auto operator*() noexcept -> Reference { return *m_ptr; }

private:
    friend class ConditionVariable;
    LockType m_lock;
    Pointer m_ptr;
};

} // namespace siren
