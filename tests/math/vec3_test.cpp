#include <array>
#include <cmath>
#include <concepts>
#include <limits>
#include <doctest/doctest.h>

#include "2iREN/math/vec3.hpp"

using namespace siren;

namespace {
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
}

TEST_SUITE("vec3") {
    TEST_CASE("formatting produces readable text") {
        const auto integer  = Vec3i{-4, 2, 8};
        const auto floating = Vec3f{1.5f, -2.25f, 8.0f};

        CHECK_EQ(integer.to_string(), "Vec3<int>(x=-4, y=2, z=8)");
        CHECK_EQ(floating.to_string(), "Vec3<float>(x=1.5, y=-2.25, z=8)");
    }

    TEST_CASE("aliases expose expected types") {
        static_assert(std::same_as<Vec3f::Type, f32>);
        static_assert(std::same_as<Vec3d::Type, f64>);
        static_assert(std::same_as<Vec3i::Type, i32>);
        static_assert(std::same_as<Vec3u::Type, u32>);
    }

    TEST_CASE("default construction produces zero") {
        const auto vec      = Vec3f{};
        const auto expected = Vec3f{0.0f, 0.0f, 0.0f};

        CHECK_EQ(vec, expected);
    }

    TEST_CASE("uniform construction fills every component") {
        const auto vec      = Vec3f{14.4f};
        const auto expected = Vec3f{14.4f, 14.4f, 14.4f};

        CHECK_EQ(vec, expected);
    }

    TEST_CASE("component construction converts values") {
        const auto vec      = Vec3f{14, 12, -3};
        const auto expected = Vec3f{14.0f, 12.0f, -3.0f};

        CHECK_EQ(vec, expected);
    }

    TEST_CASE("making from data copies every component") {
        const auto values   = std::array{f32{2.5f}, f32{-4.0f}, f32{8.0f}};
        const auto vec      = Vec3f::make(values.data());
        const auto expected = Vec3f{2.5f, -4.0f, 8.0f};

        CHECK_EQ(vec, expected);
    }

    TEST_CASE("constants produce expected vectors") {
        const auto zero         = Vec3i::ZERO();
        const auto one          = Vec3i::ONE();
        const auto negative_one = Vec3i::NEGATIVE_ONE();
        const auto minimum      = Vec3i::MIN();
        const auto maximum      = Vec3i::MAX();
        const auto lowest       = std::numeric_limits<i32>::lowest();
        const auto highest      = std::numeric_limits<i32>::max();
        const auto float_min    = Vec3f::MIN();
        const auto float_lowest = std::numeric_limits<f32>::lowest();

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
        static_assert(!HasVec3NegativeOne<Vec3u>);
    }

    TEST_CASE("directions produce unit vectors") {
        CHECK_EQ(Vec3i::UP(), Vec3i{0, 1, 0});
        CHECK_EQ(Vec3i::DOWN(), Vec3i{0, -1, 0});
        CHECK_EQ(Vec3i::LEFT(), Vec3i{-1, 0, 0});
        CHECK_EQ(Vec3i::RIGHT(), Vec3i{1, 0, 0});
        CHECK_EQ(Vec3i::BACKWARD(), Vec3i{0, 0, -1});
        CHECK_EQ(Vec3i::FORWARD(), Vec3i{0, 0, 1});
    }

    TEST_CASE("equal vectors compare equal") {
        const auto vec       = Vec3i{1, 2, 3};
        const auto equal     = Vec3i{1, 2, 3};
        const auto different = Vec3i{1, 2, 4};

        CHECK_EQ(vec, equal);
        CHECK_NE(vec, different);
    }

    TEST_CASE("adding vectors produces component sums") {
        const auto left      = Vec3i{10, -4, 2};
        const auto right     = Vec3i{3, 6, -5};
        const auto result    = left + right;
        const auto expected  = Vec3i{13, 2, -3};
        auto assigned        = left;
        const auto* returned = &(assigned += right);

        CHECK_EQ(result, expected);
        CHECK_EQ(assigned, expected);
        CHECK_EQ(returned, &assigned);
        CHECK_EQ(left.x, 10);
        CHECK_EQ(left.y, -4);
        CHECK_EQ(left.z, 2);
    }

    TEST_CASE("subtracting vectors produces component differences") {
        const auto left      = Vec3i{10, -4, 2};
        const auto right     = Vec3i{3, 6, -5};
        const auto result    = left - right;
        const auto expected  = Vec3i{7, -10, 7};
        auto assigned        = left;
        const auto* returned = &(assigned -= right);

        CHECK_EQ(result, expected);
        CHECK_EQ(assigned, expected);
        CHECK_EQ(returned, &assigned);
        CHECK_EQ(left.x, 10);
        CHECK_EQ(left.y, -4);
        CHECK_EQ(left.z, 2);
    }

    TEST_CASE("negating a vector flips every sign") {
        const auto vec      = Vec3i{4, -2, 8};
        const auto result   = -vec;
        const auto expected = Vec3i{-4, 2, -8};

        CHECK_EQ(result, expected);
    }

    TEST_CASE("offsetting changes every component") {
        const auto vec        = Vec3i{12, -6, 3};
        const auto added      = vec + 2;
        const auto subtracted = vec - 2;
        auto assigned         = vec;

        const auto* added_return = &(assigned += 2);
        CHECK_EQ(added_return, &assigned);
        CHECK_EQ(assigned, added);

        const auto* subtracted_return = &(assigned -= 2);
        CHECK_EQ(subtracted_return, &assigned);
        CHECK_EQ(assigned, vec);

        const auto added_expected      = Vec3i{14, -4, 5};
        const auto subtracted_expected = Vec3i{10, -8, 1};
        CHECK_EQ(added, added_expected);
        CHECK_EQ(subtracted, subtracted_expected);
    }

    TEST_CASE("scaling changes every component") {
        const auto vec       = Vec3i{10, -4, 2};
        const auto expected  = Vec3i{30, -12, 6};
        auto assigned        = vec;
        const auto* returned = &(assigned *= 3);

        CHECK_EQ(vec * 3, expected);
        CHECK_EQ(3 * vec, expected);
        CHECK_EQ(assigned, expected);
        CHECK_EQ(returned, &assigned);
    }

    TEST_CASE("dividing changes every component") {
        const auto vec       = Vec3i{12, -18, 21};
        const auto expected  = Vec3i{4, -6, 7};
        auto assigned        = vec;
        const auto* returned = &(assigned /= 3);

        CHECK_EQ(vec / 3, expected);
        CHECK_EQ(assigned, expected);
        CHECK_EQ(returned, &assigned);
    }

    TEST_CASE("length measures the vector magnitude") {
        const auto vec    = Vec3i{1, 1, 1};
        const auto length = vec.length();

        static_assert(std::same_as<decltype(vec.length()), f64>);
        CHECK_EQ(length, doctest::Approx(std::sqrt(3.0)));
    }

    TEST_CASE("normalizing produces unit length") {
        const auto vec        = Vec3d{0.0, 3.0, 4.0};
        const auto normalized = Vec3d::normalize(vec);

        static_assert(std::same_as<decltype(Vec3d::normalize(vec)), Vec3d>);
        CHECK_EQ(normalized.x, doctest::Approx(0.0));
        CHECK_EQ(normalized.y, doctest::Approx(0.6));
        CHECK_EQ(normalized.z, doctest::Approx(0.8));
        CHECK_EQ(normalized.length(), doctest::Approx(1.0));
    }

    TEST_CASE("cross product produces perpendicular vectors") {
        const auto right             = Vec3i::RIGHT();
        const auto up                = Vec3i::UP();
        const auto forward           = Vec3i::cross(right, up);
        const auto backward          = Vec3i::cross(up, right);
        const auto expected_forward  = Vec3i{0, 0, 1};
        const auto expected_backward = Vec3i{0, 0, -1};

        CHECK_EQ(forward, expected_forward);
        CHECK_EQ(backward, expected_backward);
    }

    TEST_CASE("dot product measures vector alignment") {
        const auto left  = Vec3i{1, 2, 3};
        const auto right = Vec3i{4, -5, 6};

        CHECK_EQ(Vec3i::dot(left, right), 12);
    }

    TEST_CASE("normalizing zero preserves zero") {
        CHECK_EQ(Vec3f::normalize(Vec3f::ZERO()), Vec3f::ZERO());
    }

    TEST_CASE("vector products stay unavailable") {
        static_assert(!HasVec3Multiply<Vec3i>);
        static_assert(!HasVec3MultiplyAssign<Vec3i>);
        static_assert(!HasVec3Divide<Vec3i>);
        static_assert(!HasVec3DivideAssign<Vec3i>);
    }
}
