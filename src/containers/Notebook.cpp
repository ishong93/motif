#include <motif/containers/Notebook.h>
#include <motif/core/Application.h>
#include <X11/Xlib.h>

namespace motif {

Notebook::Notebook(Widget* parent) : Manager(parent) {
    resources_.set<int>("currentPage", 0);
    resources_.set<int>("tabHeight", 28);
}
Notebook::~Notebook() = default;

std::vector<ResourceSpec> Notebook::resourceSpecs() const {
    auto specs = Manager::resourceSpecs();
    specs.emplace_back("currentPage", "CurrentPage", typeid(int), 0, ResourceValue{0});
    specs.emplace_back("tabHeight", "TabHeight", typeid(int), 0, ResourceValue{28});
    return specs;
}

void Notebook::onResourceChanged(const std::string& resourceName) {
    if (resourceName == "currentPage") {
        int page = resources_.get<int>("currentPage", 0);
        if (page != currentPage_) setCurrentPage(page);
        return;
    } else if (resourceName == "tabHeight") {
        tabHeight_ = resources_.get<int>("tabHeight", 28);
    }
    Manager::onResourceChanged(resourceName);
}

void Notebook::addPage(Widget* child, Widget* tab, const std::string& label) {
    Page p;
    p.child = child;
    p.tab = tab;
    p.label = label;
    p.pageNumber = static_cast<int>(pages_.size());
    pages_.push_back(p);
    if (realized_) handleExpose();
}

void Notebook::removePage(int pageNumber) {
    if (pageNumber >= 0 && pageNumber < static_cast<int>(pages_.size())) {
        pages_.erase(pages_.begin() + pageNumber);
        if (currentPage_ >= static_cast<int>(pages_.size())) {
            currentPage_ = std::max(0, static_cast<int>(pages_.size()) - 1);
        }
    }
}

void Notebook::setCurrentPage(int pageNumber) {
    if (pageNumber >= 0 && pageNumber < static_cast<int>(pages_.size())) {
        currentPage_ = pageNumber;
        invokeCallbacks(PageChangedCallback);
        layout();
        if (realized_) handleExpose();
    }
}

void Notebook::handleButtonPress(unsigned int button, int x, int y) {
    if (button != 1 || pages_.empty()) return;

    // Check if click is in tab area
    if (y < tabHeight_) {
        int tabW = width_ / static_cast<int>(pages_.size());
        int tabIndex = x / tabW;
        if (tabIndex >= 0 && tabIndex < static_cast<int>(pages_.size())) {
            setCurrentPage(tabIndex);
        }
        return;
    }
    Manager::handleButtonPress(button, x, y);
}

void Notebook::layout() {
    // Tabs at top, content below
    for (size_t i = 0; i < pages_.size(); ++i) {
        if (pages_[i].child) {
            if (static_cast<int>(i) == currentPage_) {
                pages_[i].child->setPosition(0, tabHeight_, width_, height_ - tabHeight_);
                pages_[i].child->show();
            } else {
                pages_[i].child->hide();
            }
        }
    }
}

void Notebook::expose() {
    if (!window_ || !gc_) return;
    auto* dpy = Application::instance().display();
    auto& app = Application::instance();

    XClearWindow(dpy, window_);

    // Draw tabs
    int tabW = pages_.empty() ? 0 : (width_ / static_cast<int>(pages_.size()));

    for (size_t i = 0; i < pages_.size(); ++i) {
        int tx = static_cast<int>(i) * tabW;
        bool active = (static_cast<int>(i) == currentPage_);

        if (active) {
            XSetForeground(dpy, gc_, app.whitePixel());
        } else {
            XSetForeground(dpy, gc_, app.blackPixel());
        }
        XFillRectangle(dpy, window_, gc_, tx, 0, tabW, tabHeight_);

        XSetForeground(dpy, gc_, app.blackPixel());
        XDrawRectangle(dpy, window_, gc_, tx, 0, tabW - 1, tabHeight_ - 1);

        // Tab label
        XSetForeground(dpy, gc_, active ? app.blackPixel() : app.whitePixel());
        XFontStruct* font = XQueryFont(dpy, XGContextFromGC(gc_));
        if (font) {
            int tw = XTextWidth(font, pages_[i].label.c_str(),
                                static_cast<int>(pages_[i].label.size()));
            XDrawString(dpy, window_, gc_,
                        tx + (tabW - tw) / 2,
                        font->ascent + 4,
                        pages_[i].label.c_str(),
                        static_cast<int>(pages_[i].label.size()));
            XFreeFontInfo(nullptr, font, 0);
        }
    }

    // Border around content area
    XSetForeground(dpy, gc_, app.blackPixel());
    XDrawRectangle(dpy, window_, gc_, 0, tabHeight_, width_ - 1, height_ - tabHeight_ - 1);
}

} // namespace motif
