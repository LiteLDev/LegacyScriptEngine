#include "api/ContainerAPI.h"

#include "api/APIHelp.h"
#include "api/ItemAPI.h"
#include "ll/api/utils/StringUtils.h"
#include "mc/world/item/ItemStack.h"

//////////////////// Class Definition ////////////////////

ClassDefine<ContainerClass> ContainerClassBuilder =
    defineClass<ContainerClass>("LLSE_Container")
        .constructor(nullptr)
        .instanceProperty("size", &ContainerClass::getSize)
        .instanceProperty("type", &ContainerClass::getType)

        .instanceFunction("addItem", &ContainerClass::addItem)
        .instanceFunction("addItemToFirstEmptySlot", &ContainerClass::addItemToFirstEmptySlot)
        .instanceFunction("hasRoomFor", &ContainerClass::hasRoomFor)
        .instanceFunction("removeItem", &ContainerClass::removeItem)
        .instanceFunction("getItem", &ContainerClass::getItem)
        .instanceFunction("setItem", &ContainerClass::setItem)
        .instanceFunction("getAllItems", &ContainerClass::getAllItems)
        .instanceFunction("removeAllItems", &ContainerClass::removeAllItems)
        .instanceFunction("isEmpty", &ContainerClass::isEmpty)

        // For Compatibility
        .instanceFunction("getSlot", &ContainerClass::getItem)
        .instanceFunction("getAllSlots", &ContainerClass::getAllItems)
        .build();

//////////////////// Classes ////////////////////

ContainerClass::ContainerClass(Container* p)
: ScriptClass(ScriptClass::ConstructFromCpp<ContainerClass>{}),
  container(p) {}

// 生成函数
Local<Object> ContainerClass::newContainer(Container* p) {
    auto newp = new ContainerClass(p);
    return newp->getScriptObject();
}
Container* ContainerClass::extract(Local<Value> v) {
    if (EngineScope::currentEngine()->isInstanceOf<ContainerClass>(v))
        return EngineScope::currentEngine()->getNativeInstance<ContainerClass>(v)->get();
    else return nullptr;
}

// 成员函数
Local<Value> ContainerClass::getSize() {
    try {
        return Number::newNumber(container->getContainerSize());
    }
    CATCH("Fail in getSize!")
}

Local<Value> ContainerClass::getType() {
    try {
        return String::newString(container->getTypeName());
    }
    CATCH("Fail in getType!")
}

Local<Value> ContainerClass::addItem(const Arguments& args) {
    CHECK_ARGS_COUNT(args, 1);

    try {
        ItemStack* item = ItemClass::extract(args[0]);
        if (!item) {
            LOG_WRONG_ARG_TYPE(__FUNCTION__);
            return Local<Value>();
        }
        if (args.size() >= 2) {
            CHECK_ARG_TYPE(args[1], ValueKind::kNumber);
            item->set(args[1].asNumber().toInt32());
            if (container->addItem(*item)) {
                return Boolean::newBoolean(false);
            }
            return Boolean::newBoolean(true);
        }
        return Boolean::newBoolean(container->addItem(*item));
    }
    CATCH("Fail in addItem!");
}

Local<Value> ContainerClass::addItemToFirstEmptySlot(const Arguments& args) {
    CHECK_ARGS_COUNT(args, 1);

    try {
        ItemStack* item = ItemClass::extract(args[0]);
        if (!item) {
            LOG_WRONG_ARG_TYPE(__FUNCTION__);
            return Local<Value>();
        }
        return Boolean::newBoolean(container->addItemToFirstEmptySlot(*item));
    }
    CATCH("Fail in addItemToFirstEmptySlot!");
}

Local<Value> ContainerClass::hasRoomFor(const Arguments& args) {
    CHECK_ARGS_COUNT(args, 1);

    try {
        ItemStack* item = ItemClass::extract(args[0]);
        if (!item) {
            LOG_WRONG_ARG_TYPE(__FUNCTION__);
            return Local<Value>();
        }
        return Boolean::newBoolean(container->hasRoomForItem(*item));
    }
    CATCH("Fail in hasRoomFor!");
}

Local<Value> ContainerClass::removeItem(const Arguments& args) {
    CHECK_ARGS_COUNT(args, 2);
    CHECK_ARG_TYPE(args[0], ValueKind::kNumber);
    CHECK_ARG_TYPE(args[1], ValueKind::kNumber);

    try {
        container->removeItem(args[0].asNumber().toInt32(), args[1].asNumber().toInt32());
        return Boolean::newBoolean(true);
    }
    CATCH("Fail in removeItem!");
}

Local<Value> ContainerClass::getItem(const Arguments& args) {
    CHECK_ARGS_COUNT(args, 1);
    CHECK_ARG_TYPE(args[0], ValueKind::kNumber);

    try {
        ItemStack* item = &const_cast<ItemStack&>(container->getItem(args[0].asNumber().toInt32()));
        if (!item) {
            LOG_ERROR_WITH_SCRIPT_INFO(__FUNCTION__, "Fail to get slot from container!");
            return Local<Value>();
        }
        return ItemClass::newItem(item);
    }
    CATCH("Fail in getItem!");
}

Local<Value> ContainerClass::setItem(const Arguments& args) {
    CHECK_ARGS_COUNT(args, 2);
    CHECK_ARG_TYPE(args[0], ValueKind::kNumber);

    try {
        ItemStack* item = ItemClass::extract(args[1]);
        if (!item) {
            LOG_WRONG_ARG_TYPE(__FUNCTION__);
            return Local<Value>();
        }

        container->setItem(args[0].asNumber().toInt32(), *item);
        return Boolean::newBoolean(true);
    }
    CATCH("Fail in getItem!");
}

Local<Value> ContainerClass::getAllItems(const Arguments&) {
    try {
        auto list = container->getSlots();

        Local<Array> res = Array::newArray();
        for (auto& item : list) {
            res.add(ItemClass::newItem(const_cast<ItemStack*>(item)));
        }
        return res;
    }
    CATCH("Fail in getAllItems!");
}

Local<Value> ContainerClass::removeAllItems(const Arguments&) {
    try {
        container->removeAllItems();
        return Boolean::newBoolean(true);
    }
    CATCH("Fail in removeAllItems!");
}

Local<Value> ContainerClass::isEmpty(const Arguments&) {
    try {
        return Boolean::newBoolean(container->isEmpty());
    }
    CATCH("Fail in isEmpty!");
}
