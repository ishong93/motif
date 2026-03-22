#include <motif/widgets/ComboBox.h>
#include <motif/core/Application.h>
#include <X11/Xlib.h>

namespace motif {

ComboBox::ComboBox(Widget* parent) : Manager(parent) {
    resources_.set<int>("comboBoxType", static_cast<int>(Type::DropDown));
    resources_.set<bool>("editable", true);
    resources_.set<int>("visibleItemCount", 5);
}
ComboBox::~ComboBox() = default;

std::vector<ResourceSpec> ComboBox::resourceSpecs() const {
    auto specs = Manager::resourceSpecs();
    specs.emplace_back("comboBoxType", "ComboBoxType", typeid(int), 0,
                       ResourceValue{static_cast<int>(Type::DropDown)});
    specs.emplace_back("editable", "Editable", typeid(bool), 0, ResourceValue{true});
    specs.emplace_back("visibleItemCount", "VisibleItemCount", typeid(int), 0,
                       ResourceValue{5});
    return specs;
}

void ComboBox::onResourceChanged(const std::string& resourceName) {
    if (resourceName == "comboBoxType") {
        type_ = static_cast<Type>(resources_.get<int>("comboBoxType", 0));
    } else if (resourceName == "editable") {
        editable_ = resources_.get<bool>("editable", true);
    } else if (resourceName == "visibleItemCount") {
        visibleCount_ = resources_.get<int>("visibleItemCount", 5);
    }
    Manager::onResourceChanged(resourceName);
}

void ComboBox::setItems(const std::vector<std::string>& items) {
    items_ = items;
    if (realized_) handleExpose();
}

void ComboBox::setSelectedItem(const std::string& item) {
    selectedItem_ = item;
    for (size_t i = 0; i < items_.size(); ++i) {
        if (items_[i] == item) {
            selectedPos_ = static_cast<int>(i);
            break;
        }
    }
    invokeCallbacks(SelectionCallback);
    if (realized_) handleExpose();
}

void ComboBox::setSelectedPosition(int pos) {
    if (pos >= 0 && pos < static_cast<int>(items_.size())) {
        selectedPos_ = pos;
        selectedItem_ = items_[pos];
        invokeCallbacks(SelectionCallback);
        if (realized_) handleExpose();
    }
}

void ComboBox::layout() {
    // Layout is handled internally
}

void ComboBox::expose() {
    if (!window_ || !gc_) return;
    auto* dpy = Application::instance().display();
    auto& app = Application::instance();

    XClearWindow(dpy, window_);

    // Text field area
    int arrowW = 20;
    XSetForeground(dpy, gc_, app.blackPixel());
    XDrawRectangle(dpy, window_, gc_, 0, 0, width_ - arrowW - 1, height_ - 1);

    // Current value text
    if (!selectedItem_.empty()) {
        XFontStruct* font = XQueryFont(dpy, XGContextFromGC(gc_));
        if (font) {
            XSetForeground(dpy, gc_, foreground_);
            XDrawString(dpy, window_, gc_, 4, font->ascent + 2,
                        selectedItem_.c_str(), static_cast<int>(selectedItem_.size()));
            XFreeFontInfo(nullptr, font, 0);
        }
    }

    // Drop-down arrow button
    int ax = width_ - arrowW;
    XSetForeground(dpy, gc_, app.whitePixel());
    XFillRectangle(dpy, window_, gc_, ax, 0, arrowW, height_);
    XSetForeground(dpy, gc_, app.blackPixel());
    XDrawRectangle(dpy, window_, gc_, ax, 0, arrowW - 1, height_ - 1);

    // Arrow triangle
    int cx = ax + arrowW / 2;
    int cy = height_ / 2;
    XPoint pts[3] = {
        {(short)(cx - 5), (short)(cy - 3)},
        {(short)(cx + 5), (short)(cy - 3)},
        {(short)cx, (short)(cy + 3)}
    };
    XFillPolygon(dpy, window_, gc_, pts, 3, Convex, CoordModeOrigin);
}

void ComboBox::handleButtonPress(unsigned int button, int x, int /*y*/) {
    if (button != 1 || !sensitive_) return;

    int arrowW = 20;
    int arrowX = width_ - arrowW;

    if (x >= arrowX || type_ == Type::DropDownList) {
        // Toggle dropdown
        if (listVisible_) {
            hideDropdown();
        } else {
            showDropdown();
        }
    }
}

void ComboBox::showDropdown() {
    if (items_.empty()) return;
    listVisible_ = true;

    auto* dpy = Application::instance().display();
    auto& app = Application::instance();
    if (!window_ || !gc_) return;

    // Draw dropdown list below the combo box
    XFontStruct* font = XQueryFont(dpy, XGContextFromGC(gc_));
    if (!font) return;

    int lineH = font->ascent + font->descent + 4;
    int listH = std::min(visibleCount_, static_cast<int>(items_.size())) * lineH;
    int arrowW = 20;

    // Draw dropdown background
    XSetForeground(dpy, gc_, app.whitePixel());
    XFillRectangle(dpy, window_, gc_, 0, height_, width_ - arrowW, listH);
    XSetForeground(dpy, gc_, app.blackPixel());
    XDrawRectangle(dpy, window_, gc_, 0, height_, width_ - arrowW - 1, listH - 1);

    // Draw items
    for (int i = 0; i < std::min(visibleCount_, static_cast<int>(items_.size())); ++i) {
        int iy = height_ + i * lineH;
        if (i == selectedPos_) {
            XSetForeground(dpy, gc_, app.blackPixel());
            XFillRectangle(dpy, window_, gc_, 1, iy, width_ - arrowW - 2, lineH);
            XSetForeground(dpy, gc_, app.whitePixel());
        } else {
            XSetForeground(dpy, gc_, foreground_);
        }
        XDrawString(dpy, window_, gc_, 4, iy + font->ascent + 2,
                    items_[i].c_str(), static_cast<int>(items_[i].size()));
    }

    XFreeFontInfo(nullptr, font, 0);
}

void ComboBox::hideDropdown() {
    listVisible_ = false;
    if (realized_) handleExpose();
}

} // namespace motif
