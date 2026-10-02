#pragma once

#include <optional>

#include "2iREN/graphics/device.hpp"
#include "2iREN/graphics/fwd.hpp"
#include "2iREN/graphics/resource.hpp"

namespace siren {

/// @brief The different data requests that can be made using a query.
enum class QueryKind {
    None,
    NumberSamplesPassed,
    AnySampledPassed,
};

/// @brief Parameters used to describe the creation of a query.
struct QueryDescriptor {
    /// @brief An optional label.
    Label label = std::nullopt;
    /// @brief The type information to request from the query.
    QueryKind kind;
};

/// @brief A Query represents a way to gather information on Gpu work done.
class Query final : public RenderResource<Query> {
    using Base = RenderResource<Query>;

public:
    Query(Device* device, QueryHandle handle);
    ~Query();
    Query(Query&& other) noexcept;
    Query& operator=(Query&& other) noexcept;

    /// @brief Returns the descriptor of this @ref Query.
    [[nodiscard]]
    auto descriptor() const -> const QueryDescriptor&;
};

} // namespace siren
