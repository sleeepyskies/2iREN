#include <doctest/doctest.h>

#include <numbers>

#include "2iREN/math/angle.hpp"

TEST_SUITE("Radians") {
    TEST_CASE("constructor") {
        const auto rad = siren::Radians{1.5};

        CHECK_EQ(rad.value, 1.5);
    }

    TEST_CASE("converts to degrees") {
        const auto rad = siren::Radians{std::numbers::pi};

        const auto deg = rad.to_degrees();

        CHECK_EQ(deg.value, doctest::Approx(180.0));
    }

    TEST_CASE("formats as string") {
        const auto rad = siren::Radians{1.5};

        CHECK_EQ(rad.to_string(), "Radians(1.5)");
    }

    TEST_CASE("compares values") {
        const auto first  = siren::Radians{1.0};
        const auto second = siren::Radians{2.0};
        const auto copy   = siren::Radians{1.0};

        CHECK(first < second);
        CHECK(first == copy);
        CHECK(first != second);
    }
}

TEST_SUITE("Degrees") {
    TEST_CASE("constructor") {
        const auto deg = siren::Degrees{90.0};

        CHECK_EQ(deg.value, 90.0);
    }

    TEST_CASE("converts to radians") {
        const auto deg = siren::Degrees{180.0};

        const auto rad = deg.to_radians();

        CHECK_EQ(rad.value, doctest::Approx(std::numbers::pi));
    }

    TEST_CASE("formats as string") {
        const auto deg = siren::Degrees{90.0};

        CHECK_EQ(deg.to_string(), "Degrees(90)");
    }

    TEST_CASE("compares values") {
        const auto first  = siren::Degrees{45.0};
        const auto second = siren::Degrees{90.0};
        const auto copy   = siren::Degrees{45.0};

        CHECK(first < second);
        CHECK(first == copy);
        CHECK(first != second);
    }
}
