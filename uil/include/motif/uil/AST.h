#pragma once

#include <string>
#include <vector>
#include <variant>
#include <memory>
#include <cstdint>

namespace motif::uil {

// Forward declarations
struct ModuleNode;
struct ObjectNode;
struct ValueNode;
struct ProcedureDecl;
struct ArgumentNode;
struct CallbackNode;
struct ControlNode;
struct Expression;

// UIL value expression
using ExprValue = std::variant<
    int64_t,                                    // integer
    double,                                     // float
    bool,                                       // boolean
    std::string,                                // string literal
    std::vector<std::string>                    // compound_string segments
>;

struct Expression {
    enum class Kind {
        Literal,          // direct value
        Identifier,       // reference to another value/object
        CompoundString,   // compound_string(...)
        Concatenation,    // expr & expr
        Negate,           // ~expr or -expr
        BinaryOp,         // expr +/-/*// expr
    };

    Kind kind = Kind::Literal;
    ExprValue value;
    std::string identifier;    // for Identifier, CompoundString tag, etc.
    std::vector<Expression> children;  // sub-expressions
    bool separate = false;     // compound_string separate= flag

    int line = 0;
};

struct ArgumentNode {
    std::string name;       // e.g., "XmNlabelString"
    Expression value;
    int line = 0;
};

struct CallbackNode {
    std::string reason;     // e.g., "XmNactivateCallback"
    std::string procedure;  // procedure name
    Expression argument;    // optional argument
    bool hasArgument = false;
    int line = 0;
};

struct ControlNode {
    std::string widgetClass;   // e.g., "XmPushButton"
    std::string name;          // object name
    bool managed = true;
    int line = 0;
};

struct ObjectNode {
    std::string name;
    std::string widgetClass;    // e.g., "XmBulletinBoard"
    bool exported = false;
    bool privateDef = false;

    std::vector<ArgumentNode> arguments;
    std::vector<CallbackNode> callbacks;
    std::vector<ControlNode> controls;
    int line = 0;
};

struct ProcedureDecl {
    std::string name;
    bool hasArgument = false;
    int line = 0;
};

struct ValueDecl {
    std::string name;
    std::string typeName;   // "integer", "string", etc.
    Expression value;
    bool exported = false;
    bool privateDef = false;
    int line = 0;
};

struct IncludeDecl {
    std::string filename;
    int line = 0;
};

struct ModuleNode {
    std::string name;
    std::string version;
    bool caseSensitive = true;

    std::vector<ProcedureDecl> procedures;
    std::vector<ObjectNode> objects;
    std::vector<ValueDecl> values;
    std::vector<IncludeDecl> includes;
};

} // namespace motif::uil
