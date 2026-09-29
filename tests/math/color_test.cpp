#include <concepts>
#include <utility>
#include <doctest/doctest.h>

#include "2iREN/math/color.hpp"

using namespace siren;

TEST_SUITE("rgba") {
    TEST_CASE("default construction produces black") {
        const auto color = Rgba{};

        CHECK_EQ(color, Rgba{0.0f, 0.0f, 0.0f, 1.0f});
    }

    TEST_CASE("uniform construction fills rgb channels") {
        const auto color = Rgba{0.25f};

        CHECK_EQ(color, Rgba{0.25f, 0.25f, 0.25f, 1.0f});
    }

    TEST_CASE("rgb construction preserves alpha") {
        const auto color = Rgba{0.25f, 0.5f};

        CHECK_EQ(color, Rgba{0.25f, 0.25f, 0.25f, 0.5f});
    }

    TEST_CASE("channel construction preserves every value") {
        const auto color = Rgba{0.1f, 0.2f, 0.3f, 0.4f};

        CHECK_EQ(color.r, 0.1f);
        CHECK_EQ(color.g, 0.2f);
        CHECK_EQ(color.b, 0.3f);
        CHECK_EQ(color.a, 0.4f);
    }

    TEST_CASE("lerping produces the midpoint color") {
        const auto color = Rgba::lerp(Rgba::BLACK(), Rgba::WHITE());

        CHECK_EQ(color, Rgba{0.5f, 0.5f, 0.5f, 1.0f});
    }

    TEST_CASE("lerping clamps the interpolation amount") {
        const auto color = Rgba::lerp(
            Rgba::BLACK(),
            Rgba::WHITE(),
            Rgba::LerpT{2.0f}
        );

        CHECK_EQ(color, Rgba::WHITE());
    }

    TEST_CASE("color constants produce expected channels") {
        CHECK_EQ(Rgba::ZERO(), Rgba{0.0f, 0.0f, 0.0f, 0.0f});
        CHECK_EQ(Rgba::ONE(), Rgba{1.0f, 1.0f, 1.0f, 1.0f});
        CHECK_EQ(Rgba::BLACK(), Rgba{0.0f, 0.0f, 0.0f, 1.0f});
        CHECK_EQ(Rgba::GRAY(), Rgba{0.5f, 0.5f, 0.5f, 1.0f});
        CHECK_EQ(Rgba::WHITE(), Rgba{1.0f, 1.0f, 1.0f, 1.0f});
        CHECK_EQ(Rgba::RED(), Rgba{1.0f, 0.0f, 0.0f, 1.0f});
        CHECK_EQ(Rgba::GREEN(), Rgba{0.0f, 1.0f, 0.0f, 1.0f});
        CHECK_EQ(Rgba::BLUE(), Rgba{0.0f, 0.0f, 1.0f, 1.0f});
        CHECK_EQ(Rgba::YELLOW(), Rgba{1.0f, 1.0f, 0.0f, 1.0f});
        CHECK_EQ(Rgba::PURPLE(), Rgba{1.0f, 0.0f, 1.0f, 1.0f});
        CHECK_EQ(Rgba::CYAN(), Rgba{0.0f, 1.0f, 1.0f, 1.0f});
    }

    TEST_CASE("data returns a pointer to the channels") {
        constexpr auto returns_pointer = std::same_as<decltype(std::declval<Rgba&>().data()), f32*>;

        CHECK(returns_pointer);
    }

    TEST_CASE("formatting produces readable text") {
        const auto color = Rgba{1.0f, 0.5f, 0.25f, 1.0f};

        CHECK_EQ(color.to_string(), "Rgba(1, 0.5, 0.25, 1)");
    }
}
