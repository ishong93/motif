#pragma once

#include <string>
#include <cstdint>

namespace motif::uil {

enum class TokenType {
    // Literals
    Identifier,
    StringLiteral,
    IntegerLiteral,
    FloatLiteral,
    BooleanLiteral,

    // Keywords
    Module,
    EndModule,         // "end module"
    Procedure,
    Object,
    Value,
    List,
    Include,
    Arguments,
    Callbacks,
    Controls,
    Identifier_kw,     // "identifier" keyword (for exported identifiers)

    // Types
    Integer,
    Boolean,
    Float,
    String,
    CompoundString,
    CompoundStringComponent,
    FontTable,
    Font,
    Color,
    Pixmap,
    Icon,
    Reason,
    Argument,
    TranslationTable,
    Keysym,
    CharSet,
    WideCharacter,

    // Built-in functions
    CompoundStringFunc,  // compound_string(...)
    AsciiStringTable,
    FontTableFunc,

    // Operators and punctuation
    Equals,       // =
    Semicolon,    // ;
    Colon,        // :
    Comma,        // ,
    LeftBrace,    // {
    RightBrace,   // }
    LeftParen,    // (
    RightParen,   // )
    Ampersand,    // & (string concatenation)
    Plus,         // +
    Minus,        // -
    Multiply,     // *
    Divide,       // /
    Tilde,        // ~ (not)
    Pound,        // #

    // Directives
    Names,
    CaseSensitive,
    CaseInsensitive,
    Version,
    CharacterSet,
    Managed,
    Unmanaged,
    Private,
    Exported,
    Imported,

    // Special
    Separate,     // separate= in compound_string
    True,
    False,
    On,
    Off,

    // Meta
    Eof,
    Error,
};

struct Token {
    TokenType type = TokenType::Error;
    std::string text;
    int line = 0;
    int column = 0;

    bool isKeyword() const;
    bool isLiteral() const;
    bool isOperator() const;
};

} // namespace motif::uil
