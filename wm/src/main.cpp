#include <motif/wm/WindowManager.h>

#include <iostream>
#include <cstring>

int main(int argc, char* argv[]) {
    std::string displayName;

    for (int i = 1; i < argc; ++i) {
        if (std::strcmp(argv[i], "-display") == 0 && i + 1 < argc) {
            displayName = argv[++i];
        } else if (std::strcmp(argv[i], "--help") == 0 || std::strcmp(argv[i], "-h") == 0) {
            std::cout << "Usage: motif-wm [-display name]\n";
            return 0;
        }
    }

    motif::wm::WindowManager wm;

    if (!wm.initialize(displayName)) {
        return 1;
    }

    std::cout << "motif-wm: starting\n";
    wm.run();

    return 0;
}
