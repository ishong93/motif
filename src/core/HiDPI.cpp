#include <motif/core/HiDPI.h>

#include <X11/Xlib.h>
#include <X11/Xresource.h>
#include <X11/Xatom.h>

#include <cmath>
#include <cstdlib>
#include <cstring>

namespace motif::core {

HiDPI& HiDPI::instance() {
    static HiDPI inst;
    return inst;
}

void HiDPI::initialize(Display* dpy, int screen) {
    if (initialized_) return;
    initialized_ = true;

    // Priority order:
    // 1. MOTIF_SCALE environment variable (explicit override)
    // 2. GDK_SCALE environment variable (GTK apps)
    // 3. Xft.dpi from XRDB (most common desktop setting)
    // 4. _GTK_SCALE from root window
    // 5. Physical DPI from screen dimensions

    // 1. Check MOTIF_SCALE env var
    const char* envScale = std::getenv("MOTIF_SCALE");
    if (envScale) {
        double scale = std::atof(envScale);
        if (scale >= 0.5 && scale <= 4.0) {
            setScaleFactor(scale);
            return;
        }
    }

    // 2. Check GDK_SCALE
    const char* gdkScale = std::getenv("GDK_SCALE");
    if (gdkScale) {
        double scale = std::atof(gdkScale);
        if (scale >= 1.0 && scale <= 4.0) {
            setScaleFactor(scale);
            return;
        }
    }

    if (!dpy) return;

    // 3. Try Xft.dpi from XRDB
    double xrdbDpi = detectDpiFromXrdb(dpy);
    if (xrdbDpi > 0) {
        dpi_ = xrdbDpi;
        scaleFactor_ = std::round(dpi_ / 96.0 * 4.0) / 4.0; // round to .25
        if (scaleFactor_ < 1.0) scaleFactor_ = 1.0;
        return;
    }

    // 4. Try GTK scale from root window
    double gtkDpi = detectDpiFromGtkSettings(dpy);
    if (gtkDpi > 0) {
        dpi_ = gtkDpi;
        scaleFactor_ = std::round(dpi_ / 96.0 * 4.0) / 4.0;
        if (scaleFactor_ < 1.0) scaleFactor_ = 1.0;
        return;
    }

    // 5. Calculate from screen physical dimensions
    double physDpi = detectDpiFromScreen(dpy, screen);
    if (physDpi > 0) {
        dpi_ = physDpi;
        // Only scale if DPI is significantly above 96
        if (dpi_ > 140) {
            scaleFactor_ = std::round(dpi_ / 96.0 * 4.0) / 4.0;
        }
    }
}

void HiDPI::setScaleFactor(double factor) {
    if (factor < 0.5) factor = 0.5;
    if (factor > 4.0) factor = 4.0;
    scaleFactor_ = factor;
    dpi_ = factor * 96.0;
}

int HiDPI::scale(int value) const {
    return static_cast<int>(std::round(value * scaleFactor_));
}

double HiDPI::scaleD(double value) const {
    return value * scaleFactor_;
}

int HiDPI::unscale(int value) const {
    if (scaleFactor_ == 0) return value;
    return static_cast<int>(std::round(value / scaleFactor_));
}

double HiDPI::detectDpiFromXrdb(Display* dpy) const {
    // Read Xft.dpi from X resource database
    char* resourceStr = XResourceManagerString(dpy);
    if (!resourceStr) return 0;

    XrmInitialize();
    XrmDatabase db = XrmGetStringDatabase(resourceStr);
    if (!db) return 0;

    char* type = nullptr;
    XrmValue value;

    if (XrmGetResource(db, "Xft.dpi", "Xft.Dpi", &type, &value)) {
        if (type && strcmp(type, "String") == 0 && value.addr) {
            double dpi = std::atof(value.addr);
            XrmDestroyDatabase(db);
            if (dpi > 0) return dpi;
        }
    }

    XrmDestroyDatabase(db);
    return 0;
}

double HiDPI::detectDpiFromScreen(Display* dpy, int screen) const {
    int widthPx = DisplayWidth(dpy, screen);
    int widthMm = DisplayWidthMM(dpy, screen);

    if (widthMm <= 0) return 0;

    return (widthPx * 25.4) / widthMm;
}

double HiDPI::detectDpiFromGtkSettings(Display* dpy) const {
    // Read _XSETTINGS_SETTINGS for Gdk/WindowScalingFactor
    // or check _GTK_SCALE property on root
    Window root = DefaultRootWindow(dpy);
    Atom gtkScale = XInternAtom(dpy, "_GTK_SCALE", True);
    if (gtkScale == None) return 0;

    Atom actualType;
    int actualFormat;
    unsigned long nitems, bytesAfter;
    unsigned char* data = nullptr;

    if (XGetWindowProperty(dpy, root, gtkScale,
                           0, 1, False, XA_CARDINAL,
                           &actualType, &actualFormat,
                           &nitems, &bytesAfter, &data) == Success && data) {
        long scale = *reinterpret_cast<long*>(data);
        XFree(data);
        if (scale >= 1 && scale <= 4) {
            return scale * 96.0;
        }
    }
    if (data) XFree(data);

    return 0;
}

} // namespace motif::core
