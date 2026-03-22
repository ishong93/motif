#pragma once

#include <X11/Xlib.h>
#include <string>

namespace motif::core {

/// HiDPI / display scaling support.
/// Detects the DPI scaling factor from multiple sources
/// and provides helpers for scaling pixel coordinates.
class HiDPI {
public:
    static HiDPI& instance();

    /// Detect scale factor from display properties.
    /// Call after Display is opened.
    void initialize(Display* dpy, int screen);

    /// The scaling factor (1.0 = 96 DPI, 2.0 = 192 DPI, etc.)
    double scaleFactor() const { return scaleFactor_; }

    /// Override the scale factor (e.g., from user config)
    void setScaleFactor(double factor);

    /// Physical DPI of the display
    double dpi() const { return dpi_; }

    /// Scale a pixel value by the scale factor
    int scale(int value) const;

    /// Scale a double value
    double scaleD(double value) const;

    /// Unscale (physical → logical)
    int unscale(int value) const;

    /// Convenience: scale common widget metrics
    int borderWidth() const { return scale(1); }
    int defaultPadding() const { return scale(4); }
    int defaultMargin() const { return scale(8); }
    int defaultFontSize() const { return scale(12); }

private:
    HiDPI() = default;

    double detectDpiFromXrdb(Display* dpy) const;
    double detectDpiFromScreen(Display* dpy, int screen) const;
    double detectDpiFromGtkSettings(Display* dpy) const;

    double scaleFactor_ = 1.0;
    double dpi_ = 96.0;
    bool initialized_ = false;
};

} // namespace motif::core
