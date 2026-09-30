#include "commands.hpp"

namespace siren {

auto CommandBuffer::statistics() const noexcept -> const Statistics& {
    return m_statistics;
}

} // namespace siren
