#include <motif/widgets/Text.h>
#include <motif/core/Application.h>
#include <X11/Xlib.h>
#include <X11/keysym.h>
#include <algorithm>

namespace motif {

Text::Text(Widget* parent) : Primitive(parent) {
    shadowThickness_ = 2;
    resources_.set<std::string>("value", "");
    resources_.set<bool>("editable", true);
    resources_.set<int>("maxLength", 0);
    resources_.set<int>("rows", 1);
    resources_.set<int>("columns", 20);
    resources_.set<bool>("wordWrap", false);
}
Text::~Text() = default;

std::vector<ResourceSpec> Text::resourceSpecs() const {
    auto specs = Primitive::resourceSpecs();
    specs.emplace_back("value", "Value", typeid(std::string), 0,
                       ResourceValue{std::string("")});
    specs.emplace_back("editable", "Editable", typeid(bool), 0,
                       ResourceValue{true});
    specs.emplace_back("maxLength", "MaxLength", typeid(int), 0,
                       ResourceValue{0});
    specs.emplace_back("rows", "Rows", typeid(int), 0, ResourceValue{1});
    specs.emplace_back("columns", "Columns", typeid(int), 0, ResourceValue{20});
    specs.emplace_back("wordWrap", "WordWrap", typeid(bool), 0,
                       ResourceValue{false});
    return specs;
}

void Text::onResourceChanged(const std::string& resourceName) {
    if (resourceName == "value") {
        value_ = resources_.get<std::string>("value", "");
        cursorPos_ = std::min(cursorPos_, static_cast<int>(value_.size()));
    } else if (resourceName == "editable") {
        editable_ = resources_.get<bool>("editable", true);
    } else if (resourceName == "maxLength") {
        maxLength_ = resources_.get<int>("maxLength", 0);
    } else if (resourceName == "rows") {
        rows_ = resources_.get<int>("rows", 1);
    } else if (resourceName == "columns") {
        columns_ = resources_.get<int>("columns", 20);
    } else if (resourceName == "wordWrap") {
        wordWrap_ = resources_.get<bool>("wordWrap", false);
    }
    Primitive::onResourceChanged(resourceName);
}

void Text::setValue(const std::string& value) {
    value_ = value;
    cursorPos_ = std::min(cursorPos_, static_cast<int>(value_.size()));
    invokeCallbacks(ValueChangedCallback);
    if (realized_) handleExpose();
}

void Text::setCursorPosition(int pos) {
    cursorPos_ = std::clamp(pos, 0, static_cast<int>(value_.size()));
    if (realized_) handleExpose();
}

void Text::setSelection(int start, int end) {
    selStart_ = std::clamp(start, 0, static_cast<int>(value_.size()));
    selEnd_ = std::clamp(end, 0, static_cast<int>(value_.size()));
    if (realized_) handleExpose();
}

std::string Text::selectedText() const {
    if (selStart_ < 0 || selEnd_ < 0) return {};
    int s = std::min(selStart_, selEnd_);
    int e = std::max(selStart_, selEnd_);
    return value_.substr(s, e - s);
}

void Text::insert(int pos, const std::string& text) {
    pos = std::clamp(pos, 0, static_cast<int>(value_.size()));
    if (maxLength_ > 0 && static_cast<int>(value_.size() + text.size()) > maxLength_) return;
    value_.insert(pos, text);
    cursorPos_ = pos + static_cast<int>(text.size());
    invokeCallbacks(ValueChangedCallback);
    if (realized_) handleExpose();
}

void Text::replace(int from, int to, const std::string& text) {
    from = std::clamp(from, 0, static_cast<int>(value_.size()));
    to = std::clamp(to, from, static_cast<int>(value_.size()));
    value_.replace(from, to - from, text);
    cursorPos_ = from + static_cast<int>(text.size());
    invokeCallbacks(ValueChangedCallback);
    if (realized_) handleExpose();
}

void Text::expose() {
    if (!window_ || !gc_) return;
    auto* dpy = Application::instance().display();
    auto& app = Application::instance();

    XClearWindow(dpy, window_);

    // Draw border
    XSetForeground(dpy, gc_, app.blackPixel());
    XDrawRectangle(dpy, window_, gc_, 0, 0, width_ - 1, height_ - 1);

    XFontStruct* font = XQueryFont(dpy, XGContextFromGC(gc_));
    if (!font) return;
    int asc = font->ascent;
    int lineH = font->ascent + font->descent;

    // Draw text
    XSetForeground(dpy, gc_, foreground_);
    int margin = shadowThickness_ + 2;

    if (rows_ <= 1) {
        // Single line
        XDrawString(dpy, window_, gc_, margin - scrollOffset_, margin + asc,
                    value_.c_str(), static_cast<int>(value_.size()));

        // Cursor
        if (editable_) {
            std::string beforeCursor = value_.substr(0, cursorPos_);
            int cx = margin - scrollOffset_ + XTextWidth(font, beforeCursor.c_str(),
                                                          static_cast<int>(beforeCursor.size()));
            XDrawLine(dpy, window_, gc_, cx, margin, cx, margin + lineH);
        }
    } else {
        // Multi-line: split on newlines
        int y = margin + asc;
        size_t pos = 0;
        while (pos < value_.size() && y < height_) {
            size_t nl = value_.find('\n', pos);
            if (nl == std::string::npos) nl = value_.size();
            std::string line = value_.substr(pos, nl - pos);
            XDrawString(dpy, window_, gc_, margin, y,
                        line.c_str(), static_cast<int>(line.size()));
            y += lineH;
            pos = nl + 1;
        }
    }

    XFreeFontInfo(nullptr, font, 0);
}

void Text::handleKeyPress(unsigned int keycode, unsigned int state) {
    if (!editable_ && !sensitive_) return;
    auto* dpy = Application::instance().display();

    // Construct a proper XKeyEvent for XLookupString
    XKeyEvent keyEvent;
    keyEvent.type = KeyPress;
    keyEvent.display = dpy;
    keyEvent.window = window_;
    keyEvent.state = state;
    keyEvent.keycode = keycode;

    char buf[32] = {};
    KeySym keysym = NoSymbol;
    int len = XLookupString(&keyEvent, buf, sizeof(buf), &keysym, nullptr);

    switch (keysym) {
        case XK_Left:
            if (cursorPos_ > 0) cursorPos_--;
            selStart_ = selEnd_ = -1;
            break;
        case XK_Right:
            if (cursorPos_ < static_cast<int>(value_.size())) cursorPos_++;
            selStart_ = selEnd_ = -1;
            break;
        case XK_BackSpace:
            if (editable_ && cursorPos_ > 0) {
                value_.erase(cursorPos_ - 1, 1);
                cursorPos_--;
                invokeCallbacks(ValueChangedCallback);
            }
            break;
        case XK_Delete:
            if (editable_ && cursorPos_ < static_cast<int>(value_.size())) {
                value_.erase(cursorPos_, 1);
                invokeCallbacks(ValueChangedCallback);
            }
            break;
        case XK_Home:
            cursorPos_ = 0;
            break;
        case XK_End:
            cursorPos_ = static_cast<int>(value_.size());
            break;
        case XK_Return:
            if (rows_ > 1 && editable_) {
                insert(cursorPos_, "\n");
            }
            break;
        default:
            if (editable_ && len > 0 && buf[0] >= 0x20) {
                insert(cursorPos_, std::string(buf, len));
            }
            break;
    }
    if (realized_) handleExpose();
}

void Text::handleButtonPress(unsigned int button, int x, int /*y*/) {
    if (button != 1) return;

    auto* dpy = Application::instance().display();
    XFontStruct* font = XQueryFont(dpy, XGContextFromGC(gc_));
    if (!font) return;

    int margin = shadowThickness_ + 2;
    int clickX = x - margin + scrollOffset_;

    // Find character position closest to click
    int bestPos = 0;
    int bestDist = clickX;  // distance from position 0
    for (int i = 1; i <= static_cast<int>(value_.size()); ++i) {
        int charW = XTextWidth(font, value_.c_str(), i);
        int dist = std::abs(clickX - charW);
        if (dist < bestDist) {
            bestDist = dist;
            bestPos = i;
        }
    }

    cursorPos_ = bestPos;
    selStart_ = selEnd_ = -1;
    XFreeFontInfo(nullptr, font, 0);

    if (realized_) handleExpose();
}

} // namespace motif
