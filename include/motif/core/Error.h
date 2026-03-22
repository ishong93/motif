#pragma once

#include <stdexcept>
#include <string>
#include <functional>

namespace motif {

/// Base exception for all Motif toolkit errors
class MotifError : public std::runtime_error {
public:
    explicit MotifError(const std::string& what) : std::runtime_error(what) {}
};

/// Error related to X11 display/connection
class DisplayError : public MotifError {
public:
    explicit DisplayError(const std::string& what) : MotifError(what) {}
};

/// Error related to resource management
class ResourceError : public MotifError {
public:
    explicit ResourceError(const std::string& what) : MotifError(what) {}
};

/// Error related to widget operations (e.g., realize without parent)
class WidgetError : public MotifError {
public:
    explicit WidgetError(const std::string& what) : MotifError(what) {}
};

/// Error related to UIL compilation
class UilError : public MotifError {
public:
    explicit UilError(const std::string& what) : MotifError(what) {}
};

/// X11 error information captured from XErrorEvent
struct X11ErrorInfo {
    unsigned char errorCode = 0;
    unsigned char requestCode = 0;
    unsigned char minorCode = 0;
    unsigned long resourceId = 0;
    std::string description;
};

/// X11 error handler that integrates with the Log system.
/// Installs itself as the X error handler and can optionally
/// forward to a user-provided callback.
class X11ErrorHandler {
public:
    using ErrorCallback = std::function<void(const X11ErrorInfo&)>;

    static X11ErrorHandler& instance();

    /// Install as the active X error handler
    void install();

    /// Set a callback for X11 errors (in addition to logging)
    void setCallback(ErrorCallback cb);

    /// Check if an error occurred since last reset
    bool errorOccurred() const { return errorOccurred_; }

    /// Get last error info
    const X11ErrorInfo& lastError() const { return lastError_; }

    /// Reset error state
    void resetError() { errorOccurred_ = false; }

    // Non-copyable
    X11ErrorHandler(const X11ErrorHandler&) = delete;
    X11ErrorHandler& operator=(const X11ErrorHandler&) = delete;

    /// Called by the static X error handler
    void recordError(const X11ErrorInfo& info);

private:
    X11ErrorHandler() = default;

    bool errorOccurred_ = false;
    X11ErrorInfo lastError_;
    ErrorCallback callback_;
};

} // namespace motif
