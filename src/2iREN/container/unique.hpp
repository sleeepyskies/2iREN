#pragma once

// TODO: one day, would have nicer syntax compared to unique_ptr<T>
// esp for passing around borrows. such as:
//
// auto ptr = Unique<T>::make(bla, ble, blo);
// ptr->do_something();
// function(ptr.view());
// auto view = ptr.view();
// *view.ptr += 1;          // not allowed
// view.ptr = other.view(); // allowed
//
// This would be something like
// Unique<T>::view() -> View<T>;

namespace siren {

namespace impl {

template <typename T>
struct DefaultDelete {
    DefaultDelete() = default;
    auto operator()(T* ptr) const noexcept -> void {
        delete ptr;
    }
};

} // namespace impl

template <typename T>
class View {
    View() = default;

    const T* ptr;
};

template <typename T, typename D = impl::DefaultDelete<T>>
class Unique {

    template <typename... Args>
    [[nodiscard]]
    static constexpr auto make() -> Unique<T, D> {
        return {};
    }

private:
    Unique() = default;

    T* m_ptr = nullptr;
};

} // namespace siren
