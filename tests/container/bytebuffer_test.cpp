#include <array>
#include <concepts>
#include <cstring>
#include <span>
#include <utility>
#include <doctest/doctest.h>

#include "2iREN/container/bytebuffer.hpp"

using namespace siren;

TEST_SUITE("bytebuffer") {
    TEST_CASE("default construction produces an empty buffer") {
        const auto buffer = ByteBuffer{};

        CHECK(buffer.empty());
        CHECK_EQ(buffer.size_bytes(), 0);
        CHECK_EQ(buffer.size_as<u8>(), 0);
    }

    TEST_CASE("making from a list copies every item") {
        const auto buffer   = ByteBuffer::make<u32>({1, 2, 3});
        const auto expected = std::array{
            u32{1},
            u32{2},
            u32{3},
        };
        const auto byte_size = expected.size() * sizeof(u32);

        CHECK_FALSE(buffer.empty());
        CHECK_EQ(buffer.size_bytes(), byte_size);
        CHECK_EQ(buffer.size_as<u32>(), expected.size());
        CHECK_EQ(std::memcmp(buffer.data(), expected.data(), byte_size), 0);
    }

    TEST_CASE("list construction copies every item") {
        const auto buffer = ByteBuffer{
            u16{4},
            u16{8},
            u16{15},
        };
        const auto expected = std::array{
            u16{4},
            u16{8},
            u16{15},
        };
        const auto byte_size = expected.size() * sizeof(u16);

        CHECK_EQ(buffer.size_bytes(), byte_size);
        CHECK_EQ(buffer.size_as<u16>(), expected.size());
        CHECK_EQ(std::memcmp(buffer.data(), expected.data(), byte_size), 0);
    }

    TEST_CASE("span construction copies every item") {
        const auto values = std::array{
            u16{16},
            u16{23},
            u16{42},
        };
        const auto items  = std::span<const u16>{values};
        const auto buffer = ByteBuffer{items};

        CHECK_EQ(buffer.size_bytes(), items.size_bytes());
        CHECK_EQ(buffer.size_as<u16>(), items.size());
        CHECK_EQ(std::memcmp(buffer.data(), items.data(), items.size_bytes()), 0);
    }

    TEST_CASE("writing values appends their bytes") {
        auto buffer = ByteBuffer{};

        const auto first  = u16{4};
        const auto second = u16{8};
        buffer.write(first);
        buffer.write(second);

        const auto expected = std::array{
            u16{4},
            u16{8},
        };
        const auto byte_size = expected.size() * sizeof(u16);

        CHECK_EQ(buffer.size_bytes(), byte_size);
        CHECK_EQ(std::memcmp(buffer.data(), expected.data(), byte_size), 0);
    }

    TEST_CASE("writing a list appends every item") {
        auto buffer = ByteBuffer{};

        buffer.write({
            u32{1},
            u32{2},
            u32{3},
        });

        const auto expected = std::array{
            u32{1},
            u32{2},
            u32{3},
        };
        const auto byte_size = expected.size() * sizeof(u32);

        CHECK_EQ(buffer.size_bytes(), byte_size);
        CHECK_EQ(std::memcmp(buffer.data(), expected.data(), byte_size), 0);
    }

    TEST_CASE("writing a span appends every item") {
        auto buffer = ByteBuffer{};

        const auto values = std::array{
            u32{5},
            u32{10},
            u32{15},
        };
        const auto items = std::span<const u32>{values};
        buffer.write(items);

        CHECK_EQ(buffer.size_bytes(), items.size_bytes());
        CHECK_EQ(buffer.size_as<u32>(), items.size());
        CHECK_EQ(std::memcmp(buffer.data(), items.data(), items.size_bytes()), 0);
    }

    TEST_CASE("resizing changes the byte count") {
        auto buffer = ByteBuffer{};

        buffer.resize_bytes(7);

        CHECK_FALSE(buffer.empty());
        CHECK_EQ(buffer.size_bytes(), 7);
        CHECK_EQ(buffer.size_as<u8>(), 7);

        buffer.resize_as<u32>(3);

        CHECK_EQ(buffer.size_bytes(), sizeof(u32) * 3);
        CHECK_EQ(buffer.size_as<u32>(), 3);
    }

    TEST_CASE("reserving increases capacity without changing size") {
        auto buffer = ByteBuffer{};

        buffer.reserve_bytes(128);

        CHECK_GE(buffer.capacity_bytes(), 128);
        CHECK_EQ(buffer.capacity_as<u32>(), buffer.capacity_bytes() / sizeof(u32));

        buffer.reserve_as<u32>(64);

        CHECK_GE(buffer.capacity_bytes(), sizeof(u32) * 64);
    }

    TEST_CASE("clearing removes every byte") {
        auto buffer = ByteBuffer::make<u32>({1, 2, 3});

        buffer.clear();

        CHECK(buffer.empty());
        CHECK_EQ(buffer.size_bytes(), 0);
        CHECK_EQ(buffer.size_as<u32>(), 0);
    }

    TEST_CASE("view exposes the stored bytes") {
        const auto buffer = ByteBuffer::make<u16>({4, 8, 15});
        const auto view   = buffer.view();

        static_assert(std::same_as<decltype(view), const ByteBufferView>);

        CHECK_EQ(view.size(), buffer.size_bytes());
        CHECK_EQ(view.data(), buffer.data());
        CHECK_EQ(std::memcmp(view.data(), buffer.data(), view.size()), 0);
    }

    TEST_CASE("data allows byte access") {
        auto buffer = ByteBuffer::make<u8>({1, 2, 3});

        static_assert(std::same_as<decltype(buffer.data()), byte*>);
        static_assert(std::same_as<decltype(std::as_const(buffer).data()), const byte*>);

        buffer.data()[1] = std::byte{42};

        CHECK_EQ(buffer.data()[0], std::byte{1});
        CHECK_EQ(buffer.data()[1], std::byte{42});
        CHECK_EQ(buffer.data()[2], std::byte{3});
    }

    TEST_CASE("typed access exposes stored values") {
        auto buffer = ByteBuffer::make<u32>({4, 8, 15});

        static_assert(std::same_as<decltype(buffer.as<u32>()), u32*>);
        static_assert(
            std::same_as<decltype(std::as_const(buffer).as<u32>()), const u32*>
        );

        const auto expected = std::array{
            u32{4},
            u32{8},
            u32{15},
        };
        const auto byte_size = expected.size() * sizeof(u32);

        CHECK_EQ(std::memcmp(buffer.as<u32>(), expected.data(), byte_size), 0);
    }

    TEST_CASE("copying produces independent storage") {
        const auto original = ByteBuffer::make<u8>({1, 2, 3});
        auto       copy     = original;
        auto       assigned = ByteBuffer{};
        assigned            = original;

        copy.data()[0]     = std::byte{42};
        assigned.data()[1] = std::byte{43};

        CHECK_EQ(original.data()[0], std::byte{1});
        CHECK_EQ(original.data()[1], std::byte{2});
        CHECK_EQ(copy.data()[0], std::byte{42});
        CHECK_EQ(assigned.data()[1], std::byte{43});
        CHECK_EQ(original.size_bytes(), copy.size_bytes());
        CHECK_EQ(original.size_bytes(), assigned.size_bytes());
    }

    TEST_CASE("moving preserves stored values") {
        auto source = ByteBuffer::make<u32>({4, 8, 15});
        auto moved  = std::move(source);
        auto assigned = ByteBuffer{};
        assigned      = std::move(moved);

        const auto expected = std::array{
            u32{4},
            u32{8},
            u32{15},
        };
        const auto byte_size = expected.size() * sizeof(u32);

        CHECK_EQ(assigned.size_bytes(), byte_size);
        CHECK_EQ(std::memcmp(assigned.data(), expected.data(), byte_size), 0);
    }

    TEST_CASE("making from values appends every value") {
        const auto buffer = ByteBuffer::make(
            u16{4},
            u16{8},
            u16{15}
        );

        CHECK_EQ(buffer.size_as<u16>(), 3);
        CHECK_EQ(buffer.as<u16>()[0], 4);
        CHECK_EQ(buffer.as<u16>()[1], 8);
        CHECK_EQ(buffer.as<u16>()[2], 15);
    }

    TEST_CASE("sized construction reserves requested bytes") {
        const auto buffer = ByteBuffer::with_size_bytes(64);

        CHECK(buffer.empty());
        CHECK_GE(buffer.capacity_bytes(), 64);
    }

    TEST_CASE("aligned writes append zero padding") {
        auto buffer = ByteBuffer{};

        buffer.write(u16{0x1234}, 4);

        CHECK_EQ(buffer.size_bytes(), 4);
        CHECK_EQ(buffer.data()[2], std::byte{0});
        CHECK_EQ(buffer.data()[3], std::byte{0});
    }

    TEST_CASE("subview exposes the selected bytes") {
        const auto buffer = ByteBuffer::make<u8>({1, 2, 3, 4});
        const auto view   = buffer.subview(Range<usize>::make(1, 3));

        CHECK_EQ(view.size(), 2);
        CHECK_EQ(view[0], std::byte{2});
        CHECK_EQ(view[1], std::byte{3});
    }

    TEST_CASE("copying a view fills the destination") {
        const auto buffer = ByteBuffer::make<u16>({4, 8, 15});
        auto destination  = std::array<u16, 3>{};

        bufcpy(buffer.view(), destination.data());

        CHECK_EQ(destination, std::array{u16{4}, u16{8}, u16{15}});
    }
}
