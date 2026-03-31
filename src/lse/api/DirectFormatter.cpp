#include "lse/api/DirectFormatter.h"

namespace lse::io {
void DirectFormatter::format(LogMessageView const& view, std::string& buffer) const noexcept { buffer = view.msg; }
} // namespace lse::io
