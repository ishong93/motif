#pragma once

#include <motif/uil/Lexer.h>
#include <motif/uil/AST.h>
#include <motif/uil/SymbolTable.h>
#include <memory>
#include <vector>
#include <string>

namespace motif::uil {

class Parser {
public:
    explicit Parser(Lexer& lexer);

    std::unique_ptr<ModuleNode> parseModule();

    bool hasErrors() const { return !errors_.empty(); }
    const std::vector<std::string>& errors() const { return errors_; }

private:
    // Module-level
    void parseModuleHeader(ModuleNode& mod);
    void parseModuleBody(ModuleNode& mod);

    // Declarations
    void parseProcedureSection(ModuleNode& mod);
    void parseObjectDecl(ModuleNode& mod);
    void parseValueSection(ModuleNode& mod);
    void parseIncludeDirective(ModuleNode& mod);

    // Object internals
    void parseArgumentsBlock(ObjectNode& obj);
    void parseCallbacksBlock(ObjectNode& obj);
    void parseControlsBlock(ObjectNode& obj);

    // Expressions
    Expression parseExpression();
    Expression parsePrimary();
    Expression parseCompoundString();

    // Helpers
    Token expect(TokenType type, const std::string& context);
    Token advance();
    Token peek();
    bool check(TokenType type);
    bool match(TokenType type);
    void error(const std::string& msg);
    void synchronize();

    Lexer& lexer_;
    Token current_;
    SymbolTable symbols_;
    std::vector<std::string> errors_;
};

} // namespace motif::uil
