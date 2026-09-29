#include <concepts>
#include <doctest/doctest.h>

#include "2iREN/math/point2.hpp"

using namespace siren;

TEST_SUITE("point2") {
    TEST_CASE("default construction produces the origin") {
        const auto point = Point2f{};

        CHECK_EQ(point, Point2f::ORIGIN());
    }

    TEST_CASE("uniform construction fills both coordinates") {
        const auto point = Point2i{3};

        CHECK_EQ(point.x, 3);
        CHECK_EQ(point.y, 3);
    }

    TEST_CASE("coordinate construction preserves values") {
        const auto point = Point2f{1.5f, -2.0f};

        CHECK_EQ(point.x, 1.5f);
        CHECK_EQ(point.y, -2.0f);
    }

    TEST_CASE("aliases preserve their scalar types") {
        static_assert(std::same_as<Point2f::Type, f32>);
        static_assert(std::same_as<Point2i::Type, i32>);
        static_assert(std::same_as<Point2u::Type, u32>);
    }

    TEST_CASE("formatting produces readable text") {
        const auto point = Point2i{-4, 2};

        CHECK_EQ(point.to_string(), "Point2<int>(x=-4, y=2)");
    }
}
