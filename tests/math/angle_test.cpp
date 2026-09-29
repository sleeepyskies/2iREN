#include <numbers>
#include <doctest/doctest.h>

#include "2iREN/math/angle.hpp"

using namespace siren;

TEST_SUITE("angle") {
    TEST_CASE("radians preserve their value") {
        const auto angle = Radians{1.5};

        CHECK_EQ(angle.value, 1.5);
        CHECK_EQ(angle.to_string(), "Radians(1.5)");
    }

    TEST_CASE("degrees preserve their value") {
        const auto angle = Degrees{90.0};

        CHECK_EQ(angle.value, 90.0);
        CHECK_EQ(angle.to_string(), "Degrees(90)");
    }

    TEST_CASE("radians convert to degrees") {
        const auto degrees = Radians{std::numbers::pi}.to_degrees();

        CHECK_EQ(degrees.value, doctest::Approx(180.0));
    }

    TEST_CASE("degrees convert to radians") {
        const auto radians = Degrees{180.0}.to_radians();

        CHECK_EQ(radians.value, doctest::Approx(std::numbers::pi));
    }

    TEST_CASE("angles compare by value") {
        CHECK(Radians{1.0} < Radians{2.0});
        CHECK(Radians{1.0} == Radians{1.0});
        CHECK(Degrees{45.0} < Degrees{90.0});
        CHECK(Degrees{45.0} == Degrees{45.0});
    }
}
