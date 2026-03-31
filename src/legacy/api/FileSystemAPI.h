#pragma once
#include "legacy/api/APIHelp.h"

#include <fstream>
#include <mutex>
#include <string>

//////////////////// APIs ////////////////////

Local<Value> OpenFile(Arguments const& args);

Local<Value> FileReadFrom(Arguments const& args);
Local<Value> FileWriteTo(Arguments const& args);
Local<Value> FileWriteLine(Arguments const& args);

Local<Value> DirCreate(Arguments const& args);
Local<Value> PathCopy(Arguments const& args);
Local<Value> PathMove(Arguments const& args);
Local<Value> PathRename(Arguments const& args);
Local<Value> PathDelete(Arguments const& args);
Local<Value> PathExists(Arguments const& args);
Local<Value> CheckIsDir(Arguments const& args);
Local<Value> GetFileSize(Arguments const& args);
Local<Value> GetFilesList(Arguments const& args);

//////////////////// Classes ////////////////////
class FileClass : public ScriptClass {
private:
    std::fstream file;
    std::string  path;
    bool         isBinary;
    std::mutex   lock;

public:
    explicit FileClass(Local<Object> const& scriptObj, std::fstream&& f, std::string const& path, bool isBinary);
    explicit FileClass(std::fstream&& f, std::string const& path, bool isBinary);
    static FileClass* constructor(Arguments const& args);

    Local<Value> getPath() const;
    Local<Value> getAbsolutePath() const;
    Local<Value> getSize();

    Local<Value> readSync(Arguments const& args);
    Local<Value> readLineSync(Arguments const& args);
    Local<Value> readAllSync(Arguments const& args);
    Local<Value> writeSync(Arguments const& args);
    Local<Value> writeLineSync(Arguments const& args);

    Local<Value> read(Arguments const& args);
    Local<Value> readLine(Arguments const& args);
    Local<Value> readAll(Arguments const& args);
    Local<Value> write(Arguments const& args);
    Local<Value> writeLine(Arguments const& args);

    Local<Value> seekTo(Arguments const& args);
    Local<Value> setSize(Arguments const& args);
    Local<Value> close(Arguments const& args);
    Local<Value> isEOF(Arguments const& args) const;
    Local<Value> flush(Arguments const& args);
    Local<Value> errorCode(Arguments const& args);
    Local<Value> clear(Arguments const& args);

    static Local<Value> readFromStatic(Arguments const& args) { return FileReadFrom(args); }
    static Local<Value> writeToStatic(Arguments const& args) { return FileWriteTo(args); }
    static Local<Value> writeLineStatic(Arguments const& args) { return FileWriteLine(args); }

    static Local<Value> createDir(Arguments const& args) { return DirCreate(args); }
    static Local<Value> copy(Arguments const& args) { return PathCopy(args); }
    static Local<Value> move(Arguments const& args) { return PathMove(args); }
    static Local<Value> rename(Arguments const& args) { return PathRename(args); }
    static Local<Value> del(Arguments const& args) { return PathDelete(args); }
    static Local<Value> exists(Arguments const& args) { return PathExists(args); }
    static Local<Value> checkIsDir(Arguments const& args) { return CheckIsDir(args); }
    static Local<Value> getFileSize(Arguments const& args) { return GetFileSize(args); }
    static Local<Value> getFilesList(Arguments const& args) { return GetFilesList(args); }

    // For Compatibility
    static Local<Object> newFile(std::fstream&& f, std::string const& path, bool isBinary);
    static Local<Value>  open(Arguments const& args) { return OpenFile(args); }
};
extern ClassDefine<FileClass> FileClassBuilder;
