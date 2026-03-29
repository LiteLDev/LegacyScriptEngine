#include "api/FileSystemAPI.h"

#include "api/APIHelp.h"
#include "engine/EngineManager.h"
#include "engine/LocalShareData.h"
#include "engine/TimeTaskSystem.h"
#include "ll/api/service/GamingStatus.h"
#include "ll/api/utils/StringUtils.h"

#include <filesystem>
#include <fstream>
#include <ll/api/io/FileUtils.h>
#include <string>
#include <system_error>

using namespace std::filesystem;

//////////////////// Class Definition ////////////////////

enum class FileOpenMode : int { ReadMode, WriteMode, AppendMode };

ClassDefine<FileClass> FileClassBuilder =
    defineClass<FileClass>("File")
        .constructor(&FileClass::constructor)
        .instanceProperty("path", &FileClass::getPath)
        .instanceProperty("absolutePath", &FileClass::getAbsolutePath)
        .instanceProperty("size", &FileClass::getSize)

        .instanceFunction("readSync", &FileClass::readSync)
        .instanceFunction("readLineSync", &FileClass::readLineSync)
        .instanceFunction("readAllSync", &FileClass::readAllSync)
        .instanceFunction("writeSync", &FileClass::writeSync)
        .instanceFunction("writeLineSync", &FileClass::writeLineSync)

        .instanceFunction("read", &FileClass::read)
        .instanceFunction("readLine", &FileClass::readLine)
        .instanceFunction("readAll", &FileClass::readAll)
        .instanceFunction("write", &FileClass::write)
        .instanceFunction("writeLine", &FileClass::writeLine)

        .instanceFunction("isEOF", &FileClass::isEOF)
        .instanceFunction("seekTo", &FileClass::seekTo)
        .instanceFunction("setSize", &FileClass::setSize)
        .instanceFunction("close", &FileClass::close)
        .instanceFunction("flush", &FileClass::flush)
        .instanceFunction("errorCode", &FileClass::errorCode)
        .instanceFunction("clear", &FileClass::clear)

        .property("ReadMode", [] { return Number::newNumber(static_cast<int>(FileOpenMode::ReadMode)); })
        .property("WriteMode", [] { return Number::newNumber(static_cast<int>(FileOpenMode::WriteMode)); })
        .property("AppendMode", [] { return Number::newNumber(static_cast<int>(FileOpenMode::AppendMode)); })

        .function("readFrom", &FileClass::readFromStatic)
        .function("writeTo", &FileClass::writeToStatic)
        .function("writeLine", &FileClass::writeLineStatic)

        .function("createDir", &FileClass::createDir)
        .function("mkdir", &FileClass::createDir)
        .function("copy", &FileClass::copy)
        .function("move", &FileClass::move)
        .function("rename", &FileClass::rename)
        .function("delete", &FileClass::del)
        .function("exists", &FileClass::exists)
        .function("checkIsDir", &FileClass::checkIsDir)
        .function("getFileSize", &FileClass::getFileSize)
        .function("getFilesList", &FileClass::getFilesList)

        // For Compatibility
        .function("open", &FileClass::open)
        .build();

//////////////////// Classes ////////////////////

// 生成函数
FileClass::FileClass(Local<Object> const& scriptObj, std::fstream&& f, std::string const& path, bool isBinary)
: ScriptClass(scriptObj) {
    this->file     = std::move(f);
    this->path     = path;
    this->isBinary = isBinary;
}

FileClass::FileClass(std::fstream&& f, std::string const& path, bool isBinary)
: ScriptClass(ScriptClass::ConstructFromCpp<FileClass>{}) {
    this->file     = std::move(f);
    this->path     = path;
    this->isBinary = isBinary;
}

FileClass* FileClass::constructor(Arguments const& args) {
    CHECK_ARGS_COUNT(args, 2);
    CHECK_ARG_TYPE(args[0], ValueKind::kString);
    CHECK_ARG_TYPE(args[1], ValueKind::kNumber);
    if (args.size() >= 3) CHECK_ARG_TYPE(args[2], ValueKind::kBoolean);

    try {
        std::filesystem::path path(args[0].asString().toU8string());
        if (!path.empty()) {
            if (path.has_parent_path()) {
                std::filesystem::create_directories(path.parent_path());
            }
        } else {
            throw CreateExceptionWithInfo(__FUNCTION__, "File " + args[0].asString().toString() + " doesn't exist!");
        }
        FileOpenMode fMode = static_cast<FileOpenMode>(args[1].asNumber().toInt32());
        // Auto Create
        if (fMode == FileOpenMode::ReadMode || fMode == FileOpenMode::WriteMode) {
            std::fstream tmp(path, std::ios_base::app);
            tmp.flush();
            tmp.close();
        }

        std::ios_base::openmode mode = std::ios_base::in;
        if (fMode == FileOpenMode::WriteMode) {
            mode |= std::ios_base::out;
            // mode |= ios_base::ate;
            mode |= std::ios_base::trunc;
        } else if (fMode == FileOpenMode::AppendMode) {
            mode |= std::ios_base::app;
        }

        bool isBinary = false;
        if (args.size() >= 3 && args[2].asBoolean().value()) {
            isBinary  = true;
            mode     |= std::ios_base::binary;
        }

        std::fstream fs(path, mode);
        if (!fs.is_open()) {
            throw CreateExceptionWithInfo(__FUNCTION__, "Fail to Open File " + path.string() + "!");
        }
        return new FileClass(args.thiz(), std::move(fs), path.string(), isBinary);
    }
    CATCH_AND_THROW
}

// 成员函数
Local<Value> FileClass::getPath() const {
    try {
        return String::newString(path);
    }
    CATCH_AND_THROW
}

Local<Value> FileClass::getAbsolutePath() const {
    try {
        return String::newString(canonical(std::filesystem::path(ll::string_utils::str2wstr(path))).u8string());
    }
    CATCH_AND_THROW
}

Local<Value> FileClass::getSize() {
    try {
        size_t cur = file.tellg();
        file.seekg(0, file.end);
        size_t size = file.tellg();
        file.seekg(cur, file.beg);

        return Number::newNumber(static_cast<long long>(size));
    }
    CATCH_AND_THROW
}

Local<Value> FileClass::readSync(Arguments const& args) {
    CHECK_ARGS_COUNT(args, 1);
    CHECK_ARG_TYPE(args[0], ValueKind::kNumber);

    try {
        int               cnt = args[0].asNumber().toInt32();
        std::vector<char> buf(cnt + 1);
        file.read(buf.data(), cnt);
        size_t bytes = file.gcount();

        Local<Value> res = isBinary ? ByteBuffer::newByteBuffer(buf.data(), bytes).asValue()
                                    : String::newString(std::string_view(buf.data(), bytes)).asValue();
        return res;
    }
    CATCH_AND_THROW
}

Local<Value> FileClass::readLineSync(Arguments const&) {
    try {
        std::string buf;
        getline(file, buf);
        return String::newString(buf);
    }
    CATCH_AND_THROW
}

Local<Value> FileClass::readAllSync(Arguments const&) {
    try {
        std::string res((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
        return isBinary ? ByteBuffer::newByteBuffer(res.data(), res.size()).asValue()
                        : String::newString(res).asValue();
    }
    CATCH_AND_THROW
}

Local<Value> FileClass::writeSync(Arguments const& args) {
    CHECK_ARGS_COUNT(args, 1);

    try {
        if (args[0].isString()) {
            file << args[0].asString().toString();
        } else if (args[0].isByteBuffer()) {
            file.write(static_cast<char*>(args[0].asByteBuffer().getRawBytes()), args[0].asByteBuffer().byteLength());
        } else {
            throw WrongArgTypeException(__FUNCTION__);
        }
        return Boolean::newBoolean(!file.fail() && !file.bad());
    }
    CATCH_AND_THROW
}

Local<Value> FileClass::writeLineSync(Arguments const& args) {
    CHECK_ARGS_COUNT(args, 1);
    CHECK_ARG_TYPE(args[0], ValueKind::kString);

    try {
        file << args[0].asString().toString() << "\n";
        return Boolean::newBoolean(!file.fail() && !file.bad());
    }
    CATCH_AND_THROW
}

Local<Value> FileClass::read(Arguments const& args) {
    CHECK_ARGS_COUNT(args, 2);
    CHECK_ARG_TYPE(args[0], ValueKind::kNumber);
    CHECK_ARG_TYPE(args[1], ValueKind::kFunction);

    try {
        int                      cnt = args[0].asNumber().toInt32();
        script::Global<Function> callbackFunc{args[1].asFunction()};

        pool.execute(
            [cnt,
             fp{&file},
             isBinary{isBinary},
             lock{&lock},
             callback{std::move(callbackFunc)},
             engine{EngineScope::currentEngine()}]() -> void {
                if ((ll::getGamingStatus() != ll::GamingStatus::Running)) return;
                if (!EngineManager::isValid(engine)) return;

                std::vector<char> buf(cnt + 1);
                lock->lock();
                fp->read(buf.data(), cnt);
                size_t bytes = fp->gcount();
                lock->unlock();

                EngineScope scope(engine);
                try {
                    Local<Value> res = isBinary ? ByteBuffer::newByteBuffer(buf.data(), bytes).asValue()
                                                : String::newString(std::string_view(buf.data(), bytes)).asValue();
                    // dangerous
                    NewTimeout(callback.get(), {res}, 1);
                }
                CATCH_IN_CALLBACK("ReadFile")
            }
        );
        return Boolean::newBoolean(true);
    }
    CATCH_AND_THROW
}

Local<Value> FileClass::readLine(Arguments const& args) {
    CHECK_ARGS_COUNT(args, 1);
    CHECK_ARG_TYPE(args[0], ValueKind::kFunction);

    try {
        script::Global<Function> callbackFunc{args[0].asFunction()};

        pool.execute(
            [fp{&file}, lock{&lock}, callback{std::move(callbackFunc)}, engine{EngineScope::currentEngine()}]() {
                if ((ll::getGamingStatus() != ll::GamingStatus::Running)) return;
                if (!EngineManager::isValid(engine)) return;

                std::string buf;
                lock->lock();
                getline(*fp, buf);
                lock->unlock();

                EngineScope scope(engine);
                try {
                    NewTimeout(callback.get(), {String::newString(buf)}, 1);
                }
                CATCH_IN_CALLBACK("FileReadLine")
            }
        );
        return Boolean::newBoolean(true);
    }
    CATCH_AND_THROW
}

Local<Value> FileClass::readAll(Arguments const& args) {
    CHECK_ARGS_COUNT(args, 1);
    CHECK_ARG_TYPE(args[0], ValueKind::kFunction);

    try {
        script::Global<Function> callbackFunc{args[0].asFunction()};

        pool.execute([fp{&file},
                      isBinary{isBinary},
                      lock{&lock},
                      callback{std::move(callbackFunc)},
                      engine{EngineScope::currentEngine()}]() {
            if ((ll::getGamingStatus() != ll::GamingStatus::Running)) return;
            if (!EngineManager::isValid(engine)) return;

            lock->lock();
            std::string res((std::istreambuf_iterator<char>(*fp)), std::istreambuf_iterator<char>());
            lock->unlock();

            EngineScope scope(engine);
            try {
                Local<Value> data = isBinary ? ByteBuffer::newByteBuffer(res.data(), res.size()).asValue()
                                             : String::newString(res).asValue();
                NewTimeout(callback.get(), {data}, 1);
            }
            CATCH_IN_CALLBACK("FileReadAll")
        });
        return Boolean::newBoolean(true);
    }
    CATCH_AND_THROW
}

Local<Value> FileClass::write(Arguments const& args) {
    CHECK_ARGS_COUNT(args, 1);
    if (args.size() >= 2) CHECK_ARG_TYPE(args[1], ValueKind::kFunction);

    try {
        std::string data;
        bool        isString = true;
        if (args[0].isString()) {
            data = std::move(args[0].asString().toString());
        } else if (args[0].isByteBuffer()) {
            isString = false;
            data     = std::move(
                std::string(
                    static_cast<char*>(args[0].asByteBuffer().getRawBytes()),
                    args[0].asByteBuffer().byteLength()
                )
            );
        } else {
            throw WrongArgTypeException(__FUNCTION__);
        }

        script::Global<Function> callbackFunc;
        if (args.size() >= 2) callbackFunc = args[1].asFunction();

        pool.execute([fp{&file},
                      lock{&lock},
                      data{std::move(data)},
                      isString,
                      callback{std::move(callbackFunc)},
                      engine{EngineScope::currentEngine()}]() {
            if ((ll::getGamingStatus() != ll::GamingStatus::Running)) return;
            if (!EngineManager::isValid(engine)) return;

            lock->lock();
            if (isString) *fp << data;
            else fp->write(data.data(), data.size());
            bool isOk = !fp->fail() && !fp->bad();
            lock->unlock();

            if (!callback.isEmpty()) {
                EngineScope scope(engine);
                try {
                    NewTimeout(callback.get(), {Boolean::newBoolean(isOk)}, 1);
                }
                CATCH_IN_CALLBACK("WriteFile")
            }
        });
        return Boolean::newBoolean(true);
    }
    CATCH_AND_THROW
}

Local<Value> FileClass::writeLine(Arguments const& args) {
    CHECK_ARGS_COUNT(args, 1);
    CHECK_ARG_TYPE(args[0], ValueKind::kString);
    if (args.size() >= 2) CHECK_ARG_TYPE(args[1], ValueKind::kFunction);

    try {
        std::string data{args[0].asString().toString()};

        script::Global<Function> callbackFunc;
        if (args.size() >= 2) callbackFunc = args[1].asFunction();

        pool.execute([fp{&file},
                      lock{&lock},
                      data{std::move(data)},
                      callback{std::move(callbackFunc)},
                      engine{EngineScope::currentEngine()}]() {
            if ((ll::getGamingStatus() != ll::GamingStatus::Running)) return;
            if (!EngineManager::isValid(engine)) return;

            lock->lock();
            *fp << data << "\n";
            bool isOk = !fp->fail() && !fp->bad();
            lock->unlock();

            if (!callback.isEmpty()) {
                EngineScope scope(engine);
                try {
                    NewTimeout(callback.get(), {Boolean::newBoolean(isOk)}, 1);
                }
                CATCH_IN_CALLBACK("FileWriteLine")
            }
        });
        return Boolean::newBoolean(true);
    }
    CATCH_AND_THROW
}

Local<Value> FileClass::seekTo(Arguments const& args) {
    CHECK_ARGS_COUNT(args, 2);
    CHECK_ARG_TYPE(args[0], ValueKind::kNumber);
    CHECK_ARG_TYPE(args[1], ValueKind::kBoolean);

    try {
        int pos = args[0].asNumber().toInt32();
        if (args[1].asBoolean().value()) {
            // relative
            std::ios_base::seekdir way = std::ios_base::cur;
            file.seekg(pos, way);
            file.seekp(pos, way);
        } else {
            // absolute
            if (pos >= 0) {
                std::ios_base::seekdir way = std::ios_base::beg;
                file.seekg(pos, way);
                file.seekp(pos, way);
            } else {
                std::ios_base::seekdir way = std::ios_base::end;
                file.seekg(0, way);
                file.seekp(0, way);
            }
        }
        return Boolean::newBoolean(!file.fail() && !file.bad());
    }
    CATCH_AND_THROW
}

Local<Value> FileClass::setSize(Arguments const& args) {
    CHECK_ARGS_COUNT(args, 1);
    CHECK_ARG_TYPE(args[0], ValueKind::kNumber);

    try {
        return Boolean::newBoolean(false);
    }
    CATCH_AND_THROW
}

Local<Value> FileClass::close(Arguments const&) {
    try {
        file.close();
        return Boolean::newBoolean(true);
    }
    CATCH_AND_THROW
}

Local<Value> FileClass::isEOF(Arguments const&) const {
    try {
        return Boolean::newBoolean(file.eof());
    }
    CATCH_AND_THROW
}

Local<Value> FileClass::flush(Arguments const&) {
    try {
        pool.execute([fp{&file}, lock{&lock}]() {
            lock->lock();
            fp->flush();
            lock->unlock();
        });
        return Boolean::newBoolean(true);
    }
    CATCH_AND_THROW
}

Local<Value> FileClass::errorCode(Arguments const&) {
    try {
        file.flush();
        return Number::newNumber(errno);
    }
    CATCH_AND_THROW
}

Local<Value> FileClass::clear(Arguments const&) {
    try {
        file.clear();
        return Boolean::newBoolean(true);
    }
    CATCH_AND_THROW
}

//////////////////// APIs ////////////////////

Local<Value> DirCreate(Arguments const& args) {
    CHECK_ARGS_COUNT(args, 1);
    CHECK_ARG_TYPE(args[0], ValueKind::kString);

    try {
        return Boolean::newBoolean(std::filesystem::create_directories(args[0].asString().toU8string()));
    }
    CATCH_AND_THROW
}

Local<Value> PathDelete(Arguments const& args) {
    CHECK_ARGS_COUNT(args, 1);
    CHECK_ARG_TYPE(args[0], ValueKind::kString);

    try {
        return Boolean::newBoolean(
            std::filesystem::remove_all(ll::string_utils::str2wstr(args[0].asString().toString())) > 0
        );
    }
    CATCH_AND_THROW
}

Local<Value> PathExists(Arguments const& args) {
    CHECK_ARGS_COUNT(args, 1);
    CHECK_ARG_TYPE(args[0], ValueKind::kString);

    try {
        return Boolean::newBoolean(std::filesystem::exists(ll::string_utils::str2wstr(args[0].asString().toString())));
    }
    CATCH_AND_THROW
}

Local<Value> PathCopy(Arguments const& args) {
    CHECK_ARGS_COUNT(args, 2);
    CHECK_ARG_TYPE(args[0], ValueKind::kString);
    CHECK_ARG_TYPE(args[1], ValueKind::kString);

    try {
        copy(
            ll::string_utils::str2wstr(args[0].asString().toString()),
            ll::string_utils::str2wstr(args[1].asString().toString())
        );
        return Boolean::newBoolean(true);
    }
    CATCH_AND_THROW
}

Local<Value> PathRename(Arguments const& args) {
    CHECK_ARGS_COUNT(args, 2);
    CHECK_ARG_TYPE(args[0], ValueKind::kString);
    CHECK_ARG_TYPE(args[1], ValueKind::kString);

    try {
        rename(
            ll::string_utils::str2wstr(args[0].asString().toString()),
            ll::string_utils::str2wstr(args[1].asString().toString())
        );
        return Boolean::newBoolean(true);
    }
    CATCH_AND_THROW
}

Local<Value> PathMove(Arguments const& args) {
    CHECK_ARGS_COUNT(args, 2);
    CHECK_ARG_TYPE(args[0], ValueKind::kString);
    CHECK_ARG_TYPE(args[1], ValueKind::kString);

    try {
        copy(
            ll::string_utils::str2wstr(args[0].asString().toString()),
            ll::string_utils::str2wstr(args[1].asString().toString())
        );
        remove_all(ll::string_utils::str2wstr(args[0].asString().toString()));
        return Boolean::newBoolean(true);
    }
    CATCH_AND_THROW
}

Local<Value> CheckIsDir(Arguments const& args) {
    CHECK_ARGS_COUNT(args, 1);
    CHECK_ARG_TYPE(args[0], ValueKind::kString);

    try {
        path p(ll::string_utils::str2wstr(args[0].asString().toString()));
        if (!exists(p)) return Boolean::newBoolean(false);

        return Boolean::newBoolean(directory_entry(p).is_directory());
    }
    CATCH_AND_THROW
}

Local<Value> GetFileSize(Arguments const& args) {
    CHECK_ARGS_COUNT(args, 1);
    CHECK_ARG_TYPE(args[0], ValueKind::kString);

    try {
        path p(ll::string_utils::str2wstr(args[0].asString().toString()));
        if (!exists(p)) return Number::newNumber(0);
        if (directory_entry(p).is_directory()) return Number::newNumber(0);

        auto sz = file_size(p);
        return Number::newNumber(static_cast<int64_t>(sz));
    }
    CATCH_AND_THROW
}

Local<Value> GetFilesList(Arguments const& args) {
    CHECK_ARGS_COUNT(args, 1);
    CHECK_ARG_TYPE(args[0], ValueKind::kString);

    try {
        std::filesystem::directory_entry directory(args[0].asString().toU8string());
        if (!directory.is_directory()) return {};

        Local<Array>                        arr = Array::newArray();
        std::filesystem::directory_iterator deps(directory);
        for (auto& i : deps) {
            arr.add(String::newString(ll::string_utils::u8str2str(i.path().filename().u8string())));
        }

        return arr;
    }
    CATCH_AND_THROW
}

Local<Value> FileReadFrom(Arguments const& args) {
    CHECK_ARGS_COUNT(args, 1);
    CHECK_ARG_TYPE(args[0], ValueKind::kString);

    try {
        auto content = ll::file_utils::readFile(args[0].asString().toU8string());
        if (!content) return {}; // Null
        return String::newString(content.value());
    }
    CATCH_AND_THROW
}

Local<Value> FileWriteTo(Arguments const& args) {
    CHECK_ARGS_COUNT(args, 2);
    CHECK_ARG_TYPE(args[0], ValueKind::kString);
    CHECK_ARG_TYPE(args[1], ValueKind::kString);

    try {
        std::filesystem::path path(args[0].asString().toU8string());
        if (!path.empty() && path.has_parent_path()) {
            std::error_code code;
            std::filesystem::create_directories(path.parent_path(), code);
            if (code) {
                throw CreateExceptionWithInfo(__FUNCTION__, "Fail to create directory " + path.parent_path().string() + "!");
            }
        } else {
            throw CreateExceptionWithInfo(__FUNCTION__, "Fail to create directory of " + args[0].asString().toString() + "!");
        }
        return Boolean::newBoolean(ll::file_utils::writeFile(path, args[1].asString().toString(), false));
    }
    CATCH_AND_THROW
}

Local<Value> FileWriteLine(Arguments const& args) {
    CHECK_ARGS_COUNT(args, 2);
    CHECK_ARG_TYPE(args[0], ValueKind::kString);
    CHECK_ARG_TYPE(args[1], ValueKind::kString);

    try {
        std::filesystem::path path(args[0].asString().toString());
        if (!path.empty() && path.has_parent_path()) {
            std::error_code code;
            std::filesystem::create_directories(path.parent_path(), code);
            if (code) {
                throw CreateExceptionWithInfo(__FUNCTION__, "Fail to create directory " + path.parent_path().string() + "!");
            }
        } else {
            throw CreateExceptionWithInfo(__FUNCTION__, "Fail to create directory of " + args[0].asString().toString() + "!");
        }

        std::ofstream fileWrite(path, std::ios::app);
        if (!fileWrite) return Boolean::newBoolean(false);
        fileWrite << args[1].asString().toString() << std::endl;
        return Boolean::newBoolean(fileWrite.good());
    }
    CATCH_AND_THROW
}

//////////////////// For Compatibility ////////////////////

Local<Value> OpenFile(Arguments const& args) {
    CHECK_ARGS_COUNT(args, 2);
    CHECK_ARG_TYPE(args[0], ValueKind::kString);
    CHECK_ARG_TYPE(args[1], ValueKind::kNumber);
    if (args.size() >= 3) CHECK_ARG_TYPE(args[2], ValueKind::kBoolean);

    try {
        std::filesystem::path path(args[0].asString().toU8string());
        if (!path.empty() && path.has_parent_path()) {
            std::error_code code;
            std::filesystem::create_directories(path.parent_path(), code);
            if (code) {
                throw CreateExceptionWithInfo(__FUNCTION__, "Fail to create directory " + path.parent_path().string() + "!");
            }
        } else {
            throw CreateExceptionWithInfo(__FUNCTION__, "Fail to create directory " + args[0].asString().toString() + "!");
        }

        FileOpenMode            fMode = static_cast<FileOpenMode>(args[1].asNumber().toInt32());
        std::ios_base::openmode mode  = std::ios_base::in;
        if (fMode == FileOpenMode::WriteMode) {
            std::fstream tmp(path, std::ios_base::app);
            tmp.flush();
            tmp.close();
            mode |= std::ios_base::out;
        } else if (fMode == FileOpenMode::AppendMode) mode |= std::ios_base::app;

        bool isBinary = false;
        if (args.size() >= 3 && args[2].asBoolean().value()) {
            isBinary  = true;
            mode     |= std::ios_base::binary;
        }

        std::fstream fs(path, mode);
        if (!fs.is_open()) {
            throw CreateExceptionWithInfo(__FUNCTION__, "Fail to Open File " + path.string() + "!");
        }
        return FileClass::newFile(std::move(fs), path.string(), isBinary);
    }
    CATCH_AND_THROW
}

Local<Object> FileClass::newFile(std::fstream&& f, std::string const& path, bool isBinary) {
    auto newp = new FileClass(std::move(f), path, isBinary);
    return newp->getScriptObject();
}
