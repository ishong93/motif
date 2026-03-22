#pragma once

#include <motif/uil/AST.h>
#include <string>
#include <vector>
#include <cstdint>

namespace motif::uil {

// Compiles a parsed UIL module into a UID binary file (IDB format).
class Compiler {
public:
    struct Options {
        std::string inputFile;
        std::string outputFile;    // defaults to input with .uid extension
        bool verbose = false;
        std::vector<std::string> includePaths;
    };

    explicit Compiler(const Options& opts);

    // High-level: compile a .uil file to .uid
    bool compileFile(const std::string& path);

    // Compile from source string
    bool compileSource(const std::string& source, const std::string& filename = "<input>");

    bool hasErrors() const { return !errors_.empty(); }
    const std::vector<std::string>& errors() const { return errors_; }
    const std::vector<std::string>& warnings() const { return warnings_; }

private:
    // Code generation: write IDB binary
    bool generateUID(const ModuleNode& module, const std::string& outputPath);

    // IDB record writing helpers
    void writeHeader(std::vector<uint8_t>& out);
    void writeModuleRecord(std::vector<uint8_t>& out, const ModuleNode& mod);
    void writeObjectRecord(std::vector<uint8_t>& out, const ObjectNode& obj);
    void writeValueRecord(std::vector<uint8_t>& out, const ValueDecl& val);
    void writeIndexRecord(std::vector<uint8_t>& out,
                          const std::vector<std::pair<std::string, uint32_t>>& index);

    // Expression evaluation — resolve at compile time
    Expression evaluateExpr(const Expression& expr);
    void writeExpression(std::vector<uint8_t>& out, const Expression& expr);

    // Helpers
    void writeUint32(std::vector<uint8_t>& out, uint32_t value);
    void writeUint16(std::vector<uint8_t>& out, uint16_t value);
    void writeString(std::vector<uint8_t>& out, const std::string& str);
    void padToRecord(std::vector<uint8_t>& out);

    Options opts_;
    std::vector<std::string> errors_;
    std::vector<std::string> warnings_;
};

} // namespace motif::uil
