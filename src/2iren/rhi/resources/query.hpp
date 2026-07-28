#pragma once

#include "resource.hpp"
#include "fwd.hpp"

namespace siren {
struct QueryKind {
    enum Value {
        None = 0,
        /** @brief Counts the number of samples that passed depth/stencil tests. */
        SamplesPassed,
        /** @brief Checks whether any sample passed depth/stencil tests. */
        AnySamplesPassed,
        /** @brief Measures GPU time elapsed between query begin and end. */
        TimeElapsed,
    } value;

    QueryKind() : value(None) {}
    // ReSharper disable once CppNonExplicitConvertingConstructor
    constexpr QueryKind(const Value value) noexcept : value(value) {}
    // ReSharper disable once CppNonExplicitConversionOperator
    constexpr operator Value() const { return value; }
};

struct QueryDescriptor {
    QueryKind kind;
};

class Query final : public RenderResource<Query> {
    using Base = RenderResource<Query>;

public:
    Query(Device* device, QueryHandle handle);
    ~Query();
    Query(Query&& other) noexcept;
    Query& operator=(Query&& other) noexcept;

    /** @brief Returns the descriptor of this @ref Query. */
    [[nodiscard]] auto descriptor() const -> const QueryDescriptor&;
};
} // namespace siren
