#pragma once
#include "legacy/api/APIHelp.h"

//////////////////// Classes ////////////////////
class Objective;
class ObjectiveClass : public ScriptClass {
    std::string objname;
    bool        isValid = false;

public:
    explicit ObjectiveClass(Objective* obj) : ScriptClass(ScriptClass::ConstructFromCpp<ObjectiveClass>{}) { set(obj); }

    void                 set(Objective* obj);
    Objective*           get() const;
    static Local<Object> newObjective(Objective* obj);
    Local<Value>         getName() const;
    Local<Value>         getDisplayName() const;
    Local<Value>         setDisplay(Arguments const& args) const;
    Local<Value>         setScore(Arguments const& args) const;
    Local<Value>         addScore(Arguments const& args) const;
    Local<Value>         reduceScore(Arguments const& args) const;
    Local<Value>         deleteScore(Arguments const& args) const;
    Local<Value>         getScore(Arguments const& args) const;
};
extern ClassDefine<ObjectiveClass> ObjectiveClassBuilder;
