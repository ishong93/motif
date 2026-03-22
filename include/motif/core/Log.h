#pragma once

#include <string>
#include <functional>
#include <sstream>
#include <cstdint>

namespace motif {

/// Log severity levels
enum class LogLevel : uint8_t {
    Debug   = 0,
    Info    = 1,
    Warning = 2,
    Error   = 3,
    Fatal   = 4
};

/// Convert LogLevel to string
inline const char* logLevelString(LogLevel level) {
    switch (level) {
        case LogLevel::Debug:   return "DEBUG";
        case LogLevel::Info:    return "INFO";
        case LogLevel::Warning: return "WARNING";
        case LogLevel::Error:   return "ERROR";
        case LogLevel::Fatal:   return "FATAL";
    }
    return "UNKNOWN";
}

/// Custom log handler callback
using LogHandler = std::function<void(LogLevel level, const char* component,
                                       const char* file, int line,
                                       const std::string& message)>;

/// Central logging facility for the Motif toolkit.
/// Thread-safe singleton with configurable output and minimum level.
class Log {
public:
    static Log& instance();

    /// Set the minimum severity level for output (default: Warning)
    void setLevel(LogLevel level);
    LogLevel level() const;

    /// Install a custom log handler (replaces default stderr output)
    void setHandler(LogHandler handler);

    /// Reset to default stderr handler
    void resetHandler();

    /// Core logging function
    void log(LogLevel level, const char* component,
             const char* file, int line, const std::string& message);

    // Non-copyable
    Log(const Log&) = delete;
    Log& operator=(const Log&) = delete;

private:
    Log();
    ~Log() = default;

    LogLevel minLevel_ = LogLevel::Warning;
    LogHandler handler_;
};

/// RAII helper for stream-style logging
class LogStream {
public:
    LogStream(LogLevel level, const char* component, const char* file, int line)
        : level_(level), component_(component), file_(file), line_(line) {}

    ~LogStream() {
        Log::instance().log(level_, component_, file_, line_, stream_.str());
    }

    template <typename T>
    LogStream& operator<<(const T& value) {
        stream_ << value;
        return *this;
    }

private:
    LogLevel level_;
    const char* component_;
    const char* file_;
    int line_;
    std::ostringstream stream_;
};

/// No-op stream that discards all output (used when level is filtered)
class LogNullStream {
public:
    template <typename T>
    LogNullStream& operator<<(const T&) { return *this; }
};

} // namespace motif

// ── Logging macros ──────────────────────────────────────────

/// General log macro with component tag
#define MOTIF_LOG(level, component) \
    (::motif::Log::instance().level() <= (level)) \
        ? (::motif::LogStream((level), (component), __FILE__, __LINE__)) \
        : ::motif::LogStream((level), (component), __FILE__, __LINE__)

/// Convenience macros using "motif" as default component
#define MOTIF_LOG_DEBUG   MOTIF_LOG(::motif::LogLevel::Debug,   "motif")
#define MOTIF_LOG_INFO    MOTIF_LOG(::motif::LogLevel::Info,    "motif")
#define MOTIF_LOG_WARNING MOTIF_LOG(::motif::LogLevel::Warning, "motif")
#define MOTIF_LOG_ERROR   MOTIF_LOG(::motif::LogLevel::Error,   "motif")
#define MOTIF_LOG_FATAL   MOTIF_LOG(::motif::LogLevel::Fatal,   "motif")

/// Component-specific convenience macros
#define XM_LOG_DEBUG(comp)   MOTIF_LOG(::motif::LogLevel::Debug,   (comp))
#define XM_LOG_INFO(comp)    MOTIF_LOG(::motif::LogLevel::Info,    (comp))
#define XM_LOG_WARNING(comp) MOTIF_LOG(::motif::LogLevel::Warning, (comp))
#define XM_LOG_ERROR(comp)   MOTIF_LOG(::motif::LogLevel::Error,   (comp))
