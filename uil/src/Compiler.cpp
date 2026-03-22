#include <motif/uil/Compiler.h>
#include <motif/uil/Lexer.h>
#include <motif/uil/Parser.h>
#include <fstream>
#include <sstream>
#include <cstring>
#include <algorithm>
#include <iostream>

namespace motif::uil {

// IDB format constants (matching ResourceLoader.cpp)
static constexpr uint32_t RECORD_SIZE = 4096;
static constexpr uint32_t IDB_FILE_HEADER_VALID   = 0xABCDEF01;
static constexpr uint32_t IDB_RECORD_HEADER_VALID  = 0xBEADCADE;
static constexpr uint32_t IDB_DATA_ENTRY_VALID     = 0xCEADCADE;

// Record types
static constexpr uint8_t IDB_GROUP_WIDGET    = 1;
static constexpr uint8_t IDB_GROUP_VALUE     = 2;
static constexpr uint8_t IDB_GROUP_MODULE    = 3;

static constexpr uint8_t IDB_TYPE_WIDGET     = 1;
static constexpr uint8_t IDB_TYPE_STRING     = 2;
static constexpr uint8_t IDB_TYPE_INTEGER    = 3;
static constexpr uint8_t IDB_TYPE_BOOLEAN    = 4;
static constexpr uint8_t IDB_TYPE_FLOAT      = 5;
static constexpr uint8_t IDB_TYPE_COMPOUND   = 6;

Compiler::Compiler(const Options& opts) : opts_(opts) {}

bool Compiler::compileFile(const std::string& path) {
    std::ifstream file(path);
    if (!file.is_open()) {
        errors_.push_back("cannot open input file: " + path);
        return false;
    }

    std::ostringstream ss;
    ss << file.rdbuf();
    std::string source = ss.str();

    return compileSource(source, path);
}

bool Compiler::compileSource(const std::string& source, const std::string& filename) {
    // Phase 1: Lexing + Parsing
    Lexer lexer(source, filename);
    Parser parser(lexer);

    auto module = parser.parseModule();

    if (parser.hasErrors()) {
        errors_.insert(errors_.end(), parser.errors().begin(), parser.errors().end());
        return false;
    }

    if (lexer.hasErrors()) {
        errors_.insert(errors_.end(), lexer.errors().begin(), lexer.errors().end());
        return false;
    }

    if (!module) {
        errors_.push_back("failed to parse module");
        return false;
    }

    if (opts_.verbose) {
        std::cout << "Parsed module '" << module->name << "' with "
                  << module->objects.size() << " objects, "
                  << module->values.size() << " values, "
                  << module->procedures.size() << " procedures\n";
    }

    // Phase 2: Generate UID binary
    std::string outPath = opts_.outputFile;
    if (outPath.empty()) {
        // Replace .uil extension with .uid
        outPath = filename;
        size_t dot = outPath.rfind('.');
        if (dot != std::string::npos) {
            outPath = outPath.substr(0, dot);
        }
        outPath += ".uid";
    }

    return generateUID(*module, outPath);
}

// ── Expression evaluation ─────────────────────────────────────

Expression Compiler::evaluateExpr(const Expression& expr) {
    switch (expr.kind) {
        case Expression::Kind::Literal:
        case Expression::Kind::Identifier:
        case Expression::Kind::CompoundString:
            return expr;

        case Expression::Kind::Negate: {
            if (expr.children.empty()) return expr;
            auto child = evaluateExpr(expr.children[0]);
            if (child.kind != Expression::Kind::Literal) return expr;
            if (auto* iv = std::get_if<int64_t>(&child.value)) {
                child.value = -(*iv);
                return child;
            }
            if (auto* dv = std::get_if<double>(&child.value)) {
                child.value = -(*dv);
                return child;
            }
            if (auto* bv = std::get_if<bool>(&child.value)) {
                child.value = !(*bv);
                return child;
            }
            return expr;
        }

        case Expression::Kind::Concatenation: {
            if (expr.children.size() < 2) return expr;
            auto lhs = evaluateExpr(expr.children[0]);
            auto rhs = evaluateExpr(expr.children[1]);
            // String concatenation
            std::string result;
            if (auto* ls = std::get_if<std::string>(&lhs.value)) {
                result = *ls;
            } else {
                return expr; // can't concat non-strings at compile time
            }
            if (auto* rs = std::get_if<std::string>(&rhs.value)) {
                result += *rs;
            } else {
                return expr;
            }
            Expression out;
            out.kind = Expression::Kind::Literal;
            out.value = result;
            out.line = expr.line;
            return out;
        }

        case Expression::Kind::BinaryOp: {
            if (expr.children.size() < 2) return expr;
            auto lhs = evaluateExpr(expr.children[0]);
            auto rhs = evaluateExpr(expr.children[1]);
            if (lhs.kind != Expression::Kind::Literal ||
                rhs.kind != Expression::Kind::Literal) {
                return expr;
            }

            // Determine operator from identifier field ('+', '-', '*', '/')
            char op = expr.identifier.empty() ? '+' : expr.identifier[0];

            // Integer arithmetic
            auto* li = std::get_if<int64_t>(&lhs.value);
            auto* ri = std::get_if<int64_t>(&rhs.value);
            if (li && ri) {
                int64_t r = 0;
                switch (op) {
                    case '+': r = *li + *ri; break;
                    case '-': r = *li - *ri; break;
                    case '*': r = *li * *ri; break;
                    case '/': r = (*ri != 0) ? *li / *ri : 0; break;
                    default: return expr;
                }
                Expression out;
                out.kind = Expression::Kind::Literal;
                out.value = r;
                out.line = expr.line;
                return out;
            }

            // Float arithmetic
            auto* ld = std::get_if<double>(&lhs.value);
            auto* rd = std::get_if<double>(&rhs.value);
            double lv = ld ? *ld : (li ? static_cast<double>(*li) : 0.0);
            double rv = rd ? *rd : (ri ? static_cast<double>(*ri) : 0.0);
            if (ld || rd) { // at least one is float
                double r = 0.0;
                switch (op) {
                    case '+': r = lv + rv; break;
                    case '-': r = lv - rv; break;
                    case '*': r = lv * rv; break;
                    case '/': r = (rv != 0.0) ? lv / rv : 0.0; break;
                    default: return expr;
                }
                Expression out;
                out.kind = Expression::Kind::Literal;
                out.value = r;
                out.line = expr.line;
                return out;
            }

            return expr;
        }
    }
    return expr;
}

void Compiler::writeExpression(std::vector<uint8_t>& out, const Expression& rawExpr) {
    auto expr = evaluateExpr(rawExpr);

    if (expr.kind == Expression::Kind::CompoundString) {
        out.push_back(IDB_TYPE_COMPOUND);
        out.push_back(expr.separate ? 1 : 0);
        if (auto* strVal = std::get_if<std::string>(&expr.value)) {
            writeString(out, *strVal);
        } else if (auto* segs = std::get_if<std::vector<std::string>>(&expr.value)) {
            // Join compound string segments
            std::string joined;
            for (const auto& seg : *segs) {
                if (!joined.empty()) joined += '\n';
                joined += seg;
            }
            writeString(out, joined);
        } else {
            writeString(out, "");
        }
        return;
    }

    if (expr.kind == Expression::Kind::Identifier) {
        out.push_back(IDB_TYPE_STRING);
        out.push_back(0);
        writeString(out, expr.identifier);
        return;
    }

    // Literal values
    if (auto* intVal = std::get_if<int64_t>(&expr.value)) {
        out.push_back(IDB_TYPE_INTEGER);
        out.push_back(0);
        writeUint32(out, static_cast<uint32_t>(*intVal));
    } else if (auto* strVal = std::get_if<std::string>(&expr.value)) {
        out.push_back(IDB_TYPE_STRING);
        out.push_back(0);
        writeString(out, *strVal);
    } else if (auto* boolVal = std::get_if<bool>(&expr.value)) {
        out.push_back(IDB_TYPE_BOOLEAN);
        out.push_back(0);
        writeUint32(out, *boolVal ? 1 : 0);
    } else if (auto* dblVal = std::get_if<double>(&expr.value)) {
        out.push_back(IDB_TYPE_FLOAT);
        out.push_back(0);
        uint64_t raw;
        std::memcpy(&raw, dblVal, sizeof(double));
        writeUint32(out, static_cast<uint32_t>(raw & 0xFFFFFFFF));
        writeUint32(out, static_cast<uint32_t>(raw >> 32));
    } else {
        out.push_back(0); // unknown
        out.push_back(0);
    }
}

// ── IDB binary generation ────────────────────────────────────

void Compiler::writeUint32(std::vector<uint8_t>& out, uint32_t value) {
    out.push_back(static_cast<uint8_t>(value & 0xFF));
    out.push_back(static_cast<uint8_t>((value >> 8) & 0xFF));
    out.push_back(static_cast<uint8_t>((value >> 16) & 0xFF));
    out.push_back(static_cast<uint8_t>((value >> 24) & 0xFF));
}

void Compiler::writeUint16(std::vector<uint8_t>& out, uint16_t value) {
    out.push_back(static_cast<uint8_t>(value & 0xFF));
    out.push_back(static_cast<uint8_t>((value >> 8) & 0xFF));
}

void Compiler::writeString(std::vector<uint8_t>& out, const std::string& str) {
    writeUint16(out, static_cast<uint16_t>(str.size()));
    out.insert(out.end(), str.begin(), str.end());
    // Pad to even boundary
    if (str.size() % 2 != 0) {
        out.push_back(0);
    }
}

void Compiler::padToRecord(std::vector<uint8_t>& out) {
    size_t remainder = out.size() % RECORD_SIZE;
    if (remainder > 0) {
        size_t padding = RECORD_SIZE - remainder;
        out.insert(out.end(), padding, 0);
    }
}

void Compiler::writeHeader(std::vector<uint8_t>& out) {
    writeUint32(out, IDB_FILE_HEADER_VALID);  // magic
    writeUint32(out, 1);                       // version
    writeUint32(out, 0);                       // byte order (little-endian)
    writeUint32(out, 0);                       // record count (filled later)
    writeUint32(out, 0);                       // index record offset (filled later)
    // Pad header to full record
    padToRecord(out);
}

void Compiler::writeModuleRecord(std::vector<uint8_t>& out, const ModuleNode& mod) {
    size_t recordStart = out.size();

    writeUint32(out, IDB_RECORD_HEADER_VALID);
    writeUint32(out, IDB_DATA_ENTRY_VALID);

    writeUint16(out, static_cast<uint16_t>(IDB_GROUP_MODULE));
    writeUint16(out, 0); // type

    writeString(out, mod.name);
    writeString(out, mod.version);

    // Procedure count + names
    writeUint16(out, static_cast<uint16_t>(mod.procedures.size()));
    for (auto& proc : mod.procedures) {
        writeString(out, proc.name);
        out.push_back(proc.hasArgument ? 1 : 0);
        out.push_back(0); // padding
    }

    padToRecord(out);
}

void Compiler::writeObjectRecord(std::vector<uint8_t>& out, const ObjectNode& obj) {
    size_t recordStart = out.size();

    writeUint32(out, IDB_RECORD_HEADER_VALID);
    writeUint32(out, IDB_DATA_ENTRY_VALID);

    writeUint16(out, static_cast<uint16_t>(IDB_GROUP_WIDGET));
    writeUint16(out, static_cast<uint16_t>(IDB_TYPE_WIDGET));

    // Widget name and class
    writeString(out, obj.name);
    writeString(out, obj.widgetClass);

    // Arguments
    writeUint16(out, static_cast<uint16_t>(obj.arguments.size()));
    for (auto& arg : obj.arguments) {
        writeString(out, arg.name);
        writeExpression(out, arg.value);
    }

    // Callbacks
    writeUint16(out, static_cast<uint16_t>(obj.callbacks.size()));
    for (auto& cb : obj.callbacks) {
        writeString(out, cb.reason);
        writeString(out, cb.procedure);
        out.push_back(cb.hasArgument ? 1 : 0);
        if (cb.hasArgument) {
            writeExpression(out, cb.argument);
        }
        out.push_back(0); // padding
    }

    // Controls (children)
    writeUint16(out, static_cast<uint16_t>(obj.controls.size()));
    for (auto& ctrl : obj.controls) {
        writeString(out, ctrl.widgetClass);
        writeString(out, ctrl.name);
        out.push_back(ctrl.managed ? 1 : 0);
        out.push_back(0); // padding
    }

    padToRecord(out);
}

void Compiler::writeValueRecord(std::vector<uint8_t>& out, const ValueDecl& val) {
    writeUint32(out, IDB_RECORD_HEADER_VALID);
    writeUint32(out, IDB_DATA_ENTRY_VALID);

    writeUint16(out, static_cast<uint16_t>(IDB_GROUP_VALUE));

    // Type
    if (val.typeName == "integer") {
        writeUint16(out, IDB_TYPE_INTEGER);
    } else if (val.typeName == "string") {
        writeUint16(out, IDB_TYPE_STRING);
    } else if (val.typeName == "boolean") {
        writeUint16(out, IDB_TYPE_BOOLEAN);
    } else if (val.typeName == "float") {
        writeUint16(out, IDB_TYPE_FLOAT);
    } else {
        writeUint16(out, 0);
    }

    writeString(out, val.name);

    // Serialize value expression (with compile-time evaluation)
    writeExpression(out, val.value);

    out.push_back(val.exported ? 1 : 0);
    out.push_back(0); // padding

    padToRecord(out);
}

void Compiler::writeIndexRecord(std::vector<uint8_t>& out,
                                const std::vector<std::pair<std::string, uint32_t>>& index) {
    writeUint32(out, IDB_RECORD_HEADER_VALID);

    // Index entry count
    writeUint16(out, static_cast<uint16_t>(index.size()));

    for (auto& [name, offset] : index) {
        writeString(out, name);
        writeUint32(out, offset);
    }

    padToRecord(out);
}

bool Compiler::generateUID(const ModuleNode& module, const std::string& outputPath) {
    std::vector<uint8_t> binary;

    // Reserve space for file header (first record)
    writeHeader(binary);

    // Track index entries: name → record offset
    std::vector<std::pair<std::string, uint32_t>> index;

    // Write module record
    uint32_t moduleOffset = static_cast<uint32_t>(binary.size());
    writeModuleRecord(binary, module);
    index.push_back({module.name, moduleOffset});

    // Write object records
    for (auto& obj : module.objects) {
        uint32_t offset = static_cast<uint32_t>(binary.size());
        writeObjectRecord(binary, obj);
        index.push_back({obj.name, offset});
    }

    // Write value records
    for (auto& val : module.values) {
        uint32_t offset = static_cast<uint32_t>(binary.size());
        writeValueRecord(binary, val);
        index.push_back({val.name, offset});
    }

    // Write index record
    uint32_t indexOffset = static_cast<uint32_t>(binary.size());
    writeIndexRecord(binary, index);

    // Patch file header with record count and index offset
    uint32_t recordCount = static_cast<uint32_t>(binary.size() / RECORD_SIZE);
    // Record count at offset 12
    binary[12] = static_cast<uint8_t>(recordCount & 0xFF);
    binary[13] = static_cast<uint8_t>((recordCount >> 8) & 0xFF);
    binary[14] = static_cast<uint8_t>((recordCount >> 16) & 0xFF);
    binary[15] = static_cast<uint8_t>((recordCount >> 24) & 0xFF);
    // Index offset at offset 16
    binary[16] = static_cast<uint8_t>(indexOffset & 0xFF);
    binary[17] = static_cast<uint8_t>((indexOffset >> 8) & 0xFF);
    binary[18] = static_cast<uint8_t>((indexOffset >> 16) & 0xFF);
    binary[19] = static_cast<uint8_t>((indexOffset >> 24) & 0xFF);

    // Write to file
    std::ofstream outFile(outputPath, std::ios::binary);
    if (!outFile.is_open()) {
        errors_.push_back("cannot create output file: " + outputPath);
        return false;
    }

    outFile.write(reinterpret_cast<const char*>(binary.data()),
                  static_cast<std::streamsize>(binary.size()));

    if (opts_.verbose) {
        std::cout << "Generated " << outputPath << " ("
                  << binary.size() << " bytes, "
                  << recordCount << " records)\n";
    }

    return true;
}

} // namespace motif::uil
