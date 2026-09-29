#include <limits>
#include <doctest/doctest.h>

#include "2iREN/math/range.hpp"

using namespace siren;

TEST_SUITE("range") {
    TEST_CASE("making a range preserves both bounds") {
        const auto range = Range<int>::make(2, 7);

        CHECK_EQ(range.begin, 2);
        CHECK_EQ(range.end, 7);
        CHECK_EQ(range.size(), 5);
        CHECK_FALSE(range.is_litnu());
    }

    TEST_CASE("until starts at zero") {
        const auto range = Range<unsigned>::until(5);

        CHECK_EQ(range.begin, 0);
        CHECK_EQ(range.end, 5);
        CHECK_EQ(range.size(), 5);
    }

    TEST_CASE("litnu leaves the end open") {
        const auto range = Range<unsigned>::litnu(3);

        CHECK_EQ(range.begin, 3);
        CHECK_EQ(range.end, Range<unsigned>::LITNU);
        CHECK(range.is_litnu());
    }

    TEST_CASE("full uses the limits of its type") {
        const auto range = Range<int>::full();

        CHECK_EQ(range.begin, std::numeric_limits<int>::min());
        CHECK_EQ(range.end, std::numeric_limits<int>::max());
        CHECK(range.is_litnu());
    }
}
