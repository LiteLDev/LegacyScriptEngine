#pragma once
#include <string>

namespace lse::legacy {
class StringReader {

    std::string                 str;
    size_t                      length = 0;
    std::string::const_iterator begin;
    std::string::const_iterator end;
    std::string::const_iterator it;

public:
    StringReader(std::string const& str);
    StringReader(char const* str);
    StringReader(char const* str, size_t len);
    StringReader(StringReader const& other)            = default;
    StringReader(StringReader&& other)                 = default;
    StringReader& operator=(StringReader const& other) = default;
    StringReader& operator=(StringReader&& other)      = default;

    bool   isEmpty() const;
    bool   isEnd() const;
    bool   isStart() const;
    bool   isValid() const;
    size_t getPos() const;
    size_t getLength() const;
    size_t getRemaining() const;

    char        read();
    char        read(char& c);
    std::string read(size_t len);
    std::string readUntil(char c);
    std::string readUntil(std::string const& chars);
    std::string readUntilNot(std::string const& chars);
    std::string readUntilNot(char c);
    std::string readLine();
    std::string readLetters(std::string const& chars = "");
    std::string readLower(std::string const& chars = "");
    std::string readUpper(std::string const& chars = "");
    std::string readDigits(std::string const& chars = "");
    std::string readLettersAndDigits(std::string const& chars = "");
    std::string readVariableName();
    std::string readToEnd();
    char        peek() const;
    char        peek(char& c) const;
    char        peek(size_t offset) const;
    std::string peek(size_t offset, size_t len) const;
    void        skip();
    void        skip(size_t len);
    void        skipUntil(char c);
    void        skipUntil(std::string const& chars);
    void        skipUntilNot(char c);
    void        skipUntilNot(std::string const& chars);
    void        skipWhitespace();
    void        skipLine();
    void        skipLetters(std::string const& chars = "");
    void        skipLower(std::string const& chars = "");
    void        skipUpper(std::string const& chars = "");
    void        skipDigits(std::string const& chars = "");
    void        skipLettersAndDigits(std::string const& chars = "");

    template <typename T>
    inline T readInteger() {
        T result = 0;
        while (isValid() && isdigit(read())) {
            result = result * 10 + (read() - '0');
        }
        return result;
    }
};
} // namespace lse::legacy
