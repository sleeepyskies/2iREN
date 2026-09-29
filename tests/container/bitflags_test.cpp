#include <doctest/doctest.h>

#include "2iREN/container/bitflags.hpp"

using namespace siren;

namespace {
enum class TestFlag { first, second, third, fourth, Max };
using TestFlags = BitFlags<TestFlag>;
} // namespace

TEST_SUITE("bitflags") {
    TEST_CASE("default construction produces no flags") {
        const auto flags = TestFlags{};

        CHECK(flags.none());
        CHECK_FALSE(flags.any());
        CHECK_FALSE(flags.all());
    }

    TEST_CASE("single flag construction sets one flag") {
        const auto flags = TestFlags{TestFlag::second};

        CHECK(flags.test(TestFlag::second));
        CHECK(flags.any());
        CHECK_FALSE(flags.test(TestFlag::first));
    }

    TEST_CASE("making flags sets every provided flag") {
        const auto flags = TestFlags::make(TestFlag::first, TestFlag::third);

        CHECK(flags.all(TestFlag::first, TestFlag::third));
        CHECK(flags.any(TestFlag::second, TestFlag::third));
        CHECK(flags.none(TestFlag::second, TestFlag::fourth));
    }

    TEST_CASE("setting and unsetting changes flags") {
        auto flags = TestFlags{};

        flags.set(TestFlag::first);
        flags |= TestFlag::second;
        CHECK(flags.all(TestFlag::first, TestFlag::second));

        flags.unset(TestFlag::first);
        CHECK(flags.none(TestFlag::first));
        CHECK(flags.test(TestFlag::second));

        flags.reset();
        CHECK(flags.none());
    }

    TEST_CASE("combining flags produces a new set") {
        const auto flags = TestFlag::first | TestFlag::second | TestFlag::fourth;

        CHECK(flags.all(TestFlag::first, TestFlag::second, TestFlag::fourth));
        CHECK_FALSE(flags.test(TestFlag::third));
    }

    TEST_CASE("setting every flag makes all true") {
        const auto flags = TestFlags::make(
            TestFlag::first,
            TestFlag::second,
            TestFlag::third,
            TestFlag::fourth
        );

        CHECK(flags.all());
        CHECK_EQ(flags.to_string().substr(flags.to_string().find_last_of('(')), "(1111)");
    }
}
