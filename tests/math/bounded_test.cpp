#include <doctest/doctest.h>

#include "2iREN/math/bounded.hpp"

using namespace siren;

TEST_SUITE("Bounded") {
    TEST_CASE("constructor") {
        const auto b = BoundedI32{3};
    }
}
