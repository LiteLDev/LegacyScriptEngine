#include "legacy/utils/ScriptErrorPrinter.h"

#include "legacy/engine/EngineOwnData.h"
#include "ll/api/utils/ErrorUtils.h"

#include <fmt/format.h>

#include <algorithm>
#include <cctype>
#include <exception>
#include <filesystem>
#include <fstream>
#include <optional>
#include <regex>
#include <sstream>
#include <string>
#include <vector>

namespace legacy::script_error {
namespace {

struct ScriptFrame {
    std::string file;
    size_t      line   = 0;
    size_t      column = 0;
};

bool isInternalFrame(std::string const& file) {
    if (file.empty()) return true;
    if (file == "<anonymous>" || file == "[native code]") return true;
    if (file.starts_with("native ") || file.starts_with("internal/")) return true;
    return false;
}

std::optional<ScriptFrame> parseLine(std::string const& line) {
    static std::regex const pythonPattern{R"re(File "([^"]+)", line ([0-9]+))re"};
    static std::regex const v8Pattern{R"((?:at\s+(?:.+\s+\()?)([^()\s]+):([0-9]+):([0-9]+)\)?)"};
    static std::regex const luaPattern{R"re(^\s*(?:\[string "([^"]+)"\]|([^:\s]+)):([0-9]+):)re"};
    static std::regex const qjsPattern{R"(([^@\s()]+):([0-9]+):([0-9]+))"};

    std::smatch match;
    if (std::regex_search(line, match, pythonPattern)) {
        return ScriptFrame{match[1].str(), static_cast<size_t>(std::stoull(match[2].str())), 0};
    }
    if (std::regex_search(line, match, v8Pattern)) {
        return ScriptFrame{
            match[1].str(),
            static_cast<size_t>(std::stoull(match[2].str())),
            static_cast<size_t>(std::stoull(match[3].str()))
        };
    }
    if (std::regex_search(line, match, qjsPattern)) {
        return ScriptFrame{
            match[1].str(),
            static_cast<size_t>(std::stoull(match[2].str())),
            static_cast<size_t>(std::stoull(match[3].str()))
        };
    }
    if (std::regex_search(line, match, luaPattern)) {
        auto file = match[1].matched ? match[1].str() : match[2].str();
        return ScriptFrame{file, static_cast<size_t>(std::stoull(match[3].str())), 0};
    }
    return std::nullopt;
}

std::vector<ScriptFrame> parseFrames(std::string const& stacktrace) {
    std::vector<ScriptFrame> frames;
    std::istringstream       input(stacktrace);
    std::string              line;
    while (std::getline(input, line)) {
        auto frame = parseLine(line);
        if (!frame || frame->line == 0 || isInternalFrame(frame->file)) continue;
        frames.emplace_back(std::move(*frame));
    }
    return frames;
}

std::filesystem::path pluginRoot() {
    try {
        auto data = getEngineOwnData();
        if (data && data->plugin) return data->plugin->getModDir();
    } catch (...) {
    }
    return {};
}

std::optional<std::filesystem::path> findSourceFile(std::string const& rawFile) {
    if (rawFile.empty()) return std::nullopt;

    std::filesystem::path filePath{rawFile};
    std::error_code       ec;
    if (filePath.is_absolute() && std::filesystem::is_regular_file(filePath, ec)) {
        return filePath;
    }

    auto root = pluginRoot();
    if (root.empty()) return std::nullopt;

    auto direct = root / filePath;
    if (std::filesystem::is_regular_file(direct, ec)) return direct;

    auto filename = filePath.filename();
    if (filename.empty()) return std::nullopt;

    size_t visited = 0;
    for (std::filesystem::recursive_directory_iterator it{
             root,
             std::filesystem::directory_options::skip_permission_denied,
             ec
         },
         end;
         !ec && it != end && visited < 5000;
         it.increment(ec), ++visited) {
        if (!it->is_regular_file(ec)) continue;
        if (it->path().filename() == filename) return it->path();
    }
    return std::nullopt;
}

std::optional<std::string> readLine(std::filesystem::path const& path, size_t wantedLine) {
    if (wantedLine == 0) return std::nullopt;
    std::ifstream input(path);
    if (!input) return std::nullopt;

    std::string line;
    for (size_t current = 1; std::getline(input, line); ++current) {
        if (current == wantedLine) return line;
    }
    return std::nullopt;
}

bool isIdentifierChar(char ch) {
    return std::isalnum(static_cast<unsigned char>(ch)) || ch == '_' || ch == '$';
}

std::optional<ScriptFrame>
findIdentifierInSource(std::filesystem::path const& path, std::string const& identifier, size_t preferredStartLine) {
    if (identifier.empty()) return std::nullopt;

    std::ifstream input(path);
    if (!input) return std::nullopt;

    std::optional<ScriptFrame> firstMatch;
    std::string                line;
    for (size_t current = 1; std::getline(input, line); ++current) {
        size_t pos = 0;
        while ((pos = line.find(identifier, pos)) != std::string::npos) {
            auto beforeOk = pos == 0 || !isIdentifierChar(line[pos - 1]);
            auto afterPos = pos + identifier.size();
            auto afterOk  = afterPos >= line.size() || !isIdentifierChar(line[afterPos]);
            if (beforeOk && afterOk) {
                ScriptFrame frame{"", current, pos + 1};
                if (current >= preferredStartLine) return frame;
                if (!firstMatch) firstMatch = frame;
                break;
            }
            ++pos;
        }
    }
    return firstMatch;
}

bool isLikelyScriptFile(std::filesystem::path const& path) {
    auto ext = path.extension().string();
    std::ranges::transform(ext, ext.begin(), [](unsigned char ch) { return static_cast<char>(std::tolower(ch)); });
    return ext == ".js" || ext == ".mjs" || ext == ".cjs" || ext == ".lua" || ext == ".py";
}

bool isIgnoredSourceDir(std::filesystem::path const& path) {
    for (auto const& part : path) {
        auto name = part.string();
        if (name == "node_modules" || name == ".git" || name == "__pycache__") return true;
    }
    return false;
}

std::optional<ScriptFrame> findIdentifierInPluginSource(std::string const& identifier) {
    auto root = pluginRoot();
    if (root.empty() || identifier.empty()) return std::nullopt;

    std::error_code ec;
    size_t          visited = 0;
    for (std::filesystem::recursive_directory_iterator it{
             root,
             std::filesystem::directory_options::skip_permission_denied,
             ec
         },
         end;
         !ec && it != end && visited < 5000;
         it.increment(ec), ++visited) {
        auto const& path = it->path();
        if (it->is_directory(ec) && isIgnoredSourceDir(std::filesystem::relative(path, root, ec))) {
            it.disable_recursion_pending();
            continue;
        }
        if (!it->is_regular_file(ec) || !isLikelyScriptFile(path)) continue;

        auto frame = findIdentifierInSource(path, identifier, 1);
        if (!frame) continue;

        frame->file = path.string();
        return frame;
    }
    return std::nullopt;
}

std::optional<std::string> undefinedIdentifierFromMessage(std::string const& message) {
    static std::regex const patterns[]{
        std::regex{R"((?:ReferenceError:\s*)?([A-Za-z_$][A-Za-z0-9_$]*) is not defined)"},
        std::regex{R"(Can't find variable:\s*([A-Za-z_$][A-Za-z0-9_$]*))"}
    };

    std::smatch match;
    for (auto const& pattern : patterns) {
        if (std::regex_search(message, match, pattern)) {
            return match[1].str();
        }
    }
    return std::nullopt;
}

void refineFrameFromMessage(ScriptFrame& frame, std::filesystem::path const& source, std::string const& message) {
    auto identifier = undefinedIdentifierFromMessage(message);
    if (!identifier) return;

    auto better = findIdentifierInSource(source, *identifier, frame.line);
    if (!better) return;

    frame.line   = better->line;
    frame.column = better->column;
}

std::string displayPath(std::filesystem::path const& path) {
    auto root = pluginRoot();
    std::error_code ec;
    if (!root.empty()) {
        auto relative = std::filesystem::relative(path, root, ec);
        if (!ec && !relative.empty()) return relative.generic_string();
    }
    return path.generic_string();
}

void printFrame(ScriptFrame frame, ll::io::Logger& logger, std::string const& message) {
    auto source = findSourceFile(frame.file);
    if (!source) {
        logger.error(
            "Script error location: {}:{}{} (source file not found)",
            frame.file,
            frame.line,
            frame.column ? fmt::format(":{}", frame.column) : ""
        );
        return;
    }

    refineFrameFromMessage(frame, *source, message);

    logger.error(
        "Script error location: {}:{}{}",
        displayPath(*source),
        frame.line,
        frame.column ? fmt::format(":{}", frame.column) : ""
    );

    auto before = readLine(*source, frame.line > 1 ? frame.line - 1 : 0);
    auto exact  = readLine(*source, frame.line);
    auto after  = readLine(*source, frame.line + 1);

    if (before) logger.error("{:>6} | {}", frame.line - 1, *before);
    if (exact) {
        logger.error("> {:>4} | {}", frame.line, *exact);
        if (frame.column > 0) {
            auto caretColumn = std::max<size_t>(frame.column, 1);
            logger.error("       | {}^", std::string(caretColumn - 1, ' '));
        }
    }
    if (after) logger.error("{:>6} | {}", frame.line + 1, *after);
}

void printScriptLocation(std::string const& stacktrace, std::string const& message, ll::io::Logger& logger) {
    auto frames = parseFrames(stacktrace);
    if (frames.empty()) {
        auto identifier = undefinedIdentifierFromMessage(message);
        if (!identifier) return;

        auto frame = findIdentifierInPluginSource(*identifier);
        if (!frame) return;

        printFrame(*frame, logger, message);
        return;
    }
    printFrame(frames.front(), logger, message);
}

void printScriptLocation(script::Exception const& exception, ll::io::Logger& logger) {
    auto stacktrace = exception.stacktrace();
    auto message    = exception.message();
    printScriptLocation(stacktrace.empty() ? message : stacktrace, message, logger);
}

} // namespace

void printException(script::Exception const& exception, ll::io::Logger& logger) {
    printScriptLocation(exception, logger);
    ll::error_utils::printException(exception, logger);
}

void printRawError(std::string const& error, ll::io::Logger& logger) { printScriptLocation(error, error, logger); }

void printCurrentException(ll::io::Logger& logger) {
    auto current = std::current_exception();
    if (!current) return;

    try {
        std::rethrow_exception(current);
    } catch (script::Exception const& exception) {
        printException(exception, logger);
    } catch (...) {
        ll::error_utils::printCurrentException(logger);
    }
}

} // namespace legacy::script_error
