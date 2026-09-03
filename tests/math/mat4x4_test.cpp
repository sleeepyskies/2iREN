#include <array>
#include <concepts>
#include <numbers>
#include <span>

#include <doctest/doctest.h>

#include "2iREN/math/mat4x4.hpp"

TEST_SUITE("Mat4x4") {
    TEST_CASE("format") {
        const auto matrix = siren::Mat4x4i{};

        CHECK_EQ(matrix.to_string(), "Mat4x4<int>(\n1 0 0 0\n0 1 0 0\n0 0 1 0\n0 0 0 1\n )");
    }

    TEST_CASE("types") {
        static_assert(std::same_as<siren::Mat4x4f::Type, siren::f32>);
        static_assert(std::same_as<siren::Mat4x4i::Type, siren::i32>);
        static_assert(std::same_as<siren::Mat4x4u::Type, siren::u32>);
        static_assert(std::same_as<siren::Mat4x4f::Column, std::span<siren::f32, 4>>);
        static_assert(std::same_as<siren::Mat4x4i::Elements, std::array<siren::i32, 16>>);
    }

    TEST_CASE("default") {
        const auto matrix = siren::Mat4x4i{};

        CHECK_EQ(matrix[0][0], 1);
        CHECK_EQ(matrix[1][1], 1);
        CHECK_EQ(matrix[2][2], 1);
        CHECK_EQ(matrix[3][3], 1);
        CHECK_EQ(matrix[0][1], 0);
        CHECK_EQ(matrix[1][2], 0);
        CHECK_EQ(matrix[2][3], 0);
        CHECK_EQ(matrix[3][0], 0);
    }

    TEST_CASE("value") {
        const auto matrix = siren::Mat4x4i{4};

        CHECK_EQ(matrix[0][0], 4);
        CHECK_EQ(matrix[1][1], 4);
        CHECK_EQ(matrix[2][2], 4);
        CHECK_EQ(matrix[3][3], 4);
        CHECK_EQ(matrix[3][0], 4);
    }

    TEST_CASE("columns") {
        auto c0 = std::array{siren::i32{1}, siren::i32{2}, siren::i32{3}, siren::i32{4}};
        auto c1 = std::array{siren::i32{5}, siren::i32{6}, siren::i32{7}, siren::i32{8}};
        auto c2 = std::array{siren::i32{9}, siren::i32{10}, siren::i32{11}, siren::i32{12}};
        auto c3 = std::array{siren::i32{13}, siren::i32{14}, siren::i32{15}, siren::i32{16}};
        const auto matrix = siren::Mat4x4i{
            siren::Mat4x4i::Column{c0},
            siren::Mat4x4i::Column{c1},
            siren::Mat4x4i::Column{c2},
            siren::Mat4x4i::Column{c3}
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

    TEST_CASE("elements") {
        const auto values =
            siren::Mat4x4i::Elements{0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15};
        const auto matrix = siren::Mat4x4i{values};

        CHECK_EQ(matrix[0][0], 0);
        CHECK_EQ(matrix[0][3], 3);
        CHECK_EQ(matrix[1][0], 4);
        CHECK_EQ(matrix[1][3], 7);
        CHECK_EQ(matrix[2][0], 8);
        CHECK_EQ(matrix[2][3], 11);
        CHECK_EQ(matrix[3][0], 12);
        CHECK_EQ(matrix[3][3], 15);
    }

    TEST_CASE("make") {
        const auto values = std::array{
            siren::i32{0},
            siren::i32{1},
            siren::i32{2},
            siren::i32{3},
            siren::i32{4},
            siren::i32{5},
            siren::i32{6},
            siren::i32{7},
            siren::i32{8},
            siren::i32{9},
            siren::i32{10},
            siren::i32{11},
            siren::i32{12},
            siren::i32{13},
            siren::i32{14},
            siren::i32{15}
        };
        const auto matrix = siren::Mat4x4i::make(values.data());

        CHECK_EQ(matrix[0][0], 0);
        CHECK_EQ(matrix[1][0], 4);
        CHECK_EQ(matrix[2][0], 8);
        CHECK_EQ(matrix[3][0], 12);
        CHECK_EQ(matrix[0][3], 3);
        CHECK_EQ(matrix[1][3], 7);
        CHECK_EQ(matrix[2][3], 11);
        CHECK_EQ(matrix[3][3], 15);
    }

    TEST_CASE("factories") {
        const auto identity = siren::Mat4x4i::IDENTITY();
        const auto zero     = siren::Mat4x4i::ZERO();

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

    TEST_CASE("equality") {
        const auto identity = siren::Mat4x4i::IDENTITY();
        const auto other    = siren::Mat4x4i::translate(identity, siren::Vec3i{1, 2, 3});

        CHECK(identity == siren::Mat4x4i::IDENTITY());
        CHECK(identity != other);
    }

    TEST_CASE("multiply") {
        const auto left =
            siren::Mat4x4i::translate(siren::Mat4x4i::IDENTITY(), siren::Vec3i{2, 3, 4});
        const auto right = siren::Mat4x4i::scale(siren::Mat4x4i::IDENTITY(), siren::Vec3i{5, 6, 7});
        const auto matrix = left * right;

        CHECK_EQ(matrix[0][0], 5);
        CHECK_EQ(matrix[1][1], 6);
        CHECK_EQ(matrix[2][2], 7);
        CHECK_EQ(matrix[3][0], 2);
        CHECK_EQ(matrix[3][1], 3);
        CHECK_EQ(matrix[3][2], 4);
        CHECK_EQ(matrix[3][3], 1);
    }

    TEST_CASE("translate") {
        const auto matrix =
            siren::Mat4x4f::translate(siren::Mat4x4f::IDENTITY(), siren::Vec3f{2.0f, 3.0f, 4.0f});

        CHECK_EQ(matrix[3][0], 2.0f);
        CHECK_EQ(matrix[3][1], 3.0f);
        CHECK_EQ(matrix[3][2], 4.0f);
        CHECK_EQ(matrix[3][3], 1.0f);
    }

    TEST_CASE("scale") {
        const auto matrix =
            siren::Mat4x4f::scale(siren::Mat4x4f::IDENTITY(), siren::Vec3f{2.0f, 3.0f, 4.0f});

        CHECK_EQ(matrix[0][0], 2.0f);
        CHECK_EQ(matrix[1][1], 3.0f);
        CHECK_EQ(matrix[2][2], 4.0f);
        CHECK_EQ(matrix[3][3], 1.0f);
    }

    TEST_CASE("rotate") {
        const auto matrix = siren::Mat4x4f::rotate(
            siren::Mat4x4f::IDENTITY(),
            siren::Radians{std::numbers::pi / 2.0},
            siren::Vec3f{0.0f, 0.0f, 1.0f}
        );

        CHECK(matrix[0][0] == doctest::Approx{0.0f});
        CHECK(matrix[0][1] == doctest::Approx{1.0f});
        CHECK(matrix[1][0] == doctest::Approx{-1.0f});
        CHECK(matrix[1][1] == doctest::Approx{0.0f});
        CHECK(matrix[2][2] == doctest::Approx{1.0f});
        CHECK(matrix[3][3] == doctest::Approx{1.0f});
    }

    TEST_CASE("perspective") {
        const auto matrix = siren::Mat4x4f::perspective(
            siren::Radians{std::numbers::pi / 2.0},
            siren::NonZeroPositiveF32{1.0f},
            siren::NonZeroPositiveF32{1.0f},
            siren::NonZeroPositiveF32{10.0f}
        );

        CHECK(matrix[0][0] == doctest::Approx{1.0f});
        CHECK(matrix[1][1] == doctest::Approx{1.0f});
        CHECK(matrix[2][2] == doctest::Approx{-11.0f / 9.0f});
        CHECK_EQ(matrix[2][3], -1.0f);
        CHECK(matrix[3][2] == doctest::Approx{-20.0f / 9.0f});
        CHECK_EQ(matrix[3][3], 0.0f);
    }
}
