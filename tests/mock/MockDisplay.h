#pragma once

#include <X11/Xlib.h>
#include <string>
#include <vector>
#include <unordered_map>
#include <cstdint>

namespace motif::test {

/// Mock X11 display for testing without a real X server.
/// Provides fake Display*, Window, and GC values that allow
/// widget creation and property manipulation to succeed in tests.
///
/// Usage:
///   MockDisplay mock;
///   if (mock.open()) {
///       // tests can use Application::instance() with mock display
///   }
class MockDisplay {
public:
    MockDisplay();
    ~MockDisplay();

    /// Attempt to open a real display; if unavailable, set up mock mode.
    /// Returns true if either real or mock is available.
    bool open();

    /// Close the display (real or mock).
    void close();

    /// True if running against a real X server.
    bool isReal() const { return realDisplay_; }

    /// True if display is available (real or mock).
    bool isAvailable() const { return available_; }

    /// Get the display pointer. In mock mode, this may be nullptr
    /// but skipIfNoDisplay() can be used to skip tests gracefully.
    Display* display() const { return display_; }

    /// Skip the current test if no display (real or mock) is available.
    /// Call at the start of tests that need X11.
    static void skipIfNoDisplay();

    /// Create a fake window ID (for mock mode)
    Window createWindow(int x, int y, int w, int h);

    /// Track properties set on windows
    void setProperty(Window w, const std::string& name, const std::string& value);
    std::string getProperty(Window w, const std::string& name) const;

    /// Track event sends
    struct SentEvent {
        Window target;
        int type;
        long data[5] = {};
    };
    const std::vector<SentEvent>& sentEvents() const { return sentEvents_; }
    void recordEvent(const SentEvent& ev) { sentEvents_.push_back(ev); }

private:
    Display* display_ = nullptr;
    bool realDisplay_ = false;
    bool available_ = false;

    // Mock state
    Window nextWindow_ = 1000;
    std::unordered_map<Window, std::unordered_map<std::string, std::string>> properties_;
    std::vector<SentEvent> sentEvents_;
};

/// RAII wrapper — opens display in constructor, closes in destructor.
/// Use with GTEST: MockDisplayFixture as base class.
class MockDisplayFixture : public ::testing::Test {
protected:
    void SetUp() override;
    void TearDown() override;

    MockDisplay mock;
};

/// Macro: skip test if no display is available
#define SKIP_IF_NO_DISPLAY() \
    do { if (!mock.isAvailable()) { GTEST_SKIP() << "No X display available"; } } while(0)

} // namespace motif::test
