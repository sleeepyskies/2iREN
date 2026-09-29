#include <concepts>
#include <compare>
#include <doctest/doctest.h>

#include "2iREN/math/bounded.hpp"

using namespace siren;

namespace {
using SmallInteger   = Bounded<i32, 1, 5>;
using ClampedInteger = Bounded<i32, 1, 5, ClampBoundsPolicy>;

template <typename T>
concept SupportsThreeWayComparison = requires(const T& left, const T& right) {
    { left.operator<=>(right) } -> std::same_as<std::strong_ordering>;
};
} // namespace

TEST_SUITE("bounded") {
    TEST_CASE("default construction uses the minimum") {
        const auto value = SmallInteger{};

        CHECK_EQ(value.get(), 1);
    }

    TEST_CASE("value construction preserves an in range value") {
        const auto value = SmallInteger{3};

        CHECK_EQ(value.get(), 3);
    }

    TEST_CASE("setting changes the stored value") {
        auto value = SmallInteger{2};

        value.set(4);

        CHECK_EQ(value.get(), 4);
    }

    TEST_CASE("clamping keeps values inside the bounds") {
        auto below = ClampedInteger{-10};
        auto above = ClampedInteger{10};

        CHECK_EQ(below.get(), 1);
        CHECK_EQ(above.get(), 5);

        below.set(20);
        above.set(-20);
        CHECK_EQ(below.get(), 5);
        CHECK_EQ(above.get(), 1);
    }

    TEST_CASE("bound policies include or exclude endpoints") {
        CHECK(InclusiveBoundsPolicy::check_lower(1, 1));
        CHECK(InclusiveBoundsPolicy::check_upper(5, 5));
        CHECK_FALSE(ExclusiveBoundsPolicy::check_lower(1, 1));
        CHECK_FALSE(ExclusiveBoundsPolicy::check_upper(5, 5));
        CHECK(ExclusiveBoundsPolicy::check_lower(2, 1));
        CHECK(ExclusiveBoundsPolicy::check_upper(4, 5));
    }

    TEST_CASE("conversion produces the underlying value") {
        const auto value     = SmallInteger{3};
        const auto converted = static_cast<i64>(value);

        static_assert(std::same_as<decltype(converted), const i64>);
        CHECK_EQ(converted, 3);
    }

    TEST_CASE("comparing bounded values produces an ordering") {
        CHECK((SupportsThreeWayComparison<SmallInteger>));
    }

    TEST_CASE("subtracting a value produces the difference" * doctest::skip()) {
        const auto value = SmallInteger{3};

        CHECK_EQ(value - 2, 1);
    }

    TEST_CASE("aliases expose their documented limits") {
        static_assert(BoundedI32<1, 5>::MIN == 1);
        static_assert(BoundedI32<1, 5>::MAX == 5);
        static_assert(NonZeroUsize::MIN == 1);
        static_assert(PositiveF32::MIN == 0.0f);
    }
}
