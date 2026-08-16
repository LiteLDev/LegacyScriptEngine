#pragma once

#include "legacy/utils/UsingScriptX.h"

#include <ll/api/io/Logger.h>

namespace legacy::script_error {

void printException(script::Exception const& exception, ll::io::Logger& logger);

void printCurrentException(ll::io::Logger& logger);

} // namespace legacy::script_error
