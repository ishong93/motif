#include <motif/uil/Lexer.h>
#include <algorithm>
#include <cctype>
#include <unordered_map>

namespace motif::uil {

// ── Token helpers ────────────────────────────────────────────

bool Token::isKeyword() const {
    return type >= TokenType::Module && type <= TokenType::Off;
}

bool Token::isLiteral() const {
    return type >= TokenType::Identifier && type <= TokenType::BooleanLiteral;
}

bool Token::isOperator() const {
    return type >= TokenType::Equals && type <= TokenType::Pound;
}

// ── Lexer ────────────────────────────────────────────────────

Lexer::Lexer(const std::string& source, const std::string& filename)
    : source_(source), filename_(filename) {}

Token Lexer::makeToken(TokenType type, const std::string& text) {
    return Token{type, text, line_, column_};
}

void Lexer::skipWhitespace() {
    while (pos_ < source_.size()) {
        char c = source_[pos_];
        if (c == '\n') {
            ++line_;
            column_ = 1;
            ++pos_;
        } else if (c == '\r') {
            ++pos_;
            if (pos_ < source_.size() && source_[pos_] == '\n') ++pos_;
            ++line_;
            column_ = 1;
        } else if (std::isspace(static_cast<unsigned char>(c))) {
            ++pos_;
            ++column_;
        } else {
            break;
        }
    }
}

void Lexer::skipComment() {
    // UIL comments start with '!'
    while (pos_ < source_.size() && source_[pos_] != '\n') {
        ++pos_;
    }
}

Token Lexer::readString() {
    ++pos_; // skip opening quote
    ++column_;
    std::string result;

    while (pos_ < source_.size() && source_[pos_] != '\'') {
        if (source_[pos_] == '\\' && pos_ + 1 < source_.size()) {
            ++pos_;
            ++column_;
            switch (source_[pos_]) {
                case 'n': result += '\n'; break;
                case 't': result += '\t'; break;
                case '\\': result += '\\'; break;
                case '\'': result += '\''; break;
                default: result += source_[pos_]; break;
            }
        } else if (source_[pos_] == '\n') {
            result += '\n';
            ++line_;
            column_ = 0;
        } else {
            result += source_[pos_];
        }
        ++pos_;
        ++column_;
    }

    if (pos_ < source_.size()) {
        ++pos_; // skip closing quote
        ++column_;
    } else {
        errors_.push_back(filename_ + ":" + std::to_string(line_) + ": unterminated string literal");
    }

    return makeToken(TokenType::StringLiteral, result);
}

Token Lexer::readNumber() {
    size_t start = pos_;
    bool isFloat = false;

    if (source_[pos_] == '-' || source_[pos_] == '+') {
        ++pos_;
        ++column_;
    }

    while (pos_ < source_.size() && std::isdigit(static_cast<unsigned char>(source_[pos_]))) {
        ++pos_;
        ++column_;
    }

    if (pos_ < source_.size() && source_[pos_] == '.') {
        isFloat = true;
        ++pos_;
        ++column_;
        while (pos_ < source_.size() && std::isdigit(static_cast<unsigned char>(source_[pos_]))) {
            ++pos_;
            ++column_;
        }
    }

    std::string text = source_.substr(start, pos_ - start);
    return makeToken(isFloat ? TokenType::FloatLiteral : TokenType::IntegerLiteral, text);
}

TokenType Lexer::classifyKeyword(const std::string& word) const {
    std::string lower = word;
    std::transform(lower.begin(), lower.end(), lower.begin(),
                   [](unsigned char c) { return std::tolower(c); });

    static const std::unordered_map<std::string, TokenType> keywords = {
        {"module", TokenType::Module},
        {"end", TokenType::EndModule},
        {"procedure", TokenType::Procedure},
        {"object", TokenType::Object},
        {"value", TokenType::Value},
        {"list", TokenType::List},
        {"include", TokenType::Include},
        {"arguments", TokenType::Arguments},
        {"callbacks", TokenType::Callbacks},
        {"controls", TokenType::Controls},
        {"identifier", TokenType::Identifier_kw},
        {"integer", TokenType::Integer},
        {"boolean", TokenType::Boolean},
        {"float", TokenType::Float},
        {"string", TokenType::String},
        {"compound_string", TokenType::CompoundStringFunc},
        {"font_table", TokenType::FontTableFunc},
        {"font", TokenType::Font},
        {"color", TokenType::Color},
        {"pixmap", TokenType::Pixmap},
        {"icon", TokenType::Icon},
        {"reason", TokenType::Reason},
        {"argument", TokenType::Argument},
        {"translation_table", TokenType::TranslationTable},
        {"keysym", TokenType::Keysym},
        {"character_set", TokenType::CharacterSet},
        {"names", TokenType::Names},
        {"case_sensitive", TokenType::CaseSensitive},
        {"case_insensitive", TokenType::CaseInsensitive},
        {"version", TokenType::Version},
        {"managed", TokenType::Managed},
        {"unmanaged", TokenType::Unmanaged},
        {"private", TokenType::Private},
        {"exported", TokenType::Exported},
        {"imported", TokenType::Imported},
        {"separate", TokenType::Separate},
        {"true", TokenType::True},
        {"false", TokenType::False},
        {"on", TokenType::On},
        {"off", TokenType::Off},
    };

    auto it = keywords.find(lower);
    if (it != keywords.end()) return it->second;
    return TokenType::Identifier;
}

Token Lexer::readIdentifier() {
    size_t start = pos_;
    while (pos_ < source_.size() &&
           (std::isalnum(static_cast<unsigned char>(source_[pos_])) ||
            source_[pos_] == '_')) {
        ++pos_;
        ++column_;
    }

    std::string word = source_.substr(start, pos_ - start);
    TokenType type = classifyKeyword(word);

    if (type == TokenType::True || type == TokenType::False) {
        return makeToken(TokenType::BooleanLiteral, word);
    }

    return makeToken(type, word);
}

Token Lexer::nextToken() {
    if (hasPeeked_) {
        hasPeeked_ = false;
        return peeked_;
    }

    skipWhitespace();

    if (pos_ >= source_.size()) {
        return makeToken(TokenType::Eof, "");
    }

    char c = source_[pos_];

    if (c == '!') {
        skipComment();
        return nextToken();
    }

    if (c == '\'') {
        return readString();
    }

    if (std::isdigit(static_cast<unsigned char>(c)) ||
        (c == '-' && pos_ + 1 < source_.size() &&
         std::isdigit(static_cast<unsigned char>(source_[pos_ + 1])))) {
        return readNumber();
    }

    if (std::isalpha(static_cast<unsigned char>(c)) || c == '_') {
        return readIdentifier();
    }

    ++pos_;
    ++column_;
    switch (c) {
        case '=': return makeToken(TokenType::Equals, "=");
        case ';': return makeToken(TokenType::Semicolon, ";");
        case ':': return makeToken(TokenType::Colon, ":");
        case ',': return makeToken(TokenType::Comma, ",");
        case '{': return makeToken(TokenType::LeftBrace, "{");
        case '}': return makeToken(TokenType::RightBrace, "}");
        case '(': return makeToken(TokenType::LeftParen, "(");
        case ')': return makeToken(TokenType::RightParen, ")");
        case '&': return makeToken(TokenType::Ampersand, "&");
        case '+': return makeToken(TokenType::Plus, "+");
        case '*': return makeToken(TokenType::Multiply, "*");
        case '/': return makeToken(TokenType::Divide, "/");
        case '~': return makeToken(TokenType::Tilde, "~");
        case '#': return makeToken(TokenType::Pound, "#");
        default:
            errors_.push_back(filename_ + ":" + std::to_string(line_) +
                              ": unexpected character '" + std::string(1, c) + "'");
            return makeToken(TokenType::Error, std::string(1, c));
    }
}

Token Lexer::peekToken() {
    if (!hasPeeked_) {
        peeked_ = nextToken();
        hasPeeked_ = true;
    }
    return peeked_;
}

} // namespace motif::uil
