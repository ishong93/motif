#pragma once

#include <motif/uil/Token.h>
#include <string>
#include <vector>

namespace motif::uil {

class Lexer {
public:
    explicit Lexer(const std::string& source, const std::string& filename = "<input>");

    Token nextToken();
    Token peekToken();
    const std::string& filename() const { return filename_; }
    int currentLine() const { return line_; }

    bool hasErrors() const { return !errors_.empty(); }
    const std::vector<std::string>& errors() const { return errors_; }

private:
    void skipWhitespace();
    void skipComment();
    Token readString();
    Token readNumber();
    Token readIdentifier();
    Token makeToken(TokenType type, const std::string& text);

    TokenType classifyKeyword(const std::string& word) const;

    std::string source_;
    std::string filename_;
    size_t pos_ = 0;
    int line_ = 1;
    int column_ = 1;
    bool caseSensitive_ = true;

    Token peeked_;
    bool hasPeeked_ = false;
    std::vector<std::string> errors_;
};

} // namespace motif::uil
