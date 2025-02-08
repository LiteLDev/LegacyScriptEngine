#include "ll/api/io/Formatter.h"

namespace lse::io {
using namespace ll::io;
class DirectFormatter : public Formatter {
public:
    DirectFormatter()                   = default;
    virtual ~DirectFormatter() override = default;

    void format(LogMessageView const& view, std::string& buffer) const noexcept override;
};

} // namespace lse::io