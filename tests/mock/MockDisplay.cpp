#include "MockDisplay.h"
#include <motif/core/Application.h>
#include <gtest/gtest.h>
#include <cstdlib>

namespace motif::test {

MockDisplay::MockDisplay() = default;

MockDisplay::~MockDisplay() {
    close();
}

bool MockDisplay::open() {
    // Try to open a real display first
    display_ = XOpenDisplay(nullptr);
    if (display_) {
        realDisplay_ = true;
        available_ = true;
        return true;
    }

    // Try DISPLAY env variable alternatives
    const char* display_env = std::getenv("DISPLAY");
    if (!display_env) {
        // No display available — mock mode
        // We can't create a fake Display* without a real X server,
        // but we can signal that tests should run in headless mode.
        realDisplay_ = false;
        available_ = false;
        return false;
    }

    available_ = false;
    return false;
}

void MockDisplay::close() {
    if (display_ && realDisplay_) {
        XCloseDisplay(display_);
    }
    display_ = nullptr;
    realDisplay_ = false;
    available_ = false;
    properties_.clear();
    sentEvents_.clear();
}

void MockDisplay::skipIfNoDisplay() {
    Display* dpy = XOpenDisplay(nullptr);
    if (dpy) {
        XCloseDisplay(dpy);
        return;
    }
    GTEST_SKIP() << "No X display available — skipping display-dependent test";
}

Window MockDisplay::createWindow(int /*x*/, int /*y*/, int /*w*/, int /*h*/) {
    return nextWindow_++;
}

void MockDisplay::setProperty(Window w, const std::string& name,
                                const std::string& value) {
    properties_[w][name] = value;
}

std::string MockDisplay::getProperty(Window w, const std::string& name) const {
    auto wit = properties_.find(w);
    if (wit == properties_.end()) return {};
    auto pit = wit->second.find(name);
    if (pit == wit->second.end()) return {};
    return pit->second;
}

// ── MockDisplayFixture ──────────────────────────────────────

void MockDisplayFixture::SetUp() {
    mock.open();
}

void MockDisplayFixture::TearDown() {
    mock.close();
}

} // namespace motif::test
