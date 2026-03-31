#pragma once
#include "mc/platform/Result.h"

namespace ItemStackSerializerHelpers {
template <typename T>
MCAPI void write(T const& item, BinaryStream& stream);
template <typename T>
MCAPI Bedrock::Result<T> read(ReadOnlyBinaryStream& stream);
} // namespace ItemStackSerializerHelpers
