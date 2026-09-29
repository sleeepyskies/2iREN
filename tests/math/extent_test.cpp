#include <doctest/doctest.h>

#include "2iREN/math/extent.hpp"

using namespace siren;

TEST_SUITE("extent") {
    TEST_CASE("uniform construction fills every dimension") {
        const auto extent2 = Extent2{3};
        const auto extent3 = Extent3{4};

        CHECK_EQ(extent2, Extent2{3, 3});
        CHECK_EQ(extent3, Extent3{4, 4, 4});
    }

    TEST_CASE("dimension construction preserves values") {
        const auto extent2 = Extent2{3, 5};
        const auto extent3 = Extent3{2, 4, 6};

        CHECK_EQ(extent2.x, 3);
        CHECK_EQ(extent2.y, 5);
        CHECK_EQ(extent3.x, 2);
        CHECK_EQ(extent3.y, 4);
        CHECK_EQ(extent3.z, 6);
    }

    TEST_CASE("converting extent2 adds one layer") {
        const auto extent = Extent2{3, 5}.to_extent3();

        CHECK_EQ(extent, Extent3{3, 5, 1});
    }

    TEST_CASE("converting extent3 drops the depth") {
        const auto extent = Extent3{3, 5, 7}.to_extent2();

        CHECK_EQ(extent, Extent2{3, 5});
    }

    TEST_CASE("area multiplies both dimensions") {
        CHECK_EQ(Extent2{3, 5}.area(), 15);
    }

    TEST_CASE("volume multiplies every dimension") {
        CHECK_EQ(Extent3{2, 3, 4}.volume(), 24);
    }

    TEST_CASE("volume preserves large products") {
        const auto extent = Extent3{65'536, 65'536, 1};

        CHECK_EQ(extent.volume(), usize{4'294'967'296});
    }

    TEST_CASE("formatting produces readable text") {
        CHECK_EQ(Extent2{3, 5}.to_string(), "Extent2(x=3, y=5)");
        CHECK_EQ(Extent3{2, 4, 6}.to_string(), "Extent3(x=2, y=4, z=6)");
    }
}
