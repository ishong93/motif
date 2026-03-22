#include <motif/uil/Parser.h>
#include <stdexcept>

namespace motif::uil {

Parser::Parser(Lexer& lexer) : lexer_(lexer) {
    current_ = lexer_.nextToken();
}

Token Parser::advance() {
    Token prev = current_;
    current_ = lexer_.nextToken();
    return prev;
}

Token Parser::peek() {
    return current_;
}

bool Parser::check(TokenType type) {
    return current_.type == type;
}

bool Parser::match(TokenType type) {
    if (check(type)) {
        advance();
        return true;
    }
    return false;
}

Token Parser::expect(TokenType type, const std::string& context) {
    if (check(type)) {
        return advance();
    }
    error("expected " + context + " but got '" + current_.text + "'");
    return current_;
}

void Parser::error(const std::string& msg) {
    errors_.push_back(lexer_.filename() + ":" + std::to_string(current_.line) + ": " + msg);
}

void Parser::synchronize() {
    // Skip tokens until we find a likely statement boundary
    while (!check(TokenType::Eof)) {
        if (check(TokenType::Semicolon)) {
            advance();
            return;
        }
        if (check(TokenType::Object) || check(TokenType::Procedure) ||
            check(TokenType::Value) || check(TokenType::EndModule)) {
            return;
        }
        advance();
    }
}

// ── Module ───────────────────────────────────────────────────

std::unique_ptr<ModuleNode> Parser::parseModule() {
    auto mod = std::make_unique<ModuleNode>();

    parseModuleHeader(*mod);
    parseModuleBody(*mod);

    // Expect "end module ;"
    if (check(TokenType::EndModule)) {
        advance(); // "end"
        if (check(TokenType::Module)) {
            advance(); // "module"
        }
        match(TokenType::Semicolon);
    }

    return mod;
}

void Parser::parseModuleHeader(ModuleNode& mod) {
    expect(TokenType::Module, "'module'");

    if (check(TokenType::Identifier)) {
        mod.name = advance().text;
    }

    // Parse optional module clauses
    while (!check(TokenType::Eof) && !check(TokenType::Procedure) &&
           !check(TokenType::Object) && !check(TokenType::Value) &&
           !check(TokenType::EndModule) && !check(TokenType::Include)) {

        if (check(TokenType::Version)) {
            advance();
            expect(TokenType::Equals, "'='");
            if (check(TokenType::StringLiteral)) {
                mod.version = advance().text;
            }
        } else if (check(TokenType::Names)) {
            advance();
            expect(TokenType::Equals, "'='");
            if (check(TokenType::CaseSensitive)) {
                advance();
                mod.caseSensitive = true;
            } else if (check(TokenType::CaseInsensitive)) {
                advance();
                mod.caseSensitive = false;
            }
        } else if (check(TokenType::CharacterSet)) {
            advance();
            expect(TokenType::Equals, "'='");
            advance(); // skip charset name
        } else {
            // Unknown clause, skip
            advance();
        }
    }
}

void Parser::parseModuleBody(ModuleNode& mod) {
    while (!check(TokenType::Eof) && !check(TokenType::EndModule)) {
        if (check(TokenType::Procedure)) {
            parseProcedureSection(mod);
        } else if (check(TokenType::Object)) {
            parseObjectDecl(mod);
        } else if (check(TokenType::Value)) {
            parseValueSection(mod);
        } else if (check(TokenType::Include)) {
            parseIncludeDirective(mod);
        } else {
            error("unexpected token '" + current_.text + "' in module body");
            synchronize();
        }
    }
}

// ── Procedure declarations ───────────────────────────────────

void Parser::parseProcedureSection(ModuleNode& mod) {
    advance(); // consume 'procedure'

    while (!check(TokenType::Eof) && !check(TokenType::Object) &&
           !check(TokenType::Value) && !check(TokenType::EndModule) &&
           !check(TokenType::Procedure) && !check(TokenType::Include)) {

        if (!check(TokenType::Identifier)) break;

        ProcedureDecl proc;
        proc.line = current_.line;
        proc.name = advance().text;

        if (match(TokenType::LeftParen)) {
            // Optional argument type
            if (!check(TokenType::RightParen)) {
                proc.hasArgument = true;
                advance(); // skip type name
            }
            expect(TokenType::RightParen, "')'");
        }

        expect(TokenType::Semicolon, "';'");

        symbols_.define(proc.name, SymbolKind::Procedure, {}, false, proc.line);
        mod.procedures.push_back(std::move(proc));
    }
}

// ── Object declarations ──────────────────────────────────────

void Parser::parseObjectDecl(ModuleNode& mod) {
    advance(); // consume 'object'

    ObjectNode obj;
    obj.line = current_.line;

    // name : WidgetClass { ... };
    if (check(TokenType::Identifier)) {
        obj.name = advance().text;
    }

    expect(TokenType::Colon, "':'");

    // Optional export/private
    if (check(TokenType::Exported)) {
        obj.exported = true;
        advance();
    } else if (check(TokenType::Private)) {
        obj.privateDef = true;
        advance();
    }

    // Widget class name
    if (check(TokenType::Identifier)) {
        obj.widgetClass = advance().text;
        if (!symbols_.isValidWidgetClass(obj.widgetClass)) {
            error("unknown widget class '" + obj.widgetClass + "'");
        }
    }

    // Object body { ... }
    if (match(TokenType::LeftBrace)) {
        while (!check(TokenType::RightBrace) && !check(TokenType::Eof)) {
            if (check(TokenType::Arguments)) {
                parseArgumentsBlock(obj);
            } else if (check(TokenType::Callbacks)) {
                parseCallbacksBlock(obj);
            } else if (check(TokenType::Controls)) {
                parseControlsBlock(obj);
            } else {
                error("expected 'arguments', 'callbacks', or 'controls' in object body");
                synchronize();
            }
        }
        expect(TokenType::RightBrace, "'}'");
    }

    expect(TokenType::Semicolon, "';'");

    symbols_.define(obj.name, SymbolKind::Object, obj.widgetClass, obj.exported, obj.line);
    mod.objects.push_back(std::move(obj));
}

void Parser::parseArgumentsBlock(ObjectNode& obj) {
    advance(); // consume 'arguments'
    expect(TokenType::LeftBrace, "'{'");

    while (!check(TokenType::RightBrace) && !check(TokenType::Eof)) {
        ArgumentNode arg;
        arg.line = current_.line;

        if (check(TokenType::Identifier)) {
            arg.name = advance().text;
        } else {
            error("expected resource name in arguments block");
            synchronize();
            continue;
        }

        expect(TokenType::Equals, "'='");
        arg.value = parseExpression();
        expect(TokenType::Semicolon, "';'");

        obj.arguments.push_back(std::move(arg));
    }

    expect(TokenType::RightBrace, "'}'");
    match(TokenType::Semicolon);
}

void Parser::parseCallbacksBlock(ObjectNode& obj) {
    advance(); // consume 'callbacks'
    expect(TokenType::LeftBrace, "'{'");

    while (!check(TokenType::RightBrace) && !check(TokenType::Eof)) {
        CallbackNode cb;
        cb.line = current_.line;

        if (check(TokenType::Identifier)) {
            cb.reason = advance().text;
        } else {
            error("expected callback reason name");
            synchronize();
            continue;
        }

        expect(TokenType::Equals, "'='");

        // "procedure" keyword
        expect(TokenType::Procedure, "'procedure'");

        if (check(TokenType::Identifier)) {
            cb.procedure = advance().text;
        }

        // Optional argument: (expr)
        if (match(TokenType::LeftParen)) {
            if (!check(TokenType::RightParen)) {
                cb.argument = parseExpression();
                cb.hasArgument = true;
            }
            expect(TokenType::RightParen, "')'");
        }

        expect(TokenType::Semicolon, "';'");
        obj.callbacks.push_back(std::move(cb));
    }

    expect(TokenType::RightBrace, "'}'");
    match(TokenType::Semicolon);
}

void Parser::parseControlsBlock(ObjectNode& obj) {
    advance(); // consume 'controls'
    expect(TokenType::LeftBrace, "'{'");

    while (!check(TokenType::RightBrace) && !check(TokenType::Eof)) {
        ControlNode ctrl;
        ctrl.line = current_.line;

        // WidgetClass name ;
        // or: WidgetClass name = managed/unmanaged ;
        if (check(TokenType::Identifier)) {
            ctrl.widgetClass = advance().text;
        }

        if (check(TokenType::Identifier)) {
            ctrl.name = advance().text;
        }

        // Optional managed/unmanaged
        if (match(TokenType::Equals)) {
            if (check(TokenType::Managed)) {
                ctrl.managed = true;
                advance();
            } else if (check(TokenType::Unmanaged)) {
                ctrl.managed = false;
                advance();
            }
        }

        expect(TokenType::Semicolon, "';'");
        obj.controls.push_back(std::move(ctrl));
    }

    expect(TokenType::RightBrace, "'}'");
    match(TokenType::Semicolon);
}

// ── Value declarations ───────────────────────────────────────

void Parser::parseValueSection(ModuleNode& mod) {
    advance(); // consume 'value'

    while (!check(TokenType::Eof) && !check(TokenType::Object) &&
           !check(TokenType::Procedure) && !check(TokenType::EndModule) &&
           !check(TokenType::Value) && !check(TokenType::Include)) {

        if (!check(TokenType::Identifier)) break;

        ValueDecl val;
        val.line = current_.line;
        val.name = advance().text;

        expect(TokenType::Colon, "':'");

        // Optional export/private
        if (check(TokenType::Exported)) {
            val.exported = true;
            advance();
        } else if (check(TokenType::Private)) {
            val.privateDef = true;
            advance();
        }

        // Optional type name
        if (check(TokenType::Integer) || check(TokenType::Boolean) ||
            check(TokenType::Float) || check(TokenType::String) ||
            check(TokenType::Color) || check(TokenType::Font) ||
            check(TokenType::Pixmap)) {
            val.typeName = advance().text;
        }

        // Value
        val.value = parseExpression();
        expect(TokenType::Semicolon, "';'");

        symbols_.define(val.name, SymbolKind::Value, val.typeName, val.exported, val.line);
        mod.values.push_back(std::move(val));
    }
}

// ── Include directives ───────────────────────────────────────

void Parser::parseIncludeDirective(ModuleNode& mod) {
    advance(); // consume 'include'
    // include file 'filename.uil' ;
    if (check(TokenType::Identifier) && current_.text == "file") {
        advance();
    }

    IncludeDecl inc;
    inc.line = current_.line;
    if (check(TokenType::StringLiteral)) {
        inc.filename = advance().text;
    }
    expect(TokenType::Semicolon, "';'");
    mod.includes.push_back(std::move(inc));
}

// ── Expression parsing ───────────────────────────────────────

Expression Parser::parseExpression() {
    Expression left = parsePrimary();

    // Handle string concatenation with &
    while (check(TokenType::Ampersand)) {
        advance();
        Expression right = parsePrimary();

        Expression concat;
        concat.kind = Expression::Kind::Concatenation;
        concat.line = left.line;
        concat.children.push_back(std::move(left));
        concat.children.push_back(std::move(right));
        left = std::move(concat);
    }

    // Handle arithmetic operators
    while (check(TokenType::Plus) || check(TokenType::Minus) ||
           check(TokenType::Multiply) || check(TokenType::Divide)) {
        Token op = advance();
        Expression right = parsePrimary();

        Expression binOp;
        binOp.kind = Expression::Kind::BinaryOp;
        binOp.identifier = op.text;
        binOp.line = left.line;
        binOp.children.push_back(std::move(left));
        binOp.children.push_back(std::move(right));
        left = std::move(binOp);
    }

    return left;
}

Expression Parser::parsePrimary() {
    Expression expr;
    expr.line = current_.line;

    if (check(TokenType::IntegerLiteral)) {
        expr.kind = Expression::Kind::Literal;
        expr.value = static_cast<int64_t>(std::stoll(advance().text));
        return expr;
    }

    if (check(TokenType::FloatLiteral)) {
        expr.kind = Expression::Kind::Literal;
        expr.value = std::stod(advance().text);
        return expr;
    }

    if (check(TokenType::StringLiteral)) {
        expr.kind = Expression::Kind::Literal;
        expr.value = advance().text;
        return expr;
    }

    if (check(TokenType::BooleanLiteral)) {
        expr.kind = Expression::Kind::Literal;
        std::string text = advance().text;
        std::string lower = text;
        std::transform(lower.begin(), lower.end(), lower.begin(),
                       [](unsigned char c) { return std::tolower(c); });
        expr.value = (lower == "true" || lower == "on");
        return expr;
    }

    if (check(TokenType::On)) {
        advance();
        expr.kind = Expression::Kind::Literal;
        expr.value = true;
        return expr;
    }

    if (check(TokenType::Off)) {
        advance();
        expr.kind = Expression::Kind::Literal;
        expr.value = false;
        return expr;
    }

    // compound_string(...)
    if (check(TokenType::CompoundStringFunc)) {
        return parseCompoundString();
    }

    // Negation
    if (check(TokenType::Tilde) || check(TokenType::Minus)) {
        advance();
        Expression inner = parsePrimary();
        expr.kind = Expression::Kind::Negate;
        expr.children.push_back(std::move(inner));
        return expr;
    }

    // Identifier reference
    if (check(TokenType::Identifier)) {
        expr.kind = Expression::Kind::Identifier;
        expr.identifier = advance().text;
        return expr;
    }

    // Parenthesized expression
    if (match(TokenType::LeftParen)) {
        expr = parseExpression();
        expect(TokenType::RightParen, "')'");
        return expr;
    }

    error("expected expression, got '" + current_.text + "'");
    advance();
    return expr;
}

Expression Parser::parseCompoundString() {
    Expression expr;
    expr.kind = Expression::Kind::CompoundString;
    expr.line = current_.line;

    advance(); // consume 'compound_string'
    expect(TokenType::LeftParen, "'('");

    // First argument: the string
    if (check(TokenType::StringLiteral)) {
        expr.value = advance().text;
    }

    // Optional keyword arguments
    while (match(TokenType::Comma)) {
        if (check(TokenType::Separate)) {
            advance();
            expect(TokenType::Equals, "'='");
            if (check(TokenType::BooleanLiteral) || check(TokenType::On) || check(TokenType::Off)) {
                std::string text = advance().text;
                std::string lower = text;
                std::transform(lower.begin(), lower.end(), lower.begin(),
                               [](unsigned char c) { return std::tolower(c); });
                expr.separate = (lower == "true" || lower == "on");
            }
        } else if (check(TokenType::CharacterSet)) {
            advance();
            expect(TokenType::Equals, "'='");
            advance(); // skip charset value
        } else {
            advance(); // skip unknown kwarg
            if (match(TokenType::Equals)) {
                advance(); // skip value
            }
        }
    }

    expect(TokenType::RightParen, "')'");
    return expr;
}

} // namespace motif::uil
