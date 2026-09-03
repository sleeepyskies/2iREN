#include <array>
#include <concepts>
#include <limits>

#include <doctest/doctest.h>

#include "2iREN/math/vec2.hpp"

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

TEST_SUITE("Vec2") {
    TEST_CASE("format") {
        const auto integer  = siren::Vec2i{-4, 2};
        const auto floating = siren::Vec2f{1.5f, -2.25f};

        CHECK_EQ(integer.to_string(), "Vec2<int>(x=-4, y=2)");
        CHECK_EQ(floating.to_string(), "Vec2<float>(x=1.5, y=-2.25)");
    }

    TEST_CASE("types") {
        static_assert(std::same_as<siren::Vec2f::Type, siren::f32>);
        static_assert(std::same_as<siren::Vec2d::Type, siren::f64>);
        static_assert(std::same_as<siren::Vec2i::Type, siren::i32>);
        static_assert(std::same_as<siren::Vec2u::Type, siren::u32>);
    }

    TEST_CASE("default") {
        const auto vec      = siren::Vec2f{};
        const auto expected = siren::Vec2f{0.0f, 0.0f};

        CHECK_EQ(vec, expected);
    }

    TEST_CASE("fill") {
        const auto vec      = siren::Vec2f{4.5f};
        const auto expected = siren::Vec2f{4.5f, 4.5f};

        CHECK_EQ(vec, expected);
    }

    TEST_CASE("construct") {
        const auto vec      = siren::Vec2f{2, -4};
        const auto expected = siren::Vec2f{2.0f, -4.0f};

        CHECK_EQ(vec, expected);
    }

    TEST_CASE("make") {
        const auto values   = std::array{siren::f32{2.5f}, siren::f32{-4.0f}};
        const auto vec      = siren::Vec2f::make(values.data());
        const auto expected = siren::Vec2f{2.5f, -4.0f};

        CHECK_EQ(vec, expected);
    }

    TEST_CASE("constants") {
        const auto zero         = siren::Vec2i::ZERO();
        const auto one          = siren::Vec2i::ONE();
        const auto negative_one = siren::Vec2i::NEGATIVE_ONE();
        const auto minimum      = siren::Vec2i::MIN();
        const auto maximum      = siren::Vec2i::MAX();
        const auto lowest       = std::numeric_limits<siren::i32>::lowest();
        const auto highest      = std::numeric_limits<siren::i32>::max();
        const auto float_min    = siren::Vec2f::MIN();
        const auto float_lowest = std::numeric_limits<siren::f32>::lowest();

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
        static_assert(!HasVec2NegativeOne<siren::Vec2u>);
    }

    TEST_CASE("equality") {
        const auto vec       = siren::Vec2i{2, -4};
        const auto equal     = siren::Vec2i{2, -4};
        const auto different = siren::Vec2i{2, 4};

        CHECK_EQ(vec, equal);
        CHECK_NE(vec, different);
    }

    TEST_CASE("add") {
        const auto left      = siren::Vec2i{4, -2};
        const auto right     = siren::Vec2i{3, 5};
        const auto result    = left + right;
        const auto expected  = siren::Vec2i{7, 3};
        auto assigned        = left;
        const auto* returned = &(assigned += right);

        CHECK_EQ(result, expected);
        CHECK_EQ(assigned, expected);
        CHECK_EQ(returned, &assigned);
        CHECK_EQ(left.x, 4);
        CHECK_EQ(left.y, -2);
    }

    TEST_CASE("subtract") {
        const auto left      = siren::Vec2i{4, -2};
        const auto right     = siren::Vec2i{3, 5};
        const auto result    = left - right;
        const auto expected  = siren::Vec2i{1, -7};
        auto assigned        = left;
        const auto* returned = &(assigned -= right);

        CHECK_EQ(result, expected);
        CHECK_EQ(assigned, expected);
        CHECK_EQ(returned, &assigned);
        CHECK_EQ(left.x, 4);
        CHECK_EQ(left.y, -2);
    }

    TEST_CASE("negate") {
        const auto vec      = siren::Vec2i{4, -2};
        const auto result   = -vec;
        const auto expected = siren::Vec2i{-4, 2};

        CHECK_EQ(result, expected);
    }

    TEST_CASE("offset") {
        const auto vec        = siren::Vec2i{4, -2};
        const auto added      = vec + 3;
        const auto subtracted = vec - 3;
        auto assigned         = vec;

        const auto* added_return = &(assigned += 3);
        CHECK_EQ(added_return, &assigned);
        CHECK_EQ(assigned, added);

        const auto* subtracted_return = &(assigned -= 3);
        CHECK_EQ(subtracted_return, &assigned);
        CHECK_EQ(assigned, vec);

        const auto added_expected      = siren::Vec2i{7, 1};
        const auto subtracted_expected = siren::Vec2i{1, -5};
        CHECK_EQ(added, added_expected);
        CHECK_EQ(subtracted, subtracted_expected);
    }

    TEST_CASE("scale") {
        const auto vec       = siren::Vec2i{4, -2};
        const auto expected  = siren::Vec2i{12, -6};
        auto assigned        = vec;
        const auto* returned = &(assigned *= 3);

        CHECK_EQ(vec * 3, expected);
        CHECK_EQ(3 * vec, expected);
        CHECK_EQ(assigned, expected);
        CHECK_EQ(returned, &assigned);
    }

    TEST_CASE("divide") {
        const auto vec       = siren::Vec2i{12, -18};
        const auto expected  = siren::Vec2i{4, -6};
        auto assigned        = vec;
        const auto* returned = &(assigned /= 3);

        CHECK_EQ(vec / 3, expected);
        CHECK_EQ(assigned, expected);
        CHECK_EQ(returned, &assigned);
    }

    TEST_CASE("operators") {
        static_assert(!HasVec2Multiply<siren::Vec2i>);
        static_assert(!HasVec2MultiplyAssign<siren::Vec2i>);
        static_assert(!HasVec2Divide<siren::Vec2i>);
        static_assert(!HasVec2DivideAssign<siren::Vec2i>);
    }
}
