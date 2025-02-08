#include "lse/api/DirectFormatter.h"

namespace lse::io {
void DirectFormatter::format(const ll::io::LogMessageView& view, std::string& buffer) const noexcept {
    buffer = view.msg;
}
} // namespace lse::io