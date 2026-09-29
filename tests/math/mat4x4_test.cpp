#include <array>
#include <concepts>
#include <numbers>
#include <span>
#include <utility>
#include <doctest/doctest.h>

#include "2iREN/math/mat4x4.hpp"

using namespace siren;

TEST_SUITE("mat4x4") {
    TEST_CASE("formatting produces readable text") {
        const auto matrix = Mat4x4i{};

        CHECK_EQ(matrix.to_string(), "Mat4x4<int>(\n1 0 0 0\n0 1 0 0\n0 0 1 0\n0 0 0 1\n )");
    }

    TEST_CASE("aliases expose expected types") {
        static_assert(std::same_as<Mat4x4f::Type, f32>);
        static_assert(std::same_as<Mat4x4i::Type, i32>);
        static_assert(std::same_as<Mat4x4u::Type, u32>);
        static_assert(std::same_as<Mat4x4f::Column, std::span<f32, 4>>);
        static_assert(std::same_as<Mat4x4i::Elements, std::array<i32, 16>>);
    }

    TEST_CASE("default construction produces identity") {
        const auto matrix = Mat4x4i{};

        CHECK_EQ(matrix[0][0], 1);
        CHECK_EQ(matrix[1][1], 1);
        CHECK_EQ(matrix[2][2], 1);
        CHECK_EQ(matrix[3][3], 1);
        CHECK_EQ(matrix[0][1], 0);
        CHECK_EQ(matrix[1][2], 0);
        CHECK_EQ(matrix[2][3], 0);
        CHECK_EQ(matrix[3][0], 0);
    }

    TEST_CASE("value construction fills every element") {
        const auto matrix = Mat4x4i{4};

        CHECK_EQ(matrix[0][0], 4);
        CHECK_EQ(matrix[1][1], 4);
        CHECK_EQ(matrix[2][2], 4);
        CHECK_EQ(matrix[3][3], 4);
        CHECK_EQ(matrix[3][0], 4);
    }

    TEST_CASE("column construction preserves values") {
        auto c0 = std::array{i32{1}, i32{2}, i32{3}, i32{4}};
        auto c1 = std::array{i32{5}, i32{6}, i32{7}, i32{8}};
        auto c2 = std::array{i32{9}, i32{10}, i32{11}, i32{12}};
        auto c3 = std::array{i32{13}, i32{14}, i32{15}, i32{16}};
        const auto matrix = Mat4x4i{
            Mat4x4i::Column{c0},
            Mat4x4i::Column{c1},
            Mat4x4i::Column{c2},
            Mat4x4i::Column{c3}
        };

        CHECK_EQ(matrix[0][0], 1);
        CHECK_EQ(matrix[0][3], 4);
        CHECK_EQ(matrix[1][0], 5);
        CHECK_EQ(matrix[1][3], 8);
        CHECK_EQ(matrix[2][0], 9);
        CHECK_EQ(matrix[2][3], 12);
        CHECK_EQ(matrix[3][0], 13);
        CHECK_EQ(matrix[3][3], 16);
    }

    TEST_CASE("element construction preserves values") {
        const auto values =
            Mat4x4i::Elements{0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15};
        const auto matrix = Mat4x4i{values};

        CHECK_EQ(matrix[0][0], 0);
        CHECK_EQ(matrix[0][3], 3);
        CHECK_EQ(matrix[1][0], 4);
        CHECK_EQ(matrix[1][3], 7);
        CHECK_EQ(matrix[2][0], 8);
        CHECK_EQ(matrix[2][3], 11);
        CHECK_EQ(matrix[3][0], 12);
        CHECK_EQ(matrix[3][3], 15);
    }

    TEST_CASE("making from data copies every element") {
        const auto values = std::array{
            i32{0},
            i32{1},
            i32{2},
            i32{3},
            i32{4},
            i32{5},
            i32{6},
            i32{7},
            i32{8},
            i32{9},
            i32{10},
            i32{11},
            i32{12},
            i32{13},
            i32{14},
            i32{15}
        };
        const auto matrix = Mat4x4i::make(values.data());

        CHECK_EQ(matrix[0][0], 0);
        CHECK_EQ(matrix[1][0], 4);
        CHECK_EQ(matrix[2][0], 8);
        CHECK_EQ(matrix[3][0], 12);
        CHECK_EQ(matrix[0][3], 3);
        CHECK_EQ(matrix[1][3], 7);
        CHECK_EQ(matrix[2][3], 11);
        CHECK_EQ(matrix[3][3], 15);
    }

    TEST_CASE("factories produce identity and zero") {
        const auto identity = Mat4x4i::IDENTITY();
        const auto zero     = Mat4x4i::ZERO();

        CHECK_EQ(identity[0][0], 1);
        CHECK_EQ(identity[1][1], 1);
        CHECK_EQ(identity[2][2], 1);
        CHECK_EQ(identity[3][3], 1);
        CHECK_EQ(identity[3][0], 0);
        CHECK_EQ(zero[0][0], 0);
        CHECK_EQ(zero[1][1], 0);
        CHECK_EQ(zero[2][2], 0);
        CHECK_EQ(zero[3][3], 0);
    }

    TEST_CASE("equal matrices compare equal") {
        const auto identity = Mat4x4i::IDENTITY();
        const auto other    = Mat4x4i::translate(identity, Vec3i{1, 2, 3});

        CHECK(identity == Mat4x4i::IDENTITY());
        CHECK(identity != other);
    }

    TEST_CASE("multiplying matrices combines transforms") {
        const auto left =
            Mat4x4i::translate(Mat4x4i::IDENTITY(), Vec3i{2, 3, 4});
        const auto right = Mat4x4i::scale(Mat4x4i::IDENTITY(), Vec3i{5, 6, 7});
        const auto matrix = left * right;

        CHECK_EQ(matrix[0][0], 5);
        CHECK_EQ(matrix[1][1], 6);
        CHECK_EQ(matrix[2][2], 7);
        CHECK_EQ(matrix[3][0], 2);
        CHECK_EQ(matrix[3][1], 3);
        CHECK_EQ(matrix[3][2], 4);
        CHECK_EQ(matrix[3][3], 1);
    }

    TEST_CASE("translating stores the displacement") {
        const auto matrix =
            Mat4x4f::translate(Mat4x4f::IDENTITY(), Vec3f{2.0f, 3.0f, 4.0f});

        CHECK_EQ(matrix[3][0], 2.0f);
        CHECK_EQ(matrix[3][1], 3.0f);
        CHECK_EQ(matrix[3][2], 4.0f);
        CHECK_EQ(matrix[3][3], 1.0f);
    }

    TEST_CASE("scaling changes the diagonal") {
        const auto matrix =
            Mat4x4f::scale(Mat4x4f::IDENTITY(), Vec3f{2.0f, 3.0f, 4.0f});

        CHECK_EQ(matrix[0][0], 2.0f);
        CHECK_EQ(matrix[1][1], 3.0f);
        CHECK_EQ(matrix[2][2], 4.0f);
        CHECK_EQ(matrix[3][3], 1.0f);
    }

    TEST_CASE("rotating changes the selected axes") {
        const auto matrix = Mat4x4f::rotate(
            Mat4x4f::IDENTITY(),
            Radians{std::numbers::pi / 2.0},
            Vec3f{0.0f, 0.0f, 1.0f}
        );

        CHECK(matrix[0][0] == doctest::Approx{0.0f});
        CHECK(matrix[0][1] == doctest::Approx{1.0f});
        CHECK(matrix[1][0] == doctest::Approx{-1.0f});
        CHECK(matrix[1][1] == doctest::Approx{0.0f});
        CHECK(matrix[2][2] == doctest::Approx{1.0f});
        CHECK(matrix[3][3] == doctest::Approx{1.0f});
    }

    TEST_CASE("perspective produces a left handed projection") {
        const auto matrix = Mat4x4f::perspective(
            Radians{std::numbers::pi / 2.0},
            NonZeroPositiveF32{1.0f},
            NonZeroPositiveF32{1.0f},
            NonZeroPositiveF32{10.0f}
        );

        CHECK(matrix[0][0] == doctest::Approx{1.0f});
        CHECK(matrix[1][1] == doctest::Approx{1.0f});
        CHECK(matrix[2][2] == doctest::Approx{10.0f / 9.0f});
        CHECK_EQ(matrix[2][3], 1.0f);
        CHECK(matrix[3][2] == doctest::Approx{-10.0f / 9.0f});
        CHECK_EQ(matrix[3][3], 0.0f);
    }

    TEST_CASE("data exposes contiguous elements") {
        auto matrix = Mat4x4i::IDENTITY();

        static_assert(std::same_as<decltype(matrix.data()), i32*>);
        static_assert(std::same_as<decltype(std::as_const(matrix).data()), const i32*>);

        matrix.data()[12] = 7;
        CHECK_EQ(matrix[3][0], 7);
    }
}
