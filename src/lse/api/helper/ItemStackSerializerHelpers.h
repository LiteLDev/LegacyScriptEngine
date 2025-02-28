#include "mc/deps/core/utility/BinaryStream.h"
#include "mc/platform/Result.h"

namespace ItemStackSerializerHelpers {
template <typename T>
MCAPI void write(const T& item, BinaryStream& stream);
template <typename T>
MCAPI Bedrock::Result<T> read(ReadOnlyBinaryStream& stream);
} // namespace ItemStackSerializerHelpers