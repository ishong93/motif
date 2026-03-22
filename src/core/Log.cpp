#include <motif/core/Log.h>

#include <cstdio>
#include <ctime>
#include <cstdlib>

namespace motif {

Log::Log() {
    // Default handler writes to stderr
    handler_ = [](LogLevel level, const char* component,
                  const char* file, int line, const std::string& message) {
        // Timestamp
        std::time_t now = std::time(nullptr);
        char timeBuf[32];
        std::strftime(timeBuf, sizeof(timeBuf), "%H:%M:%S",
                      std::localtime(&now));

        // Extract filename from path
        const char* basename = file;
        for (const char* p = file; *p; ++p) {
            if (*p == '/' || *p == '\\') basename = p + 1;
        }

        std::fprintf(stderr, "[%s] %s (%s) %s:%d: %s\n",
                     timeBuf, logLevelString(level), component,
                     basename, line, message.c_str());

        if (level == LogLevel::Fatal) {
            std::fflush(stderr);
            std::abort();
        }
    };
}

Log& Log::instance() {
    static Log log;
    return log;
}

void Log::setLevel(LogLevel level) {
    minLevel_ = level;
}

LogLevel Log::level() const {
    return minLevel_;
}

void Log::setHandler(LogHandler handler) {
    if (handler) {
        handler_ = std::move(handler);
    }
}

void Log::resetHandler() {
    // Re-construct with default handler
    *this = Log();
}

void Log::log(LogLevel level, const char* component,
              const char* file, int line, const std::string& message) {
    if (level < minLevel_) return;
    if (handler_) {
        handler_(level, component, file, line, message);
    }
}

} // namespace motif
