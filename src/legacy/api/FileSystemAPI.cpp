#include "api/FileSystemAPI.h"

#include "api/APIHelp.h"
#include "engine/EngineManager.h"
#include "engine/LocalShareData.h"
#include "engine/TimeTaskSystem.h"
#include "ll/api/service/GamingStatus.h"
#include "ll/api/utils/StringUtils.h"

#include <corecrt_io.h>
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

        .property("ReadMode", [] { return Number::newNumber((int)FileOpenMode::ReadMode); })
        .property("WriteMode", [] { return Number::newNumber((int)FileOpenMode::WriteMode); })
        .property("AppendMode", [] { return Number::newNumber((int)FileOpenMode::AppendMode); })

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
FileClass::FileClass(const Local<Object>& scriptObj, std::fstream&& f, const std::string& path, bool isBinary)
: ScriptClass(scriptObj) {
    this->file     = std::move(f);
    this->path     = path;
    this->isBinary = isBinary;
}

FileClass::FileClass(std::fstream&& f, const std::string& path, bool isBinary)
: ScriptClass(ScriptClass::ConstructFromCpp<FileClass>{}) {
    this->file     = std::move(f);
    this->path     = path;
    this->isBinary = isBinary;
}

FileClass* FileClass::constructor(const Arguments& args) {
    CHECK_ARGS_COUNT_C(args, 2);
    CHECK_ARG_TYPE_C(args[0], ValueKind::kString);
    CHECK_ARG_TYPE_C(args[1], ValueKind::kNumber);
    if (args.size() >= 3) CHECK_ARG_TYPE_C(args[2], ValueKind::kBoolean);

    try {
        std::filesystem::path path(args[0].asString().toU8string());
        if (!path.empty() && path.has_parent_path()) {
            std::filesystem::create_directories(path.parent_path());
        } else {
            LOG_ERROR_WITH_SCRIPT_INFO(
                __FUNCTION__,
                "Fail to create directory of " + args[0].asString().toString() + "!\n"
            );
            return nullptr;
        }
        FileOpenMode fMode = (FileOpenMode)(args[1].asNumber().toInt32());
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
            LOG_ERROR_WITH_SCRIPT_INFO(__FUNCTION__, "Fail to Open File " + path.string() + "!\n");
            return nullptr;
        }
        return new FileClass(args.thiz(), std::move(fs), path.string(), isBinary);
    } catch (const filesystem_error&) {
        LOG_ERROR_WITH_SCRIPT_INFO(__FUNCTION__, "Fail to Open File " + args[0].asString().toString() + "!\n");
        return nullptr;
    }
    CATCH_C("Fail in OpenFile!");
}

// 成员函数
Local<Value> FileClass::getPath() {
    try {
        return String::newString(path);
    }
    CATCH("Fail in getPath!");
}

Local<Value> FileClass::getAbsolutePath() {
    try {
        return String::newString(canonical(std::filesystem::path(ll::string_utils::str2wstr(path))).u8string());
    }
    CATCH("Fail in getAbsolutePath!");
}

Local<Value> FileClass::getSize() {
    try {
        size_t cur = file.tellg();
        file.seekg(0, file.end);
        size_t size = file.tellg();
        file.seekg(cur, file.beg);

        return Number::newNumber((long long)size);
    }
    CATCH("Fail in getPath!");
}

Local<Value> FileClass::readSync(const Arguments& args) {
    CHECK_ARGS_COUNT(args, 1);
    CHECK_ARG_TYPE(args[0], ValueKind::kNumber);

    try {
        int   cnt = args[0].asNumber().toInt32();
        char* buf = new char[cnt];
        file.read(buf, cnt);
        size_t bytes = file.gcount();

        Local<Value> res = isBinary ? ByteBuffer::newByteBuffer(buf, bytes).asValue()
                                    : String::newString(std::string_view(buf, bytes)).asValue();
        delete[] buf;
        return res;
    }
    CATCH("Fail in readSync!");
}

Local<Value> FileClass::readLineSync(const Arguments&) {
    try {
        std::string buf;
        getline(file, buf);
        return String::newString(buf);
    }
    CATCH("Fail in readLineSync!");
}

Local<Value> FileClass::readAllSync(const Arguments&) {
    try {
        std::string res((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
        return isBinary ? ByteBuffer::newByteBuffer(res.data(), res.size()).asValue()
                        : String::newString(res).asValue();
    }
    CATCH("Fail in readAllSync!");
}

Local<Value> FileClass::writeSync(const Arguments& args) {
    CHECK_ARGS_COUNT(args, 1);

    try {
        if (args[0].isString()) {
            file << args[0].asString().toString();
        } else if (args[0].isByteBuffer()) {
            file.write((char*)args[0].asByteBuffer().getRawBytes(), args[0].asByteBuffer().byteLength());
        } else {
            LOG_WRONG_ARG_TYPE(__FUNCTION__);
            return {};
        }
        return Boolean::newBoolean(!file.fail() && !file.bad());
    }
    CATCH("Fail in writeSync!");
}

Local<Value> FileClass::writeLineSync(const Arguments& args) {
    CHECK_ARGS_COUNT(args, 1);
    CHECK_ARG_TYPE(args[0], ValueKind::kString);

    try {
        file << args[0].asString().toString() << "\n";
        return Boolean::newBoolean(!file.fail() && !file.bad());
    }
    CATCH("Fail in writeLineSync!");
}

Local<Value> FileClass::read(const Arguments& args) {
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

                char* buf = new char[cnt];
                lock->lock();
                fp->read(buf, cnt);
                size_t bytes = fp->gcount();
                lock->unlock();

                EngineScope scope(engine);
                try {
                    Local<Value> res = isBinary ? ByteBuffer::newByteBuffer(buf, bytes).asValue()
                                                : String::newString(std::string_view(buf, bytes)).asValue();
                    delete[] buf;
                    // dangerous
                    NewTimeout(callback.get(), {res}, 1);
                }
                CATCH_IN_CALLBACK("ReadFile")
            }
        );
        return Boolean::newBoolean(true);
    }
    CATCH("Fail in read!");
}

Local<Value> FileClass::readLine(const Arguments& args) {
    CHECK_ARGS_COUNT(args, 1);
    CHECK_ARG_TYPE(args[0], ValueKind::kFunction);

    try {
        script::Global<Function> callbackFunc{args[0].asFunction()};

        pool.execute([fp{&file}, lock{&lock}, callback{std::move(callbackFunc)}, engine{EngineScope::currentEngine()}](
                     ) {
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
        });
        return Boolean::newBoolean(true);
    }
    CATCH("Fail in readLine!");
}

Local<Value> FileClass::readAll(const Arguments& args) {
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
    CATCH("Fail in readAll!");
}

Local<Value> FileClass::write(const Arguments& args) {
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
                std::string((char*)args[0].asByteBuffer().getRawBytes(), args[0].asByteBuffer().byteLength())
            );
        } else {
            LOG_WRONG_ARG_TYPE(__FUNCTION__);
            return {};
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
    CATCH("Fail in write!");
}

Local<Value> FileClass::writeLine(const Arguments& args) {
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
    CATCH("Fail in writeLine!");
}

Local<Value> FileClass::seekTo(const Arguments& args) {
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
    CATCH("Fail in seekTo!");
}

Local<Value> FileClass::setSize(const Arguments& args) {
    CHECK_ARGS_COUNT(args, 1);
    CHECK_ARG_TYPE(args[0], ValueKind::kNumber);

    try {
        return Boolean::newBoolean(false);
    }
    CATCH("Fail in setSize!");
}

Local<Value> FileClass::close(const Arguments&) {
    try {
        file.close();
        return Boolean::newBoolean(true);
    }
    CATCH("Fail in flush!");
}

Local<Value> FileClass::isEOF(const Arguments&) {
    try {
        return Boolean::newBoolean(file.eof());
    }
    CATCH("Fail in isEOF!");
}

Local<Value> FileClass::flush(const Arguments&) {
    try {
        pool.execute([fp{&file}, lock{&lock}]() {
            lock->lock();
            fp->flush();
            lock->unlock();
        });
        return Boolean::newBoolean(true);
    }
    CATCH("Fail in flush!");
}

Local<Value> FileClass::errorCode(const Arguments&) {
    try {
        file.flush();
        return Number::newNumber(errno);
    }
    CATCH("Fail in flush!");
}

Local<Value> FileClass::clear(const Arguments&) {
    try {
        file.clear();
        return Boolean::newBoolean(true);
    }
    CATCH("Fail in flush!");
}

//////////////////// APIs ////////////////////

Local<Value> DirCreate(const Arguments& args) {
    CHECK_ARGS_COUNT(args, 1);
    CHECK_ARG_TYPE(args[0], ValueKind::kString);

    try {
        return Boolean::newBoolean(std::filesystem::create_directories(args[0].asString().toU8string()));
    } catch (const filesystem_error&) {
        LOG_ERROR_WITH_SCRIPT_INFO(__FUNCTION__, "Fail to Create Dir " + args[0].asString().toString() + "!\n");
        return Boolean::newBoolean(false);
    }
    CATCH("Fail in CreateDir!");
}

Local<Value> PathDelete(const Arguments& args) {
    CHECK_ARGS_COUNT(args, 1);
    CHECK_ARG_TYPE(args[0], ValueKind::kString);

    try {
        return Boolean::newBoolean(
            std::filesystem::remove_all(ll::string_utils::str2wstr(args[0].asString().toString())) > 0
        );
    } catch (const filesystem_error&) {
        LOG_ERROR_WITH_SCRIPT_INFO(__FUNCTION__, "Fail to Delete " + args[0].asString().toString() + "!\n");
        return Boolean::newBoolean(false);
    }
    CATCH("Fail in DeletePath!");
}

Local<Value> PathExists(const Arguments& args) {
    CHECK_ARGS_COUNT(args, 1);
    CHECK_ARG_TYPE(args[0], ValueKind::kString);

    try {
        return Boolean::newBoolean(std::filesystem::exists(ll::string_utils::str2wstr(args[0].asString().toString())));
    } catch (const filesystem_error&) {
        LOG_ERROR_WITH_SCRIPT_INFO(__FUNCTION__, "Fail to Check " + args[0].asString().toString() + "!\n");
        return Boolean::newBoolean(false);
    }
    CATCH("Fail in ExistsPath!");
}

Local<Value> PathCopy(const Arguments& args) {
    CHECK_ARGS_COUNT(args, 2);
    CHECK_ARG_TYPE(args[0], ValueKind::kString);
    CHECK_ARG_TYPE(args[1], ValueKind::kString);

    try {
        copy(
            ll::string_utils::str2wstr(args[0].asString().toString()),
            ll::string_utils::str2wstr(args[1].asString().toString())
        );
        return Boolean::newBoolean(true);
    } catch (const filesystem_error&) {
        LOG_ERROR_WITH_SCRIPT_INFO(__FUNCTION__, "Fail to Copy " + args[0].asString().toString() + "!\n");
        return Boolean::newBoolean(false);
    }
    CATCH("Fail in CopyPath!");
}

Local<Value> PathRename(const Arguments& args) {
    CHECK_ARGS_COUNT(args, 2);
    CHECK_ARG_TYPE(args[0], ValueKind::kString);
    CHECK_ARG_TYPE(args[1], ValueKind::kString);

    try {
        rename(
            ll::string_utils::str2wstr(args[0].asString().toString()),
            ll::string_utils::str2wstr(args[1].asString().toString())
        );
        return Boolean::newBoolean(true);
    } catch (const filesystem_error&) {
        LOG_ERROR_WITH_SCRIPT_INFO(__FUNCTION__, "Fail to Rename " + args[0].asString().toString() + "!\n");
        return Boolean::newBoolean(false);
    }
    CATCH("Fail in RenamePath!");
}

Local<Value> PathMove(const Arguments& args) {
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
    } catch (const filesystem_error&) {
        LOG_ERROR_WITH_SCRIPT_INFO(__FUNCTION__, "Fail to Move " + args[0].asString().toString() + "!\n");
        return Boolean::newBoolean(false);
    }
    CATCH("Fail in MovePath!");
}

Local<Value> CheckIsDir(const Arguments& args) {
    CHECK_ARGS_COUNT(args, 1);
    CHECK_ARG_TYPE(args[0], ValueKind::kString);

    try {
        path p(ll::string_utils::str2wstr(args[0].asString().toString()));
        if (!exists(p)) return Boolean::newBoolean(false);

        return Boolean::newBoolean(directory_entry(p).is_directory());
    } catch (const filesystem_error&) {
        LOG_ERROR_WITH_SCRIPT_INFO(__FUNCTION__, "Fail to Get Type of " + args[0].asString().toString() + "!\n");
        return {};
    }
    CATCH("Fail in CheckIsDir!");
}

Local<Value> GetFileSize(const Arguments& args) {
    CHECK_ARGS_COUNT(args, 1);
    CHECK_ARG_TYPE(args[0], ValueKind::kString);

    try {
        path p(ll::string_utils::str2wstr(args[0].asString().toString()));
        if (!exists(p)) return Number::newNumber(0);
        if (directory_entry(p).is_directory()) return Number::newNumber(0);

        auto sz = file_size(p);
        return Number::newNumber((int64_t)sz);
    } catch (const filesystem_error&) {
        LOG_ERROR_WITH_SCRIPT_INFO(__FUNCTION__, "Fail to Get Size of " + args[0].asString().toString() + "!\n");
        return {};
    }
    CATCH("Fail in GetFileSize!");
}

Local<Value> GetFilesList(const Arguments& args) {
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
    CATCH("Fail in GetFilesList!");
}

Local<Value> FileReadFrom(const Arguments& args) {
    CHECK_ARGS_COUNT(args, 1);
    CHECK_ARG_TYPE(args[0], ValueKind::kString);

    try {
        auto content = ll::file_utils::readFile(args[0].asString().toU8string());
        if (!content) return {}; // Null
        return String::newString(content.value());
    }
    CATCH("Fail in FileReadAll!");
}

Local<Value> FileWriteTo(const Arguments& args) {
    CHECK_ARGS_COUNT(args, 2);
    CHECK_ARG_TYPE(args[0], ValueKind::kString);
    CHECK_ARG_TYPE(args[1], ValueKind::kString);

    try {
        std::filesystem::path path(args[0].asString().toU8string());
        if (!path.empty() && path.has_parent_path()) {
            std::error_code code;
            std::filesystem::create_directories(path.parent_path(), code);
            if (code) {
                LOG_ERROR_WITH_SCRIPT_INFO(
                    __FUNCTION__,
                    "Fail to create directory " + path.parent_path().string() + "!\n"
                );
                return Boolean::newBoolean(false);
            }
        } else {
            LOG_ERROR_WITH_SCRIPT_INFO(
                __FUNCTION__,
                "Fail to create directory of " + args[0].asString().toString() + "!\n"
            );
            return Boolean::newBoolean(false);
        }
        return Boolean::newBoolean(ll::file_utils::writeFile(path, args[1].asString().toString(), false));
    }
    CATCH("Fail in FileWriteAll!");
}

Local<Value> FileWriteLine(const Arguments& args) {
    CHECK_ARGS_COUNT(args, 2);
    CHECK_ARG_TYPE(args[0], ValueKind::kString);
    CHECK_ARG_TYPE(args[1], ValueKind::kString);

    try {
        std::filesystem::path path(args[0].asString().toString());
        if (!path.empty() && path.has_parent_path()) {
            std::error_code code;
            std::filesystem::create_directories(path.parent_path(), code);
            if (code) {
                LOG_ERROR_WITH_SCRIPT_INFO(
                    __FUNCTION__,
                    "Fail to create directory " + path.parent_path().string() + "!\n"
                );
                return Boolean::newBoolean(false);
            }
        } else {
            LOG_ERROR_WITH_SCRIPT_INFO(
                __FUNCTION__,
                "Fail to create directory of " + args[0].asString().toString() + "!\n"
            );
            return Boolean::newBoolean(false);
        }

        std::ofstream fileWrite(path, std::ios::app);
        if (!fileWrite) return Boolean::newBoolean(false);
        fileWrite << args[1].asString().toString() << std::endl;
        return Boolean::newBoolean(fileWrite.good());
    }
    CATCH("Fail in FileWriteLine!");
}

//////////////////// For Compatibility ////////////////////

Local<Value> OpenFile(const Arguments& args) {
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
                LOG_ERROR_WITH_SCRIPT_INFO(
                    __FUNCTION__,
                    "Fail to create directory " + path.parent_path().string() + "!\n"
                );
                return Boolean::newBoolean(false);
            }
        } else {
            LOG_ERROR_WITH_SCRIPT_INFO(
                __FUNCTION__,
                "Fail to create directory " + args[0].asString().toString() + "!\n"
            );
            return {};
        }

        FileOpenMode            fMode = (FileOpenMode)(args[1].asNumber().toInt32());
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
            LOG_ERROR_WITH_SCRIPT_INFO(__FUNCTION__, "Fail to Open File " + path.string() + "!\n");
            return {};
        }
        return FileClass::newFile(std::move(fs), path.string(), isBinary);
    } catch (const filesystem_error&) {
        LOG_ERROR_WITH_SCRIPT_INFO(__FUNCTION__, "Fail to Open File " + args[0].asString().toString() + "!\n");
        return {};
    }
    CATCH("Fail in OpenFile!");
}

Local<Object> FileClass::newFile(std::fstream&& f, const std::string& path, bool isBinary) {
    auto newp = new FileClass(std::move(f), path, isBinary);
    return newp->getScriptObject();
}
