#include <motif/widgets/List.h>
#include <motif/core/Application.h>
#include <X11/Xlib.h>
#include <X11/keysym.h>
#include <algorithm>

namespace motif {

List::List(Widget* parent) : Primitive(parent) {
    shadowThickness_ = 2;
    resources_.set<int>("selectionPolicy", static_cast<int>(SelectionPolicy::Browse));
    resources_.set<int>("visibleItemCount", 8);
    resources_.set<int>("topItemPosition", 0);
}
List::~List() = default;

std::vector<ResourceSpec> List::resourceSpecs() const {
    auto specs = Primitive::resourceSpecs();
    specs.emplace_back("selectionPolicy", "SelectionPolicy", typeid(int), 0,
                       ResourceValue{static_cast<int>(SelectionPolicy::Browse)});
    specs.emplace_back("visibleItemCount", "VisibleItemCount", typeid(int), 0,
                       ResourceValue{8});
    specs.emplace_back("topItemPosition", "TopItemPosition", typeid(int), 0,
                       ResourceValue{0});
    return specs;
}

void List::onResourceChanged(const std::string& resourceName) {
    if (resourceName == "selectionPolicy") {
        policy_ = static_cast<SelectionPolicy>(resources_.get<int>("selectionPolicy", 3));
    } else if (resourceName == "visibleItemCount") {
        visibleCount_ = resources_.get<int>("visibleItemCount", 8);
    } else if (resourceName == "topItemPosition") {
        topItem_ = resources_.get<int>("topItemPosition", 0);
    }
    Primitive::onResourceChanged(resourceName);
}

void List::setItems(const std::vector<std::string>& items) {
    items_ = items;
    selected_.assign(items_.size(), false);
    if (realized_) handleExpose();
}

void List::addItem(const std::string& item, int position) {
    if (position < 0 || position >= static_cast<int>(items_.size())) {
        items_.push_back(item);
        selected_.push_back(false);
    } else {
        items_.insert(items_.begin() + position, item);
        selected_.insert(selected_.begin() + position, false);
    }
    if (realized_) handleExpose();
}

void List::removeItem(int position) {
    if (position >= 0 && position < static_cast<int>(items_.size())) {
        items_.erase(items_.begin() + position);
        selected_.erase(selected_.begin() + position);
        if (realized_) handleExpose();
    }
}

void List::removeAllItems() {
    items_.clear();
    selected_.clear();
    if (realized_) handleExpose();
}

void List::selectItem(int position) {
    if (position < 0 || position >= static_cast<int>(selected_.size())) return;
    if (policy_ == SelectionPolicy::Single || policy_ == SelectionPolicy::Browse) {
        deselectAll();
    }
    selected_[position] = true;
    if (realized_) handleExpose();
}

void List::deselectItem(int position) {
    if (position >= 0 && position < static_cast<int>(selected_.size())) {
        selected_[position] = false;
        if (realized_) handleExpose();
    }
}

void List::deselectAll() {
    std::fill(selected_.begin(), selected_.end(), false);
}

std::vector<int> List::selectedPositions() const {
    std::vector<int> result;
    for (size_t i = 0; i < selected_.size(); ++i) {
        if (selected_[i]) result.push_back(static_cast<int>(i));
    }
    return result;
}

void List::expose() {
    if (!window_ || !gc_) return;
    auto* dpy = Application::instance().display();
    auto& app = Application::instance();

    XClearWindow(dpy, window_);
    XSetForeground(dpy, gc_, app.blackPixel());
    XDrawRectangle(dpy, window_, gc_, 0, 0, width_ - 1, height_ - 1);

    XFontStruct* font = XQueryFont(dpy, XGContextFromGC(gc_));
    if (!font) return;
    int lineH = font->ascent + font->descent + 2;
    int margin = 4;

    int end = std::min(topItem_ + visibleCount_, static_cast<int>(items_.size()));
    for (int i = topItem_; i < end; ++i) {
        int y = margin + (i - topItem_) * lineH;

        if (selected_[i]) {
            XSetForeground(dpy, gc_, app.blackPixel());
            XFillRectangle(dpy, window_, gc_, 1, y, width_ - 2, lineH);
            XSetForeground(dpy, gc_, app.whitePixel());
        } else {
            XSetForeground(dpy, gc_, foreground_);
        }

        XDrawString(dpy, window_, gc_, margin, y + font->ascent,
                    items_[i].c_str(), static_cast<int>(items_[i].size()));
    }

    XFreeFontInfo(nullptr, font, 0);
}

int List::itemAtY(int y) const {
    if (!window_ || !gc_ || items_.empty()) return -1;
    auto* dpy = Application::instance().display();
    XFontStruct* font = XQueryFont(dpy, XGContextFromGC(gc_));
    if (!font) return -1;
    int lineH = font->ascent + font->descent + 2;
    XFreeFontInfo(nullptr, font, 0);

    int index = topItem_ + (y - 4) / lineH;
    if (index < 0 || index >= static_cast<int>(items_.size())) return -1;
    return index;
}

void List::handleButtonPress(unsigned int button, int /*x*/, int y) {
    if (button != 1 || !sensitive_) return;

    int index = itemAtY(y);
    if (index < 0) return;

    if (policy_ == SelectionPolicy::Multiple) {
        // Toggle selection in multiple mode
        if (selected_[index]) {
            deselectItem(index);
        } else {
            selectItem(index);
        }
    } else {
        selectItem(index);
    }

    switch (policy_) {
        case SelectionPolicy::Single:
            invokeCallbacks(SingleSelectionCallback);
            break;
        case SelectionPolicy::Browse:
            invokeCallbacks(BrowseSelectionCallback);
            break;
        case SelectionPolicy::Multiple:
        case SelectionPolicy::Extended:
            invokeCallbacks(MultipleSelectionCallback);
            break;
    }
}

void List::handleKeyPress(unsigned int keycode, unsigned int state) {
    if (!sensitive_ || items_.empty()) return;
    auto* dpy = Application::instance().display();

    XKeyEvent keyEvent;
    keyEvent.type = KeyPress;
    keyEvent.display = dpy;
    keyEvent.window = window_;
    keyEvent.state = state;
    keyEvent.keycode = keycode;

    char buf[8];
    KeySym keysym = NoSymbol;
    XLookupString(&keyEvent, buf, sizeof(buf), &keysym, nullptr);

    // Find current focused item (first selected, or 0)
    int current = 0;
    auto sel = selectedPositions();
    if (!sel.empty()) current = sel.back();

    switch (keysym) {
        case XK_Up:
            if (current > 0) {
                selectItem(current - 1);
                // Scroll if needed
                if (current - 1 < topItem_) topItem_ = current - 1;
                invokeCallbacks(BrowseSelectionCallback);
            }
            break;
        case XK_Down:
            if (current < static_cast<int>(items_.size()) - 1) {
                selectItem(current + 1);
                if (current + 1 >= topItem_ + visibleCount_) {
                    topItem_ = current + 1 - visibleCount_ + 1;
                }
                invokeCallbacks(BrowseSelectionCallback);
            }
            break;
        case XK_Home:
            selectItem(0);
            topItem_ = 0;
            invokeCallbacks(BrowseSelectionCallback);
            break;
        case XK_End:
            selectItem(static_cast<int>(items_.size()) - 1);
            topItem_ = std::max(0, static_cast<int>(items_.size()) - visibleCount_);
            invokeCallbacks(BrowseSelectionCallback);
            break;
        case XK_Page_Up: {
            int newPos = std::max(0, current - visibleCount_);
            selectItem(newPos);
            topItem_ = std::max(0, topItem_ - visibleCount_);
            invokeCallbacks(BrowseSelectionCallback);
            break;
        }
        case XK_Page_Down: {
            int newPos = std::min(static_cast<int>(items_.size()) - 1, current + visibleCount_);
            selectItem(newPos);
            topItem_ = std::min(static_cast<int>(items_.size()) - visibleCount_,
                                topItem_ + visibleCount_);
            if (topItem_ < 0) topItem_ = 0;
            invokeCallbacks(BrowseSelectionCallback);
            break;
        }
        case XK_Return:
        case XK_KP_Enter:
            invokeCallbacks(DefaultActionCallback);
            break;
        case XK_space:
            if (policy_ == SelectionPolicy::Multiple) {
                if (selected_[current]) {
                    deselectItem(current);
                } else {
                    selectItem(current);
                }
                invokeCallbacks(MultipleSelectionCallback);
            }
            break;
        default:
            break;
    }
    if (realized_) handleExpose();
}

} // namespace motif
