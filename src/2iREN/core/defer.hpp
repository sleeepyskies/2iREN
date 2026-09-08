#pragma once

namespace siren {

namespace impl {
template <typename DeferFn>
struct Defer {
    Defer(DeferFn f) : f(f) { }
    ~Defer() {
        f();
    }

    DeferFn f;
};
} // namespace impl

#define __STR_CONCAT(a, b) a##b
#define STR_CONCAT(a, b) __STR_CONCAT(a, b)

#define DEFER(x)                                                                                   \
    const auto STR_CONCAT(tmp_defer_var_name__, __LINE__) = impl::Defer {                          \
        [&]() { x; }                                                                               \
    }

} // namespace siren
