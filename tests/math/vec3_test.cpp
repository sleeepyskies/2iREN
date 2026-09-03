#include <array>
#include <cmath>
#include <concepts>
#include <limits>

#include <doctest/doctest.h>

#include "2iREN/math/vec3.hpp"

template <typename T>
concept HasVec3Multiply = requires(T left, T right) { left * right; };

template <typename T>
concept HasVec3MultiplyAssign = requires(T left, T right) { left *= right; };

template <typename T>
concept HasVec3Divide = requires(T left, T right) { left / right; };

template <typename T>
concept HasVec3DivideAssign = requires(T left, T right) { left /= right; };

template <typename T>
concept HasVec3NegativeOne = requires { T::NEGATIVE_ONE(); };

TEST_SUITE("Vec3") {
    TEST_CASE("format") {
        const auto integer  = siren::Vec3i{-4, 2, 8};
        const auto floating = siren::Vec3f{1.5f, -2.25f, 8.0f};

        CHECK_EQ(integer.to_string(), "Vec3<int>(x=-4, y=2, z=8)");
        CHECK_EQ(floating.to_string(), "Vec3<float>(x=1.5, y=-2.25, z=8)");
    }

    TEST_CASE("types") {
        static_assert(std::same_as<siren::Vec3f::Type, siren::f32>);
        static_assert(std::same_as<siren::Vec3d::Type, siren::f64>);
        static_assert(std::same_as<siren::Vec3i::Type, siren::i32>);
        static_assert(std::same_as<siren::Vec3u::Type, siren::u32>);
    }

    TEST_CASE("default") {
        const auto vec      = siren::Vec3f{};
        const auto expected = siren::Vec3f{0.0f, 0.0f, 0.0f};

        CHECK_EQ(vec, expected);
    }

    TEST_CASE("fill") {
        const auto vec      = siren::Vec3f{14.4f};
        const auto expected = siren::Vec3f{14.4f, 14.4f, 14.4f};

        CHECK_EQ(vec, expected);
    }

    TEST_CASE("construct") {
        const auto vec      = siren::Vec3f{14, 12, -3};
        const auto expected = siren::Vec3f{14.0f, 12.0f, -3.0f};

        CHECK_EQ(vec, expected);
    }

    TEST_CASE("make") {
        const auto values   = std::array{siren::f32{2.5f}, siren::f32{-4.0f}, siren::f32{8.0f}};
        const auto vec      = siren::Vec3f::make(values.data());
        const auto expected = siren::Vec3f{2.5f, -4.0f, 8.0f};

        CHECK_EQ(vec, expected);
    }

    TEST_CASE("constants") {
        const auto zero         = siren::Vec3i::ZERO();
        const auto one          = siren::Vec3i::ONE();
        const auto negative_one = siren::Vec3i::NEGATIVE_ONE();
        const auto minimum      = siren::Vec3i::MIN();
        const auto maximum      = siren::Vec3i::MAX();
        const auto lowest       = std::numeric_limits<siren::i32>::lowest();
        const auto highest      = std::numeric_limits<siren::i32>::max();
        const auto float_min    = siren::Vec3f::MIN();
        const auto float_lowest = std::numeric_limits<siren::f32>::lowest();

        CHECK_EQ(zero.x, 0);
        CHECK_EQ(zero.y, 0);
        CHECK_EQ(zero.z, 0);
        CHECK_EQ(one.x, 1);
        CHECK_EQ(one.y, 1);
        CHECK_EQ(one.z, 1);
        CHECK_EQ(negative_one.x, -1);
        CHECK_EQ(negative_one.y, -1);
        CHECK_EQ(negative_one.z, -1);
        CHECK_EQ(minimum.x, lowest);
        CHECK_EQ(minimum.y, lowest);
        CHECK_EQ(minimum.z, lowest);
        CHECK_EQ(maximum.x, highest);
        CHECK_EQ(maximum.y, highest);
        CHECK_EQ(maximum.z, highest);
        CHECK_EQ(float_min.x, float_lowest);
        CHECK_EQ(float_min.y, float_lowest);
        CHECK_EQ(float_min.z, float_lowest);
        static_assert(!HasVec3NegativeOne<siren::Vec3u>);
    }

    TEST_CASE("directions") {
        const auto up             = siren::Vec3i::UP();
        const auto right          = siren::Vec3i::RIGHT();
        const auto expected_up    = siren::Vec3i{0, 1, 0};
        const auto expected_right = siren::Vec3i{1, 0, 0};

        CHECK_EQ(up, expected_up);
        CHECK_EQ(right, expected_right);
    }

    TEST_CASE("equality") {
        const auto vec       = siren::Vec3i{1, 2, 3};
        const auto equal     = siren::Vec3i{1, 2, 3};
        const auto different = siren::Vec3i{1, 2, 4};

        CHECK_EQ(vec, equal);
        CHECK_NE(vec, different);
    }

    TEST_CASE("add") {
        const auto left      = siren::Vec3i{10, -4, 2};
        const auto right     = siren::Vec3i{3, 6, -5};
        const auto result    = left + right;
        const auto expected  = siren::Vec3i{13, 2, -3};
        auto assigned        = left;
        const auto* returned = &(assigned += right);

        CHECK_EQ(result, expected);
        CHECK_EQ(assigned, expected);
        CHECK_EQ(returned, &assigned);
        CHECK_EQ(left.x, 10);
        CHECK_EQ(left.y, -4);
        CHECK_EQ(left.z, 2);
    }

    TEST_CASE("subtract") {
        const auto left      = siren::Vec3i{10, -4, 2};
        const auto right     = siren::Vec3i{3, 6, -5};
        const auto result    = left - right;
        const auto expected  = siren::Vec3i{7, -10, 7};
        auto assigned        = left;
        const auto* returned = &(assigned -= right);

        CHECK_EQ(result, expected);
        CHECK_EQ(assigned, expected);
        CHECK_EQ(returned, &assigned);
        CHECK_EQ(left.x, 10);
        CHECK_EQ(left.y, -4);
        CHECK_EQ(left.z, 2);
    }

    TEST_CASE("negate") {
        const auto vec      = siren::Vec3i{4, -2, 8};
        const auto result   = -vec;
        const auto expected = siren::Vec3i{-4, 2, -8};

        CHECK_EQ(result, expected);
    }

    TEST_CASE("offset") {
        const auto vec        = siren::Vec3i{12, -6, 3};
        const auto added      = vec + 2;
        const auto subtracted = vec - 2;
        auto assigned         = vec;

        const auto* added_return = &(assigned += 2);
        CHECK_EQ(added_return, &assigned);
        CHECK_EQ(assigned, added);

        const auto* subtracted_return = &(assigned -= 2);
        CHECK_EQ(subtracted_return, &assigned);
        CHECK_EQ(assigned, vec);

        const auto added_expected      = siren::Vec3i{14, -4, 5};
        const auto subtracted_expected = siren::Vec3i{10, -8, 1};
        CHECK_EQ(added, added_expected);
        CHECK_EQ(subtracted, subtracted_expected);
    }

    TEST_CASE("scale") {
        const auto vec       = siren::Vec3i{10, -4, 2};
        const auto expected  = siren::Vec3i{30, -12, 6};
        auto assigned        = vec;
        const auto* returned = &(assigned *= 3);

        CHECK_EQ(vec * 3, expected);
        CHECK_EQ(3 * vec, expected);
        CHECK_EQ(assigned, expected);
        CHECK_EQ(returned, &assigned);
    }

    TEST_CASE("divide") {
        const auto vec       = siren::Vec3i{12, -18, 21};
        const auto expected  = siren::Vec3i{4, -6, 7};
        auto assigned        = vec;
        const auto* returned = &(assigned /= 3);

        CHECK_EQ(vec / 3, expected);
        CHECK_EQ(assigned, expected);
        CHECK_EQ(returned, &assigned);
    }

    TEST_CASE("length") {
        const auto vec    = siren::Vec3i{1, 1, 1};
        const auto length = vec.length();

        static_assert(std::same_as<decltype(vec.length()), siren::f64>);
        CHECK_EQ(length, doctest::Approx(std::sqrt(3.0)));
    }

    TEST_CASE("normalize") {
        const auto vec        = siren::Vec3d{0.0, 3.0, 4.0};
        const auto normalized = siren::Vec3d::normalize(vec);

        static_assert(std::same_as<decltype(siren::Vec3d::normalize(vec)), siren::Vec3d>);
        CHECK_EQ(normalized.x, doctest::Approx(0.0));
        CHECK_EQ(normalized.y, doctest::Approx(0.6));
        CHECK_EQ(normalized.z, doctest::Approx(0.8));
        CHECK_EQ(normalized.length(), doctest::Approx(1.0));
    }

    TEST_CASE("cross") {
        const auto right             = siren::Vec3i::RIGHT();
        const auto up                = siren::Vec3i::UP();
        const auto forward           = siren::Vec3i::cross(right, up);
        const auto backward          = siren::Vec3i::cross(up, right);
        const auto expected_forward  = siren::Vec3i{0, 0, 1};
        const auto expected_backward = siren::Vec3i{0, 0, -1};

        CHECK_EQ(forward, expected_forward);
        CHECK_EQ(backward, expected_backward);
    }

    TEST_CASE("operators") {
        static_assert(!HasVec3Multiply<siren::Vec3i>);
        static_assert(!HasVec3MultiplyAssign<siren::Vec3i>);
        static_assert(!HasVec3Divide<siren::Vec3i>);
        static_assert(!HasVec3DivideAssign<siren::Vec3i>);
    }
}
