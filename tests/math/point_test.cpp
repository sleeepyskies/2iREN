#include <doctest/doctest.h>

#include "2iREN/math/point.hpp"

TEST_SUITE("Point3") {
    TEST_CASE("default construction creates the origin") {
        const auto point = siren::Point3f{};

        CHECK_EQ(point, siren::Point3f::ORIGIN());
    }

    TEST_CASE("uniform construction assigns every coordinate") {
        const auto point = siren::Point3f{2.5f};

        CHECK_EQ(point.x, 2.5f);
        CHECK_EQ(point.y, 2.5f);
        CHECK_EQ(point.z, 2.5f);
    }

    TEST_CASE("coordinate construction preserves each coordinate") {
        const auto point = siren::Point3f{1.0f, 2.0f, 3.0f};

        CHECK_EQ(point.x, 1.0f);
        CHECK_EQ(point.y, 2.0f);
        CHECK_EQ(point.z, 3.0f);
    }

    TEST_CASE("translation produces a new point") {
        const auto point       = siren::Point3f{1.0f, 2.0f, 3.0f};
        const auto translation = siren::Vec3f{4.0f, -2.0f, 0.5f};

        const auto translated = siren::Point3f::translate(point, translation);

        CHECK_EQ(translated, siren::Point3f{5.0f, 0.0f, 3.5f});
        CHECK_EQ(point, siren::Point3f{1.0f, 2.0f, 3.0f});
    }

    TEST_CASE("subtracting points produces a displacement vector") {
        const auto start = siren::Point3f{1.0f, 2.0f, 3.0f};
        const auto end   = siren::Point3f{5.0f, 1.0f, 8.0f};

        const auto displacement = end - start;

        CHECK_EQ(displacement, siren::Vec3f{4.0f, -1.0f, 5.0f});
    }

    TEST_CASE("integer points produce integer displacement vectors") {
        const auto start = siren::Point3i{1, 2, 3};
        const auto end   = siren::Point3i{5, 1, 8};

        const auto displacement = end - start;

        CHECK_EQ(displacement, siren::Vec3i{4, -1, 5});
    }

    TEST_CASE("unsigned points preserve their scalar type") {
        const auto start = siren::Point3u{1u, 2u, 3u};
        const auto end   = siren::Point3u{5u, 4u, 8u};

        const auto displacement = end - start;

        CHECK_EQ(displacement, siren::Vec3u{4u, 2u, 5u});
    }
}
