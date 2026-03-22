#include <motif/core/InputMethod.h>
#include <motif/core/Log.h>

#include <X11/Xutil.h>
#include <cstring>
#include <cstdlib>

namespace motif::core {

PreeditInfo InputMethod::emptyPreedit_;

InputMethod::~InputMethod() {
    shutdown();
}

InputMethod& InputMethod::instance() {
    static InputMethod im;
    return im;
}

bool InputMethod::initialize(Display* dpy) {
    if (xim_) return true; // already initialized
    dpy_ = dpy;

    // Set locale modifiers (empty string = use XMODIFIERS env var)
    if (!XSetLocaleModifiers("")) {
        MOTIF_LOG_WARNING << "XIM: XSetLocaleModifiers failed, trying fallback";
        XSetLocaleModifiers("@im=none");
    }

    xim_ = XOpenIM(dpy_, nullptr, nullptr, nullptr);
    if (!xim_) {
        MOTIF_LOG_INFO << "XIM: No input method available, using direct input";
        return false;
    }

    inputStyle_ = chooseInputStyle();

    MOTIF_LOG_INFO << "XIM: Input method opened successfully";
    return true;
}

void InputMethod::shutdown() {
    // Destroy all input contexts
    for (auto& [w, ic] : contexts_) {
        if (ic) XDestroyIC(ic);
    }
    contexts_.clear();
    preeditState_.clear();

    if (xim_) {
        XCloseIM(xim_);
        xim_ = nullptr;
    }
    dpy_ = nullptr;
}

XIMStyle InputMethod::chooseInputStyle() const {
    if (!xim_) return 0;

    XIMStyles* styles = nullptr;
    XGetIMValues(xim_, XNQueryInputStyle, &styles, nullptr);
    if (!styles) return 0;

    // Preference order:
    // 1. On-the-spot (preedit in widget) + status nothing
    // 2. Over-the-spot (popup near cursor)
    // 3. Root window (separate composition window)
    // 4. Nothing (basic input)

    static const XIMStyle preferred[] = {
        XIMPreeditCallbacks | XIMStatusNothing,
        XIMPreeditPosition  | XIMStatusNothing,
        XIMPreeditNothing   | XIMStatusNothing,
        XIMPreeditNone      | XIMStatusNone,
    };

    XIMStyle chosen = 0;
    for (auto pref : preferred) {
        for (unsigned short i = 0; i < styles->count_styles; ++i) {
            if (styles->supported_styles[i] == pref) {
                chosen = pref;
                goto done;
            }
        }
    }

    // Fallback: pick any available style
    if (styles->count_styles > 0) {
        chosen = styles->supported_styles[0];
    }

done:
    XFree(styles);
    return chosen;
}

XIC InputMethod::createInputContext(Window w) {
    if (!xim_) return nullptr;

    // Destroy existing IC for this window
    auto it = contexts_.find(w);
    if (it != contexts_.end()) {
        if (it->second) XDestroyIC(it->second);
        contexts_.erase(it);
    }

    XIC ic = nullptr;

    if (inputStyle_ & XIMPreeditPosition) {
        // Over-the-spot: provide spot location
        XPoint spot = {0, 0};
        XVaNestedList preeditAttr = XVaCreateNestedList(0,
            XNSpotLocation, &spot,
            nullptr);

        ic = XCreateIC(xim_,
            XNInputStyle, inputStyle_,
            XNClientWindow, w,
            XNFocusWindow, w,
            XNPreeditAttributes, preeditAttr,
            nullptr);

        XFree(preeditAttr);
    } else {
        ic = XCreateIC(xim_,
            XNInputStyle, inputStyle_,
            XNClientWindow, w,
            XNFocusWindow, w,
            nullptr);
    }

    if (ic) {
        contexts_[w] = ic;

        // Request key events go through IC
        long filterMask = 0;
        XGetICValues(ic, XNFilterEvents, &filterMask, nullptr);
        if (filterMask) {
            XWindowAttributes attrs;
            XGetWindowAttributes(dpy_, w, &attrs);
            XSelectInput(dpy_, w, attrs.your_event_mask | filterMask);
        }
    } else {
        MOTIF_LOG_WARNING << "XIM: Failed to create input context for window " << w;
    }

    return ic;
}

void InputMethod::destroyInputContext(Window w) {
    auto it = contexts_.find(w);
    if (it != contexts_.end()) {
        if (it->second) XDestroyIC(it->second);
        contexts_.erase(it);
    }
    preeditState_.erase(w);
}

XIC InputMethod::getInputContext(Window w) const {
    auto it = contexts_.find(w);
    return (it != contexts_.end()) ? it->second : nullptr;
}

int InputMethod::lookupString(XIC ic, XKeyPressedEvent& event,
                              std::string& result, KeySym& keysym) {
    if (!ic) {
        // Fallback: use basic Xlib lookup
        char buf[64];
        int len = XLookupString(&event, buf, sizeof(buf), &keysym, nullptr);
        if (len > 0) {
            result.assign(buf, len);
        } else {
            result.clear();
        }
        return len;
    }

    char buf[256];
    Status status;
    int len = Xutf8LookupString(ic, &event, buf, sizeof(buf) - 1,
                                 &keysym, &status);

    switch (status) {
        case XLookupChars:
        case XLookupBoth:
            result.assign(buf, len);
            break;
        case XLookupKeySym:
            result.clear();
            break;
        case XLookupNone:
            result.clear();
            keysym = NoSymbol;
            break;
        case XBufferOverflow: {
            // Unlikely but handle: allocate larger buffer
            std::vector<char> bigBuf(len + 1);
            len = Xutf8LookupString(ic, &event, bigBuf.data(),
                                     static_cast<int>(bigBuf.size()),
                                     &keysym, &status);
            if (status == XLookupChars || status == XLookupBoth) {
                result.assign(bigBuf.data(), len);
            } else {
                result.clear();
            }
            break;
        }
        default:
            result.clear();
            break;
    }

    return static_cast<int>(result.size());
}

bool InputMethod::filterEvent(XEvent& event) {
    if (!xim_) return false;
    return XFilterEvent(&event, None) == True;
}

const PreeditInfo& InputMethod::preeditInfo(Window w) const {
    auto it = preeditState_.find(w);
    return (it != preeditState_.end()) ? it->second : emptyPreedit_;
}

void InputMethod::onPreeditChanged(PreeditCallback callback) {
    preeditCallbacks_.push_back(std::move(callback));
}

void InputMethod::setFocus(XIC ic) {
    if (ic) XSetICFocus(ic);
}

void InputMethod::unsetFocus(XIC ic) {
    if (ic) XUnsetICFocus(ic);
}

} // namespace motif::core
