#pragma once

#include <string>
#include <vector>

namespace motif::mrm {

/// Resolve UID file paths using $UIDPATH and standard search directories.
std::vector<std::string> resolveUidPaths(const std::vector<std::string>& names);

/// Resolve logical UID names and open them as a hierarchy.
bool openHierarchyFromNames(const std::vector<std::string>& names);

} // namespace motif::mrm
