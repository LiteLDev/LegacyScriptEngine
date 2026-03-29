#include "api/ContainerAPI.h"

#include "api/APIHelp.h"
#include "api/ItemAPI.h"
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
Container* ContainerClass::extract(Local<Value> const& v) {
    if (EngineScope::currentEngine()->isInstanceOf<ContainerClass>(v))
        return EngineScope::currentEngine()->getNativeInstance<ContainerClass>(v)->get();
    else return nullptr;
}

// 成员函数
Local<Value> ContainerClass::getSize() const {
    try {
        return Number::newNumber(container->getContainerSize());
    }
    CATCH_AND_THROW
}

Local<Value> ContainerClass::getType() const {
    try {
        return String::newString(container->getTypeName());
    }
    CATCH_AND_THROW
}

Local<Value> ContainerClass::addItem(Arguments const& args) const {
    CHECK_ARGS_COUNT(args, 1);

    try {
        ItemStack* item = ItemClass::extract(args[0]);
        if (!item) {
            throw WrongArgTypeException(__FUNCTION__);
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
    CATCH_AND_THROW
}

Local<Value> ContainerClass::addItemToFirstEmptySlot(Arguments const& args) const {
    CHECK_ARGS_COUNT(args, 1);

    try {
        ItemStack* item = ItemClass::extract(args[0]);
        if (!item) {
            throw WrongArgTypeException(__FUNCTION__);
        }
        return Boolean::newBoolean(container->addItemToFirstEmptySlot(*item));
    }
    CATCH_AND_THROW
}

Local<Value> ContainerClass::hasRoomFor(Arguments const& args) const {
    CHECK_ARGS_COUNT(args, 1);

    try {
        ItemStack* item = ItemClass::extract(args[0]);
        if (!item) {
            throw WrongArgTypeException(__FUNCTION__);
        }
        return Boolean::newBoolean(container->hasRoomForItem(*item));
    }
    CATCH_AND_THROW
}

Local<Value> ContainerClass::removeItem(Arguments const& args) const {
    CHECK_ARGS_COUNT(args, 2);
    CHECK_ARG_TYPE(args[0], ValueKind::kNumber);
    CHECK_ARG_TYPE(args[1], ValueKind::kNumber);

    try {
        container->removeItem(args[0].asNumber().toInt32(), args[1].asNumber().toInt32());
        return Boolean::newBoolean(true);
    }
    CATCH_AND_THROW
}

Local<Value> ContainerClass::getItem(Arguments const& args) const {
    CHECK_ARGS_COUNT(args, 1);
    CHECK_ARG_TYPE(args[0], ValueKind::kNumber);

    try {
        ItemStack* item = &const_cast<ItemStack&>(container->getItem(args[0].asNumber().toInt32()));
        if (!item) {
            throw CreateExceptionWithInfo(__FUNCTION__, "Fail to get slot from container!");
        }
        return ItemClass::newItem(item);
    }
    CATCH_AND_THROW
}

Local<Value> ContainerClass::setItem(Arguments const& args) const {
    CHECK_ARGS_COUNT(args, 2);
    CHECK_ARG_TYPE(args[0], ValueKind::kNumber);

    try {
        ItemStack* item = ItemClass::extract(args[1]);
        if (!item) {
            throw WrongArgTypeException(__FUNCTION__);
        }

        container->setItem(args[0].asNumber().toInt32(), *item);
        return Boolean::newBoolean(true);
    }
    CATCH_AND_THROW
}

Local<Value> ContainerClass::getAllItems(Arguments const&) const {
    try {
        auto list = container->getSlots();

        Local<Array> res = Array::newArray();
        for (auto& item : list) {
            res.add(ItemClass::newItem(const_cast<ItemStack*>(item)));
        }
        return res;
    }
    CATCH_AND_THROW
}

Local<Value> ContainerClass::removeAllItems(Arguments const&) const {
    try {
        container->removeAllItems();
        return Boolean::newBoolean(true);
    }
    CATCH_AND_THROW
}

Local<Value> ContainerClass::isEmpty(Arguments const&) const {
    try {
        return Boolean::newBoolean(container->isEmpty());
    }
    CATCH_AND_THROW
}
