#pragma once
#include "legacy/api/APIHelp.h"

//////////////////// Classes ////////////////////
class Container;
class ContainerClass : public ScriptClass {
private:
    // Container is managed by BDS, so use raw pointer
    Container* container;

public:
    explicit ContainerClass(Container* p);

    Container* get() const { return container; }

    static Local<Object> newContainer(Container* p);
    static Container*    extract(Local<Value> const& v);

    Local<Value> getSize() const;
    Local<Value> getType() const;

    Local<Value> addItem(Arguments const& args) const;
    Local<Value> addItemToFirstEmptySlot(Arguments const& args) const;
    Local<Value> hasRoomFor(Arguments const& args) const;
    Local<Value> removeItem(Arguments const& args) const;
    Local<Value> getItem(Arguments const& args) const;
    Local<Value> setItem(Arguments const& args) const;
    Local<Value> getAllItems(Arguments const& args) const;
    Local<Value> removeAllItems(Arguments const& args) const;
    Local<Value> isEmpty(Arguments const& args) const;
};
extern ClassDefine<ContainerClass> ContainerClassBuilder;
