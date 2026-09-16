#pragma once

#include "2iREN/graphics/commands.hpp"
#include "2iREN/graphics/statistics.hpp"

namespace siren {

struct Statistics;

/// @brief Handles trnaslating 2iREN commands to native backend specific commands.
/// Furthermore, these commands will be executed and consumed.
class CommandExecutor {
public:
    virtual ~CommandExecutor() = default;

    /// @brief Translates the command list to the corresponding GPU backend specific
    /// calls and executes them.
    virtual auto execute(CommandList&& cmds) -> void = 0;

    /// @brief Returns the gathered @ref Statistics.
    [[nodiscard]]
    virtual auto statistics() const -> const Statistics& {
        return m_statistics;
    }

protected:
    Statistics m_statistics;
};

} // namespace siren
