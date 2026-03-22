#pragma once

/// @file motif/Version.h
/// Compile-time and runtime version information for the Motif C++ toolkit.

#define MOTIF_VERSION_MAJOR 3
#define MOTIF_VERSION_MINOR 0
#define MOTIF_VERSION_PATCH 0

/// Version as a single integer: (major * 10000 + minor * 100 + patch)
#define MOTIF_VERSION ((MOTIF_VERSION_MAJOR * 10000) + \
                       (MOTIF_VERSION_MINOR * 100) + \
                       MOTIF_VERSION_PATCH)

/// Version as string literal
#define MOTIF_VERSION_STRING "3.0.0"

/// Check if the library version is at least the given version
#define MOTIF_VERSION_CHECK(major, minor, patch) \
    (MOTIF_VERSION >= ((major) * 10000 + (minor) * 100 + (patch)))

namespace motif {

/// Runtime version information
struct Version {
    static constexpr int major = MOTIF_VERSION_MAJOR;
    static constexpr int minor = MOTIF_VERSION_MINOR;
    static constexpr int patch = MOTIF_VERSION_PATCH;

    /// Returns "3.0.0"
    static constexpr const char* string() { return MOTIF_VERSION_STRING; }

    /// Returns version as single integer for comparison
    static constexpr int number() { return MOTIF_VERSION; }

    /// Check if runtime version >= given version
    static constexpr bool isAtLeast(int maj, int min, int pat) {
        return MOTIF_VERSION >= (maj * 10000 + min * 100 + pat);
    }
};

} // namespace motif
