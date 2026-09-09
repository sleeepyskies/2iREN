/// @brief macro for defering cleanup work.
/// Inspired by: https://gist.github.com/p2004a/045726d70a490d12ad62
#pragma once

#include <utility>

namespace siren {

namespace impl {

template <typename F>
struct Deferred {
    constexpr Deferred(F f) : f(f) { }
    ~Deferred() {
        f();
    }
    F f;
};

struct {
    template <typename F>
    constexpr auto operator<<(F&& f) const noexcept -> const Deferred<F> {
        return Deferred<F>(std::forward<F>(f));
    }
} inline constexpr deferer__;

} // namespace impl

#define STR_CONCAT__(a, b) a##b
#define STR_CONCAT_(a, b) STR_CONCAT__(a, b)

#define DEFER const auto STR_CONCAT_(defer_tmp_name_, __LINE__) = impl::deferer__ << [&]()

} // namespace siren
