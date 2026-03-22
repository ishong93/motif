#include <motif/uil/Compiler.h>
#include <iostream>
#include <string>

int main(int argc, char* argv[]) {
    if (argc < 2) {
        std::cerr << "Usage: motif-uil <input.uil> [-o output.uid] [-v]\n";
        return 1;
    }

    motif::uil::Compiler::Options opts;
    opts.inputFile = argv[1];

    for (int i = 2; i < argc; ++i) {
        std::string arg = argv[i];
        if (arg == "-o" && i + 1 < argc) {
            opts.outputFile = argv[++i];
        } else if (arg == "-v" || arg == "--verbose") {
            opts.verbose = true;
        } else if (arg == "-I" && i + 1 < argc) {
            opts.includePaths.push_back(argv[++i]);
        } else {
            std::cerr << "Unknown option: " << arg << "\n";
            return 1;
        }
    }

    motif::uil::Compiler compiler(opts);

    if (!compiler.compileFile(opts.inputFile)) {
        for (auto& err : compiler.errors()) {
            std::cerr << "error: " << err << "\n";
        }
        return 1;
    }

    for (auto& warn : compiler.warnings()) {
        std::cerr << "warning: " << warn << "\n";
    }

    return 0;
}
