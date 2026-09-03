#include <doctest/doctest.h>

#include "2iREN/math/color.hpp"

TEST_SUITE("Rgba") {
    TEST_CASE("default value constructor test") {
        const auto color = siren::Rgba{};
        CHECK_EQ(color.r, 0.0f);
        CHECK_EQ(color.g, 0.0f);
        CHECK_EQ(color.b, 0.0f);
        CHECK_EQ(color.a, 1.0f);
    }

    TEST_CASE("single value constructor test") {
        const auto color = siren::Rgba{2.f};
        CHECK_EQ(color.r, 2.f);
        CHECK_EQ(color.g, 2.f);
        CHECK_EQ(color.b, 2.f);
        CHECK_EQ(color.a, 1.f);
    }

    TEST_CASE("rgb alpha constructor test") {
        const auto color = siren::Rgba{2.f, 5.f};
        CHECK_EQ(color.r, 2.f);
        CHECK_EQ(color.g, 2.f);
        CHECK_EQ(color.b, 2.f);
        CHECK_EQ(color.a, 5.f);
    }

    TEST_CASE("rgba constructor test") {
        const auto color = siren::Rgba{2.f, 5.f, 4.f, 3.f};
        CHECK_EQ(color.r, 2.f);
        CHECK_EQ(color.g, 5.f);
        CHECK_EQ(color.b, 4.f);
        CHECK_EQ(color.a, 3.f);
    }

    TEST_CASE("equality test") {
        const auto left  = siren::Rgba{};
        const auto right = siren::Rgba{};
        const auto other = siren::Rgba{1.f};
        CHECK(left == right);
        CHECK(left != other);
        CHECK(other != right);
    }

    TEST_CASE("constants test") {
        constexpr auto zero  = siren::Rgba::ZERO();
        constexpr auto one   = siren::Rgba::ONE();
        constexpr auto black = siren::Rgba::BLACK();
        constexpr auto gray  = siren::Rgba::GRAY();
        constexpr auto white = siren::Rgba::WHITE();
        constexpr auto red   = siren::Rgba::RED();
        constexpr auto green = siren::Rgba::GREEN();
        constexpr auto blue  = siren::Rgba::BLUE();

        CHECK_EQ(zero.a, 0.f);
        CHECK_EQ(one.r, 1.f);
        CHECK_EQ(one.g, 1.f);
        CHECK_EQ(one.b, 1.f);
        CHECK_EQ(one.a, 1.f);

        CHECK_EQ(black.a, 1.f);
        CHECK_EQ(gray.r, 0.5f);
        CHECK_EQ(white.r, 1.f);
        CHECK_EQ(red, siren::Rgba{1.f, 0.f, 0.f, 1.f});
        CHECK_EQ(green, siren::Rgba{0.f, 1.f, 0.f, 1.f});
        CHECK_EQ(blue, siren::Rgba{0.f, 0.f, 1.f, 1.f});
    }

    TEST_CASE("format test") {
        const auto color = siren::Rgba{1.f, 0.5f, 0.25f, 1.f};
        CHECK_EQ(color.to_string(), "Rgba(1, 0.5, 0.25, 1)");
    }
}
