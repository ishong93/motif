#include <motif/core/Error.h>
#include <motif/core/Log.h>

#include <X11/Xlib.h>

namespace motif {

static int handleXError(Display* dpy, XErrorEvent* event) {
    X11ErrorInfo info;
    info.errorCode = event->error_code;
    info.requestCode = event->request_code;
    info.minorCode = event->minor_code;
    info.resourceId = event->resourceid;

    char buf[256];
    XGetErrorText(dpy, event->error_code, buf, sizeof(buf));
    info.description = buf;

    MOTIF_LOG(LogLevel::Error, "X11")
        << "X error: " << info.description
        << " (request=" << static_cast<int>(info.requestCode)
        << ", minor=" << static_cast<int>(info.minorCode)
        << ", resource=0x" << std::hex << info.resourceId << ")";

    X11ErrorHandler::instance().recordError(info);
    return 0;
}

X11ErrorHandler& X11ErrorHandler::instance() {
    static X11ErrorHandler handler;
    return handler;
}

void X11ErrorHandler::install() {
    XSetErrorHandler(handleXError);
}

void X11ErrorHandler::setCallback(ErrorCallback cb) {
    callback_ = std::move(cb);
}

void X11ErrorHandler::recordError(const X11ErrorInfo& info) {
    errorOccurred_ = true;
    lastError_ = info;
    if (callback_) {
        callback_(info);
    }
}

} // namespace motif
