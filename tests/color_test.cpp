#include <doctest/doctest.h>

#include "2iREN/math/color.hpp"

TEST_CASE("Rgba Default Value Constructor Test") {
    const auto color = siren::Rgba{};
    CHECK_EQ(color.r, 0.0f);
    CHECK_EQ(color.g, 0.0f);
    CHECK_EQ(color.b, 0.0f);
    CHECK_EQ(color.a, 1.0f);
}

TEST_CASE("Rgba Single Value Constructor Test") {
    const auto color = siren::Rgba{2.f};
    CHECK_EQ(color.r, 2.f);
    CHECK_EQ(color.g, 2.f);
    CHECK_EQ(color.b, 2.f);
    CHECK_EQ(color.a, 2.f);
}

TEST_CASE("Rgba Value Alpha Constructor Test") {
    const auto color = siren::Rgba{2.f, 5.f};
    CHECK_EQ(color.r, 2.f);
    CHECK_EQ(color.g, 2.f);
    CHECK_EQ(color.b, 2.f);
    CHECK_EQ(color.a, 5.f);
}

TEST_CASE("Rgba All Args Constructor Test") {
    const auto color = siren::Rgba{2.f, 5.f, 4.f, 3.f};
    CHECK_EQ(color.r, 2.f);
    CHECK_EQ(color.g, 5.f);
    CHECK_EQ(color.b, 4.f);
    CHECK_EQ(color.a, 3.f);
}

TEST_CASE("Rgba Equality Test") {
    const auto left  = siren::Rgba{};
    const auto right = siren::Rgba{};
    const auto other = siren::Rgba{1.f};
    CHECK(left == right);
    CHECK(left != other);
    CHECK(other != right);
}

TEST_CASE("Rgba Constants Test") {
    CHECK_EQ(siren::Rgba::ZERO.a, 0.f);
    CHECK_EQ(siren::Rgba::ONE.r, 1.f);
    CHECK_EQ(siren::Rgba::ONE.g, 1.f);
    CHECK_EQ(siren::Rgba::ONE.b, 1.f);
    CHECK_EQ(siren::Rgba::ONE.a, 1.f);

    CHECK_EQ(siren::Rgba::BLACK.a, 1.f);
    CHECK_EQ(siren::Rgba::GRAY.r, 0.5f);
    CHECK_EQ(siren::Rgba::WHITE.r, 1.f);
    CHECK_EQ(siren::Rgba::RED, siren::Rgba{1.f, 0.f, 0.f, 1.f});
    CHECK_EQ(siren::Rgba::GREEN, siren::Rgba{0.f, 1.f, 0.f, 1.f});
    CHECK_EQ(siren::Rgba::BLUE, siren::Rgba{0.f, 0.f, 1.f, 1.f});
}

TEST_CASE("Rgba Format Test") {
    const auto color = siren::Rgba{1.f, 0.5f, 0.25f, 1.f};
    CHECK_EQ(color.to_string(), "Rgba(1, 0.5, 0.25, 1)");
}
