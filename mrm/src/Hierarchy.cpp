#include <motif/mrm/ResourceLoader.h>
#include <motif/core/Log.h>

#include <algorithm>
#include <cstdlib>
#include <fstream>
#include <sstream>

namespace motif::mrm {

/// Resolve a UID file path using the standard Motif search path.
/// Checks:
///   1. The path as given
///   2. $UIDPATH directories (colon-separated)
///   3. /usr/lib/X11/uid/
///   4. Current directory
std::vector<std::string> resolveUidPaths(const std::vector<std::string>& names) {
    std::vector<std::string> searchDirs;

    // Collect UIDPATH entries
    const char* uidPath = std::getenv("UIDPATH");
    if (uidPath) {
        std::istringstream ss(uidPath);
        std::string dir;
        while (std::getline(ss, dir, ':')) {
            if (!dir.empty()) searchDirs.push_back(dir);
        }
    }

    // Standard system paths
    searchDirs.push_back("/usr/lib/X11/uid");
    searchDirs.push_back("/usr/share/X11/uid");
    searchDirs.push_back(".");

    std::vector<std::string> resolved;

    for (const auto& name : names) {
        // Try direct path first
        {
            std::ifstream test(name);
            if (test.good()) {
                resolved.push_back(name);
                continue;
            }
        }

        // Try with .uid extension if not present
        std::string withExt = name;
        if (name.size() < 4 || name.substr(name.size() - 4) != ".uid") {
            withExt = name + ".uid";
        }

        bool found = false;
        // Try direct with extension
        {
            std::ifstream test(withExt);
            if (test.good()) {
                resolved.push_back(withExt);
                found = true;
            }
        }

        if (!found) {
            // Search in directories
            for (const auto& dir : searchDirs) {
                std::string path = dir + "/" + withExt;
                std::ifstream test(path);
                if (test.good()) {
                    resolved.push_back(path);
                    found = true;
                    break;
                }
            }
        }

        if (!found) {
            XM_LOG_WARNING("MRM") << "UID file not found: " << name;
        }
    }

    return resolved;
}

/// Convenience: resolve and open a hierarchy from logical names
bool openHierarchyFromNames(const std::vector<std::string>& names) {
    auto paths = resolveUidPaths(names);
    if (paths.empty()) {
        MOTIF_LOG_ERROR << "MRM: no UID files found";
        return false;
    }
    return ResourceLoader::instance().openHierarchy(paths);
}

} // namespace motif::mrm
