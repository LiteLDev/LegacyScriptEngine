#include "api/CommandOutputAPI.h"

#include "ll/api/service/Bedrock.h"
#include "mc/server/commands/CommandOutputMessageType.h"
#include "mc/server/commands/CommandPropertyBag.h"
#include "mc/server/commands/MinecraftCommands.h"
#include "mc/world/Minecraft.h"

//////////////////// Class Definition ////////////////////

ClassDefine<CommandOutputClass> CommandOutputClassBuilder =
    defineClass<CommandOutputClass>("LLSE_CommandOutput")
        .constructor(nullptr)
        .instanceProperty("empty", &CommandOutputClass::empty)
        //.instanceProperty("type", &CommandOutputClass::getType)
        .instanceProperty("successCount", &CommandOutputClass::getSuccessCount)

        .instanceFunction("success", &CommandOutputClass::success)
        .instanceFunction("error", &CommandOutputClass::error)
        .instanceFunction("addMessage", &CommandOutputClass::addMessage)
        .instanceFunction("toString", &CommandOutputClass::toString)

        .build();

//////////////////// APIs ////////////////////

CommandOutputClass::CommandOutputClass(std::shared_ptr<CommandOutput> out, std::shared_ptr<CommandOrigin const> ori)
: ScriptClass(ScriptClass::ConstructFromCpp<CommandOutputClass>{}),
  output(out),
  origin(ori),
  isAsync(false) {};

Local<Value> CommandOutputClass::empty() {
    try {
        return Boolean::newBoolean(get()->mMessages.empty());
    }
    CATCH("Fail in empty!");
}

Local<Value> CommandOutputClass::getSuccessCount() {
    try {
        return Number::newNumber(static_cast<int64_t>(get()->mSuccessCount));
    }
    CATCH("Fail in getSuccessCount!");
};

// Local<Value> CommandOutputClass::getType()
//{
//     try
//     {
//         return String::newString(magic_enum::enum_name(get()->getType()));
//     }
//     CATCH("Fail in getType!");
// };

Local<Value> CommandOutputClass::success(const Arguments& args) {
    try {
        if (args.size() == 0) {
            ++get()->mSuccessCount;
            return Boolean::newBoolean(true);
        }
        CHECK_ARG_TYPE(args[0], ValueKind::kString);
        auto msg = args[0].asString().toString();
        if (args.size() >= 2) {
            CHECK_ARG_TYPE(args[1], ValueKind::kArray);
            std::vector<CommandOutputParameter> param{};
            auto                                paramArr = args[1].asArray();
            for (int i = 0; i < paramArr.size(); ++i) {
                param.push_back(CommandOutputParameter(paramArr.get(i).asString().toString().c_str()));
            }
            get()->success(msg, param);
            send();
            return Boolean::newBoolean(true);
        }
        get()->success(msg);
        send();
        return Boolean::newBoolean(true);
    }
    CATCH("Fail in success!");
};

Local<Value> CommandOutputClass::addMessage(const Arguments& args) {
    try {
        CHECK_ARG_TYPE(args[0], ValueKind::kString);
        auto msg = args[0].asString().toString();
        if (args.size() >= 2) {
            CHECK_ARG_TYPE(args[1], ValueKind::kArray);
            std::vector<CommandOutputParameter> param{};
            auto                                paramArr = args[1].asArray();
            for (int i = 0; i < paramArr.size(); ++i) {
                param.push_back(CommandOutputParameter(paramArr.get(i).asString().toString().c_str()));
            }
            if (args.size() >= 3) {
                CHECK_ARG_TYPE(args[2], ValueKind::kNumber);
                get()->addMessage(msg, param, (CommandOutputMessageType)args[2].asNumber().toInt32());
                send();
                return Boolean::newBoolean(true);
            }
            get()->addMessage(msg, param, (CommandOutputMessageType)0);
            send();
            return Boolean::newBoolean(true);
        }
        get()->addMessage(msg, {}, CommandOutputMessageType::Success);
        send();
        return Boolean::newBoolean(true);
    }
    CATCH("Fail in addMessage!");
};

Local<Value> CommandOutputClass::error(const Arguments& args) {
    CHECK_ARGS_COUNT(args, 1);
    CHECK_ARG_TYPE(args[0], ValueKind::kString);
    try {
        auto msg = args[0].asString().toString();
        if (args.size() >= 2) {
            CHECK_ARG_TYPE(args[1], ValueKind::kArray);
            std::vector<CommandOutputParameter> param{};
            auto                                paramArr = args[1].asArray();
            for (int i = 0; i < paramArr.size(); ++i) {
                param.push_back(CommandOutputParameter(paramArr.get(i).asString().toString().c_str()));
            }
            get()->error(msg, param);
            send();
            return Boolean::newBoolean(true);
        }
        get()->error(msg);
        send();
        return Boolean::newBoolean(true);
    }
    CATCH("Fail in error!");
};

void CommandOutputClass::send() {
    try {
        if (!isAsync) return;
        ll::service::getMinecraft()->mCommands->handleOutput(*origin, *output);
        output->mSuccessCount = 0;
        output->mMessages.clear();
    }
    CATCH_WITHOUT_RETURN("Fail in sendCommandOutput!");
}

Local<Value> CommandOutputClass::toString(const Arguments&) {
    try {
        return String::newString("<CommandOutput>");
    }
    CATCH("Fail in toString!");
};
