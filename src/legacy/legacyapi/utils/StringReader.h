#pragma once
#include <string>

namespace lse::legacy {
class StringReader {

    const std::string           str;
    size_t                      length = 0;
    std::string::const_iterator begin;
    std::string::const_iterator end;
    std::string::const_iterator it;

public:
    StringReader(const std::string& str);
    StringReader(const char* str);
    StringReader(const char* str, size_t len);
    StringReader(const StringReader& other)            = default;
    StringReader(StringReader&& other)                 = default;
    StringReader& operator=(const StringReader& other) = default;
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
    std::string readUntil(const std::string& chars);
    std::string readUntilNot(const std::string& chars);
    std::string readUntilNot(char c);
    std::string readLine();
    std::string readLetters(const std::string& chars = "");
    std::string readLower(const std::string& chars = "");
    std::string readUpper(const std::string& chars = "");
    std::string readDigits(const std::string& chars = "");
    std::string readLettersAndDigits(const std::string& chars = "");
    std::string readVariableName();
    std::string readToEnd();
    char        peek();
    char        peek(char& c);
    char        peek(size_t offset);
    std::string peek(size_t offset, size_t len);
    void        skip();
    void        skip(size_t len);
    void        skipUntil(char c);
    void        skipUntil(const std::string& chars);
    void        skipUntilNot(char c);
    void        skipUntilNot(const std::string& chars);
    void        skipWhitespace();
    void        skipLine();
    void        skipLetters(const std::string& chars = "");
    void        skipLower(const std::string& chars = "");
    void        skipUpper(const std::string& chars = "");
    void        skipDigits(const std::string& chars = "");
    void        skipLettersAndDigits(const std::string& chars = "");

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
