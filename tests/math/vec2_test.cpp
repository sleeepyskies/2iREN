#include <array>
#include <concepts>
#include <limits>
#include <doctest/doctest.h>

#include "2iREN/math/vec2.hpp"

using namespace siren;

namespace {
template <typename T>
concept HasVec2Multiply = requires(T left, T right) { left * right; };

template <typename T>
concept HasVec2MultiplyAssign = requires(T left, T right) { left *= right; };

template <typename T>
concept HasVec2Divide = requires(T left, T right) { left / right; };

template <typename T>
concept HasVec2DivideAssign = requires(T left, T right) { left /= right; };

template <typename T>
concept HasVec2NegativeOne = requires { T::NEGATIVE_ONE(); };
}


TEST_SUITE("vec2") {
    TEST_CASE("formatting produces readable text") {
        const auto integer  = Vec2i{-4, 2};
        const auto floating = Vec2f{1.5f, -2.25f};

        CHECK_EQ(integer.to_string(), "Vec2<int>(x=-4, y=2)");
        CHECK_EQ(floating.to_string(), "Vec2<float>(x=1.5, y=-2.25)");
    }

    TEST_CASE("aliases expose expected types") {
        static_assert(std::same_as<Vec2f::Type, f32>);
        static_assert(std::same_as<Vec2d::Type, f64>);
        static_assert(std::same_as<Vec2i::Type, i32>);
        static_assert(std::same_as<Vec2u::Type, u32>);
    }

    TEST_CASE("default construction produces zero") {
        const auto vec      = Vec2f{};
        const auto expected = Vec2f{0.0f, 0.0f};

        CHECK_EQ(vec, expected);
    }

    TEST_CASE("uniform construction fills both components") {
        const auto vec      = Vec2f{4.5f};
        const auto expected = Vec2f{4.5f, 4.5f};

        CHECK_EQ(vec, expected);
    }

    TEST_CASE("component construction converts values") {
        const auto vec      = Vec2f{2, -4};
        const auto expected = Vec2f{2.0f, -4.0f};

        CHECK_EQ(vec, expected);
    }

    TEST_CASE("making from data copies both components") {
        const auto values   = std::array{f32{2.5f}, f32{-4.0f}};
        const auto vec      = Vec2f::make(values.data());
        const auto expected = Vec2f{2.5f, -4.0f};

        CHECK_EQ(vec, expected);
    }

    TEST_CASE("constants produce expected vectors") {
        const auto zero         = Vec2i::ZERO();
        const auto one          = Vec2i::ONE();
        const auto negative_one = Vec2i::NEGATIVE_ONE();
        const auto minimum      = Vec2i::MIN();
        const auto maximum      = Vec2i::MAX();
        const auto lowest       = std::numeric_limits<i32>::lowest();
        const auto highest      = std::numeric_limits<i32>::max();
        const auto float_min    = Vec2f::MIN();
        const auto float_lowest = std::numeric_limits<f32>::lowest();

        CHECK_EQ(zero.x, 0);
        CHECK_EQ(zero.y, 0);
        CHECK_EQ(one.x, 1);
        CHECK_EQ(one.y, 1);
        CHECK_EQ(negative_one.x, -1);
        CHECK_EQ(negative_one.y, -1);
        CHECK_EQ(minimum.x, lowest);
        CHECK_EQ(minimum.y, lowest);
        CHECK_EQ(maximum.x, highest);
        CHECK_EQ(maximum.y, highest);
        CHECK_EQ(float_min.x, float_lowest);
        CHECK_EQ(float_min.y, float_lowest);
        static_assert(!HasVec2NegativeOne<Vec2u>);
    }

    TEST_CASE("equal vectors compare equal") {
        const auto vec       = Vec2i{2, -4};
        const auto equal     = Vec2i{2, -4};
        const auto different = Vec2i{2, 4};

        CHECK_EQ(vec, equal);
        CHECK_NE(vec, different);
    }

    TEST_CASE("adding vectors produces component sums") {
        const auto left      = Vec2i{4, -2};
        const auto right     = Vec2i{3, 5};
        const auto result    = left + right;
        const auto expected  = Vec2i{7, 3};
        auto assigned        = left;
        const auto* returned = &(assigned += right);

        CHECK_EQ(result, expected);
        CHECK_EQ(assigned, expected);
        CHECK_EQ(returned, &assigned);
        CHECK_EQ(left.x, 4);
        CHECK_EQ(left.y, -2);
    }

    TEST_CASE("subtracting vectors produces component differences") {
        const auto left      = Vec2i{4, -2};
        const auto right     = Vec2i{3, 5};
        const auto result    = left - right;
        const auto expected  = Vec2i{1, -7};
        auto assigned        = left;
        const auto* returned = &(assigned -= right);

        CHECK_EQ(result, expected);
        CHECK_EQ(assigned, expected);
        CHECK_EQ(returned, &assigned);
        CHECK_EQ(left.x, 4);
        CHECK_EQ(left.y, -2);
    }

    TEST_CASE("negating a vector flips every sign") {
        const auto vec      = Vec2i{4, -2};
        const auto result   = -vec;
        const auto expected = Vec2i{-4, 2};

        CHECK_EQ(result, expected);
    }

    TEST_CASE("offsetting changes every component") {
        const auto vec        = Vec2i{4, -2};
        const auto added      = vec + 3;
        const auto subtracted = vec - 3;
        auto assigned         = vec;

        const auto* added_return = &(assigned += 3);
        CHECK_EQ(added_return, &assigned);
        CHECK_EQ(assigned, added);

        const auto* subtracted_return = &(assigned -= 3);
        CHECK_EQ(subtracted_return, &assigned);
        CHECK_EQ(assigned, vec);

        const auto added_expected      = Vec2i{7, 1};
        const auto subtracted_expected = Vec2i{1, -5};
        CHECK_EQ(added, added_expected);
        CHECK_EQ(subtracted, subtracted_expected);
    }

    TEST_CASE("scaling changes every component") {
        const auto vec       = Vec2i{4, -2};
        const auto expected  = Vec2i{12, -6};
        auto assigned        = vec;
        const auto* returned = &(assigned *= 3);

        CHECK_EQ(vec * 3, expected);
        CHECK_EQ(3 * vec, expected);
        CHECK_EQ(assigned, expected);
        CHECK_EQ(returned, &assigned);
    }

    TEST_CASE("dividing changes every component") {
        const auto vec       = Vec2i{12, -18};
        const auto expected  = Vec2i{4, -6};
        auto assigned        = vec;
        const auto* returned = &(assigned /= 3);

        CHECK_EQ(vec / 3, expected);
        CHECK_EQ(assigned, expected);
        CHECK_EQ(returned, &assigned);
    }

    TEST_CASE("vector products stay unavailable") {
        static_assert(!HasVec2Multiply<Vec2i>);
        static_assert(!HasVec2MultiplyAssign<Vec2i>);
        static_assert(!HasVec2Divide<Vec2i>);
        static_assert(!HasVec2DivideAssign<Vec2i>);
    }
}
