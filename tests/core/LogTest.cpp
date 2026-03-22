#include <gtest/gtest.h>
#include <motif/core/Log.h>
#include <motif/core/Error.h>

using namespace motif;

TEST(LogTest, DefaultLevel) {
    auto& log = Log::instance();
    EXPECT_EQ(log.level(), LogLevel::Warning);
}

TEST(LogTest, SetLevel) {
    auto& log = Log::instance();
    log.setLevel(LogLevel::Debug);
    EXPECT_EQ(log.level(), LogLevel::Debug);

    // Reset
    log.setLevel(LogLevel::Warning);
}

TEST(LogTest, CustomHandler) {
    auto& log = Log::instance();
    log.setLevel(LogLevel::Debug);

    int callCount = 0;
    std::string lastMessage;
    LogLevel lastLevel = LogLevel::Debug;

    log.setHandler([&](LogLevel level, const char*, const char*, int,
                       const std::string& msg) {
        callCount++;
        lastLevel = level;
        lastMessage = msg;
    });

    log.log(LogLevel::Warning, "test", __FILE__, __LINE__, "test message");
    EXPECT_EQ(callCount, 1);
    EXPECT_EQ(lastLevel, LogLevel::Warning);
    EXPECT_EQ(lastMessage, "test message");

    // Reset
    log.resetHandler();
    log.setLevel(LogLevel::Warning);
}

TEST(LogTest, LevelFiltering) {
    auto& log = Log::instance();
    log.setLevel(LogLevel::Error);

    int callCount = 0;
    log.setHandler([&](LogLevel, const char*, const char*, int,
                       const std::string&) {
        callCount++;
    });

    log.log(LogLevel::Debug, "test", __FILE__, __LINE__, "filtered");
    log.log(LogLevel::Info, "test", __FILE__, __LINE__, "filtered");
    log.log(LogLevel::Warning, "test", __FILE__, __LINE__, "filtered");
    EXPECT_EQ(callCount, 0);

    log.log(LogLevel::Error, "test", __FILE__, __LINE__, "shown");
    EXPECT_EQ(callCount, 1);

    // Reset
    log.resetHandler();
    log.setLevel(LogLevel::Warning);
}

TEST(LogTest, LogLevelString) {
    EXPECT_STREQ(logLevelString(LogLevel::Debug), "DEBUG");
    EXPECT_STREQ(logLevelString(LogLevel::Info), "INFO");
    EXPECT_STREQ(logLevelString(LogLevel::Warning), "WARNING");
    EXPECT_STREQ(logLevelString(LogLevel::Error), "ERROR");
    EXPECT_STREQ(logLevelString(LogLevel::Fatal), "FATAL");
}

TEST(LogTest, StreamStyle) {
    auto& log = Log::instance();
    log.setLevel(LogLevel::Debug);

    std::string captured;
    log.setHandler([&](LogLevel, const char*, const char*, int,
                       const std::string& msg) {
        captured = msg;
    });

    // Use LogStream directly (macro would reference this file)
    {
        LogStream stream(LogLevel::Info, "test", __FILE__, __LINE__);
        stream << "value=" << 42 << " ok";
    }
    EXPECT_EQ(captured, "value=42 ok");

    // Reset
    log.resetHandler();
    log.setLevel(LogLevel::Warning);
}

// ── Error tests ──

TEST(ErrorTest, MotifErrorHierarchy) {
    try {
        throw DisplayError("cannot open display");
    } catch (const MotifError& e) {
        EXPECT_STREQ(e.what(), "cannot open display");
    }

    try {
        throw ResourceError("missing resource");
    } catch (const MotifError& e) {
        EXPECT_STREQ(e.what(), "missing resource");
    }

    try {
        throw WidgetError("not realized");
    } catch (const MotifError& e) {
        EXPECT_STREQ(e.what(), "not realized");
    }

    try {
        throw UilError("syntax error");
    } catch (const MotifError& e) {
        EXPECT_STREQ(e.what(), "syntax error");
    }
}

TEST(ErrorTest, X11ErrorHandlerState) {
    auto& handler = X11ErrorHandler::instance();
    handler.resetError();
    EXPECT_FALSE(handler.errorOccurred());
}

TEST(ErrorTest, X11ErrorCallback) {
    auto& handler = X11ErrorHandler::instance();
    handler.resetError();

    int callCount = 0;
    handler.setCallback([&](const X11ErrorInfo& info) {
        callCount++;
    });

    X11ErrorInfo info;
    info.errorCode = 8;
    info.description = "BadMatch";
    handler.recordError(info);

    EXPECT_TRUE(handler.errorOccurred());
    EXPECT_EQ(callCount, 1);
    EXPECT_EQ(handler.lastError().description, "BadMatch");

    handler.resetError();
    handler.setCallback(nullptr);
}
