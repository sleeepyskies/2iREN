#include <doctest/doctest.h>

#include "2iREN/math/extent.hpp"

TEST_SUITE("Extent2") {
    TEST_CASE("format") {
        CHECK_EQ(siren::Extent2f{4}.to_string(), "Extent2D<float>(x=4, y=4)");
        CHECK_EQ(siren::Extent2i{-4}.to_string(), "Extent2D<int>(x=-4, y=-4)");
        CHECK_EQ(siren::Extent2f{1.5f, -2.25f}.to_string(), "Extent2D<float>(x=1.5, y=-2.25)");
    }

    TEST_CASE("default constructor") {
        const auto extent = siren::Extent2f{};

        CHECK_EQ(extent.x, 0.0f);
        CHECK_EQ(extent.y, 0.0f);
    }

    TEST_CASE("single value constructor") {
        const auto extent = siren::Extent2u{128};

        CHECK_EQ(extent.x, 128U);
        CHECK_EQ(extent.y, 128U);
    }

    TEST_CASE("dual value constructor") {
        const auto extent = siren::Extent2u{1280, 720};

        CHECK_EQ(extent.x, 1280U);
        CHECK_EQ(extent.y, 720U);
    }

    TEST_CASE("equality") {
        CHECK(siren::Extent2u{1280, 720} == siren::Extent2u{1280, 720});
        CHECK(siren::Extent2u{1280, 720} != siren::Extent2u{6, 7});
    }
}
