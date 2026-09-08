#include <array>
#include <concepts>
#include <cstring>
#include <span>
#include <utility>

#include <doctest/doctest.h>

#include "2iREN/utility/byte_buffer.hpp"

TEST_SUITE("ByteBuffer") {
    TEST_CASE("default") {
        const auto buffer = siren::ByteBuffer{};

        CHECK(buffer.empty());
        CHECK_EQ(buffer.size_bytes(), 0);
        CHECK_EQ(buffer.size_as<siren::u8>(), 0);
    }

    TEST_CASE("make") {
        const auto buffer   = siren::ByteBuffer::make<siren::u32>({1, 2, 3});
        const auto expected = std::array{
            siren::u32{1},
            siren::u32{2},
            siren::u32{3},
        };
        const auto byte_size = expected.size() * sizeof(siren::u32);

        CHECK_FALSE(buffer.empty());
        CHECK_EQ(buffer.size_bytes(), byte_size);
        CHECK_EQ(buffer.size_as<siren::u32>(), expected.size());
        CHECK_EQ(std::memcmp(buffer.data(), expected.data(), byte_size), 0);
    }

    TEST_CASE("initializer") {
        const auto buffer = siren::ByteBuffer{
            siren::u16{4},
            siren::u16{8},
            siren::u16{15},
        };
        const auto expected = std::array{
            siren::u16{4},
            siren::u16{8},
            siren::u16{15},
        };
        const auto byte_size = expected.size() * sizeof(siren::u16);

        CHECK_EQ(buffer.size_bytes(), byte_size);
        CHECK_EQ(buffer.size_as<siren::u16>(), expected.size());
        CHECK_EQ(std::memcmp(buffer.data(), expected.data(), byte_size), 0);
    }

    TEST_CASE("span") {
        const auto values = std::array{
            siren::u16{16},
            siren::u16{23},
            siren::u16{42},
        };
        const auto items  = std::span<const siren::u16>{values};
        const auto buffer = siren::ByteBuffer{items};

        CHECK_EQ(buffer.size_bytes(), items.size_bytes());
        CHECK_EQ(buffer.size_as<siren::u16>(), items.size());
        CHECK_EQ(std::memcmp(buffer.data(), items.data(), items.size_bytes()), 0);
    }

    TEST_CASE("write") {
        auto buffer = siren::ByteBuffer{};

        const auto first  = siren::u16{4};
        const auto second = siren::u16{8};
        buffer.write(first);
        buffer.write(second);

        const auto expected = std::array{
            siren::u16{4},
            siren::u16{8},
        };
        const auto byte_size = expected.size() * sizeof(siren::u16);

        CHECK_EQ(buffer.size_bytes(), byte_size);
        CHECK_EQ(std::memcmp(buffer.data(), expected.data(), byte_size), 0);
    }

    TEST_CASE("write list") {
        auto buffer = siren::ByteBuffer{};

        buffer.write({
            siren::u32{1},
            siren::u32{2},
            siren::u32{3},
        });

        const auto expected = std::array{
            siren::u32{1},
            siren::u32{2},
            siren::u32{3},
        };
        const auto byte_size = expected.size() * sizeof(siren::u32);

        CHECK_EQ(buffer.size_bytes(), byte_size);
        CHECK_EQ(std::memcmp(buffer.data(), expected.data(), byte_size), 0);
    }

    TEST_CASE("write span") {
        auto buffer = siren::ByteBuffer{};

        const auto values = std::array{
            siren::u32{5},
            siren::u32{10},
            siren::u32{15},
        };
        const auto items = std::span<const siren::u32>{values};
        buffer.write(items);

        CHECK_EQ(buffer.size_bytes(), items.size_bytes());
        CHECK_EQ(buffer.size_as<siren::u32>(), items.size());
        CHECK_EQ(std::memcmp(buffer.data(), items.data(), items.size_bytes()), 0);
    }

    TEST_CASE("resize") {
        auto buffer = siren::ByteBuffer{};

        buffer.resize_bytes(7);

        CHECK_FALSE(buffer.empty());
        CHECK_EQ(buffer.size_bytes(), 7);
        CHECK_EQ(buffer.size_as<siren::u8>(), 7);

        buffer.resize_as<siren::u32>(3);

        CHECK_EQ(buffer.size_bytes(), sizeof(siren::u32) * 3);
        CHECK_EQ(buffer.size_as<siren::u32>(), 3);
    }

    TEST_CASE("reserve") {
        auto buffer = siren::ByteBuffer{};

        buffer.reserve_bytes(128);

        CHECK_GE(buffer.capacity_bytes(), 128);
        CHECK_EQ(buffer.capacity_as<siren::u32>(), buffer.capacity_bytes() / sizeof(siren::u32));

        buffer.reserve_as<siren::u32>(64);

        CHECK_GE(buffer.capacity_bytes(), sizeof(siren::u32) * 64);
    }

    TEST_CASE("clear") {
        auto buffer = siren::ByteBuffer::make<siren::u32>({1, 2, 3});

        buffer.clear();

        CHECK(buffer.empty());
        CHECK_EQ(buffer.size_bytes(), 0);
        CHECK_EQ(buffer.size_as<siren::u32>(), 0);
    }

    TEST_CASE("view") {
        const auto buffer = siren::ByteBuffer::make<siren::u16>({4, 8, 15});
        const auto view   = buffer.view();

        static_assert(std::same_as<decltype(view), const siren::ByteBufferView>);

        CHECK_EQ(view.size(), buffer.size_bytes());
        CHECK_EQ(view.data(), buffer.data());
        CHECK_EQ(std::memcmp(view.data(), buffer.data(), view.size()), 0);
    }

    TEST_CASE("data") {
        auto buffer = siren::ByteBuffer::make<siren::u8>({1, 2, 3});

        static_assert(std::same_as<decltype(buffer.data()), siren::u8*>);
        static_assert(std::same_as<decltype(std::as_const(buffer).data()), const siren::u8*>);

        buffer.data()[1] = 42;

        CHECK_EQ(buffer.data()[0], 1);
        CHECK_EQ(buffer.data()[1], 42);
        CHECK_EQ(buffer.data()[2], 3);
    }

    TEST_CASE("as") {
        auto buffer = siren::ByteBuffer::make<siren::u32>({4, 8, 15});

        static_assert(std::same_as<decltype(buffer.as<siren::u32>()), siren::u32*>);
        static_assert(
            std::same_as<decltype(std::as_const(buffer).as<siren::u32>()), const siren::u32*>
        );

        const auto expected = std::array{
            siren::u32{4},
            siren::u32{8},
            siren::u32{15},
        };
        const auto byte_size = expected.size() * sizeof(siren::u32);

        CHECK_EQ(std::memcmp(buffer.as<siren::u32>(), expected.data(), byte_size), 0);
    }

    TEST_CASE("copy") {
        const auto original = siren::ByteBuffer::make<siren::u8>({1, 2, 3});
        auto copy           = original;

        copy.data()[0] = 42;

        CHECK_EQ(original.data()[0], 1);
        CHECK_EQ(copy.data()[0], 42);
        CHECK_EQ(original.size_bytes(), copy.size_bytes());
    }

    TEST_CASE("move") {
        auto source = siren::ByteBuffer::make<siren::u32>({4, 8, 15});
        auto moved  = std::move(source);

        const auto expected = std::array{
            siren::u32{4},
            siren::u32{8},
            siren::u32{15},
        };
        const auto byte_size = expected.size() * sizeof(siren::u32);

        CHECK_EQ(moved.size_bytes(), byte_size);
        CHECK_EQ(std::memcmp(moved.data(), expected.data(), byte_size), 0);
    }
}
