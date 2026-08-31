#include <cstring>
#include <utility>
#include <vector>

#include <doctest/doctest.h>

#include "2iREN/utility/byte_buffer.hpp"

namespace {

struct Record {
    siren::u32 id;
    siren::f32 weight;

    auto operator==(const Record&) const -> bool = default;
};

template <typename T>
concept AppendableToByteBuffer =
    requires(siren::ByteBuffer& buffer, const T& value) { buffer.append(value); };

template <typename T>
auto read_at(const siren::ByteBuffer& buffer, const siren::usize offset) -> T {
    T value{};
    std::memcpy(&value, buffer.raw() + offset, sizeof(T));
    return value;
}

} // namespace

TEST_SUITE("ByteBuffer") {
    TEST_CASE("default construction creates an empty buffer") {
        const auto buffer = siren::ByteBuffer{};

        CHECK(buffer.empty());
        CHECK_EQ(buffer.size_bytes(), siren::usize{0});
        CHECK_EQ(buffer.size_as<siren::u32>(), siren::usize{0});
    }

    TEST_CASE("construction from a vector preserves every value") {
        const auto values = std::vector<Record>{{1, 1.5f}, {7, -2.f}, {42, 0.25f}};
        const auto buffer = siren::ByteBuffer{values};

        CHECK_FALSE(buffer.empty());
        CHECK_EQ(buffer.size_bytes(), values.size() * sizeof(Record));
        CHECK_EQ(buffer.size_as<Record>(), values.size());

        for (siren::usize index = 0; index < values.size(); ++index) {
            CHECK(read_at<Record>(buffer, index * sizeof(Record)) == values[index]);
        }
    }

    TEST_CASE("construction from an initializer list preserves its order") {
        const auto buffer =
            siren::ByteBuffer{siren::u16{3}, siren::u16{5}, siren::u16{8}, siren::u16{13}};

        CHECK_EQ(buffer.size_as<siren::u16>(), siren::usize{4});
        CHECK_EQ(read_at<siren::u16>(buffer, 0 * sizeof(siren::u16)), siren::u16{3});
        CHECK_EQ(read_at<siren::u16>(buffer, 1 * sizeof(siren::u16)), siren::u16{5});
        CHECK_EQ(read_at<siren::u16>(buffer, 2 * sizeof(siren::u16)), siren::u16{8});
        CHECK_EQ(read_at<siren::u16>(buffer, 3 * sizeof(siren::u16)), siren::u16{13});
    }

    TEST_CASE("append packs values in call order") {
        auto buffer = siren::ByteBuffer{};

        buffer.append(siren::u16{0x1234});
        buffer.append(siren::f32{2.5f});
        buffer.append(siren::u8{0x7f});

        CHECK_EQ(buffer.size_bytes(), sizeof(siren::u16) + sizeof(siren::f32) + sizeof(siren::u8));
        CHECK_EQ(read_at<siren::u16>(buffer, 0), siren::u16{0x1234});
        CHECK_EQ(read_at<siren::f32>(buffer, sizeof(siren::u16)), 2.5f);
        CHECK_EQ(
            read_at<siren::u8>(buffer, sizeof(siren::u16) + sizeof(siren::f32)), siren::u8{0x7f}
        );
    }

    TEST_CASE("append accepts multiple values at once") {
        auto buffer = siren::ByteBuffer{};

        buffer.append<siren::u16>({siren::u16{4}, siren::u16{8}, siren::u16{15}, siren::u16{16}});

        CHECK_EQ(buffer.size_as<siren::u16>(), siren::usize{4});
        CHECK_EQ(read_at<siren::u16>(buffer, 0 * sizeof(siren::u16)), siren::u16{4});
        CHECK_EQ(read_at<siren::u16>(buffer, 1 * sizeof(siren::u16)), siren::u16{8});
        CHECK_EQ(read_at<siren::u16>(buffer, 2 * sizeof(siren::u16)), siren::u16{15});
        CHECK_EQ(read_at<siren::u16>(buffer, 3 * sizeof(siren::u16)), siren::u16{16});
    }

    TEST_CASE("aligned append places values at the requested stride") {
        auto buffer = siren::ByteBuffer{};

        buffer.append(siren::u16{0x1234}, 8);
        buffer.append(siren::u32{0x89abcdef}, 8);

        CHECK_EQ(buffer.size_bytes(), siren::usize{16});
        CHECK_EQ(read_at<siren::u16>(buffer, 0), siren::u16{0x1234});
        CHECK_EQ(read_at<siren::u32>(buffer, 8), siren::u32{0x89abcdef});
    }

    TEST_CASE("data and raw provide mutable and const access") {
        auto buffer = siren::ByteBuffer{siren::u8{1}, siren::u8{2}, siren::u8{3}};

        buffer.data()[1] = siren::u8{9};
        buffer.raw()[2]  = siren::u8{7};

        const auto& const_buffer = buffer;
        CHECK_EQ(const_buffer.data()[0], siren::u8{1});
        CHECK_EQ(const_buffer.raw()[1], siren::u8{9});
        CHECK_EQ(const_buffer.data()[2], siren::u8{7});
    }

    TEST_CASE("typed access reads and mutates homogeneous values") {
        auto buffer = siren::ByteBuffer{siren::u32{10}, siren::u32{20}, siren::u32{30}};

        auto* values = buffer.as<siren::u32>();
        CHECK_EQ(values[0], siren::u32{10});
        CHECK_EQ(values[1], siren::u32{20});
        CHECK_EQ(values[2], siren::u32{30});

        values[1]                = siren::u32{99};
        const auto& const_buffer = buffer;
        const auto* const_values = const_buffer.as<siren::u32>();
        CHECK_EQ(const_values[1], siren::u32{99});
    }

    TEST_CASE("reserve changes capacity without changing contents") {
        auto buffer = siren::ByteBuffer{};

        buffer.reserve_bytes(33);
        CHECK(buffer.empty());
        CHECK_GE(buffer.data().capacity(), siren::usize{33});

        buffer.reserve_as<siren::u64>(8);
        CHECK(buffer.empty());
        CHECK_GE(buffer.data().capacity(), 8 * sizeof(siren::u64));
    }

    TEST_CASE("clear removes contents and permits reuse") {
        auto buffer = siren::ByteBuffer{siren::u32{1}, siren::u32{2}};

        buffer.clear();
        CHECK(buffer.empty());
        CHECK_EQ(buffer.size_bytes(), siren::usize{0});

        buffer.append(siren::u32{77});
        CHECK_FALSE(buffer.empty());
        CHECK_EQ(buffer.size_as<siren::u32>(), siren::usize{1});
        CHECK_EQ(read_at<siren::u32>(buffer, 0), siren::u32{77});
    }

    TEST_CASE("copies own independent contents") {
        const auto original = siren::ByteBuffer{siren::u32{1}, siren::u32{2}};
        auto copy           = original;
        auto assigned       = siren::ByteBuffer{};
        assigned            = original;

        copy.append(siren::u32{3});
        assigned.clear();

        CHECK_EQ(original.size_as<siren::u32>(), siren::usize{2});
        CHECK_EQ(read_at<siren::u32>(original, 0), siren::u32{1});
        CHECK_EQ(read_at<siren::u32>(original, sizeof(siren::u32)), siren::u32{2});
        CHECK_EQ(copy.size_as<siren::u32>(), siren::usize{3});
        CHECK(assigned.empty());
    }

    TEST_CASE("moves preserve the transferred contents") {
        auto source = siren::ByteBuffer{siren::u32{11}, siren::u32{22}};
        auto moved  = siren::ByteBuffer{std::move(source)};

        CHECK_EQ(moved.size_as<siren::u32>(), siren::usize{2});
        CHECK_EQ(read_at<siren::u32>(moved, 0), siren::u32{11});
        CHECK_EQ(read_at<siren::u32>(moved, sizeof(siren::u32)), siren::u32{22});

        auto assigned = siren::ByteBuffer{};
        assigned      = std::move(moved);
        CHECK_EQ(assigned.size_as<siren::u32>(), siren::usize{2});
        CHECK_EQ(read_at<siren::u32>(assigned, 0), siren::u32{11});
        CHECK_EQ(read_at<siren::u32>(assigned, sizeof(siren::u32)), siren::u32{22});
    }
}
