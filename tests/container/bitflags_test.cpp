#include <doctest/doctest.h>

#include "2iREN/container/bitflags.hpp"

using namespace siren;

enum class Flags { one, two, three, four, five, Max };

using TestFlags = BitFlags<Flags>;

TEST_SUITE("BitFlags") {
    TEST_CASE("constructor") {
        // TODO: tests pls
    }
}
