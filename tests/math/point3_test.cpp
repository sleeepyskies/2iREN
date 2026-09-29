#include <concepts>
#include <doctest/doctest.h>

#include "2iREN/math/point3.hpp"

using namespace siren;

TEST_SUITE("point3") {
    TEST_CASE("default construction produces the origin") {
        const auto point = Point3f{};

        CHECK_EQ(point, Point3f::ORIGIN());
    }

    TEST_CASE("uniform construction fills every coordinate") {
        const auto point = Point3f{2.5f};

        CHECK_EQ(point.x, 2.5f);
        CHECK_EQ(point.y, 2.5f);
        CHECK_EQ(point.z, 2.5f);
    }

    TEST_CASE("coordinate construction preserves values") {
        const auto point = Point3f{1.0f, 2.0f, 3.0f};

        CHECK_EQ(point.x, 1.0f);
        CHECK_EQ(point.y, 2.0f);
        CHECK_EQ(point.z, 3.0f);
    }

    TEST_CASE("translating produces a new point") {
        const auto point       = Point3f{1.0f, 2.0f, 3.0f};
        const auto translation = Vec3f{4.0f, -2.0f, 0.5f};

        const auto translated = Point3f::translate(point, translation);

        CHECK_EQ(translated, Point3f{5.0f, 0.0f, 3.5f});
        CHECK_EQ(point, Point3f{1.0f, 2.0f, 3.0f});
    }

    TEST_CASE("subtracting points produces a displacement") {
        const auto start = Point3i{1, 2, 3};
        const auto end   = Point3i{5, 1, 8};

        CHECK_EQ(end - start, Vec3i{4, -1, 5});
    }

    TEST_CASE("aliases preserve their scalar types") {
        static_assert(std::same_as<Point3f::Type, f32>);
        static_assert(std::same_as<Point3i::Type, i32>);
        static_assert(std::same_as<Point3u::Type, u32>);
    }
}
