#include <motif/widgets/Label.h>
#include <motif/core/Application.h>
#include <motif/core/TypeRegistry.h>

#include <X11/Xlib.h>

namespace motif {

Label::Label(Widget* parent) : Primitive(parent) {
    // Initialize resource defaults
    resources_.set<std::string>("labelString", "");
    resources_.set<int>("alignment", static_cast<int>(Alignment::Center));
    resources_.set<int>("marginWidth", 4);
    resources_.set<int>("marginHeight", 4);
    resources_.set<int>("labelType", static_cast<int>(LabelType::String));
}

Label::~Label() = default;

void Label::setText(const std::string& text) {
    text_ = text;
    labelString_ = CompoundString::create(text);
    resources_.set<std::string>("labelString", text);
    if (realized_) {
        handleExpose();
    }
}

void Label::setLabelString(const CompoundString& cs) {
    labelString_ = cs;
    // Flatten to plain text for backward compat
    text_.clear();
    for (const auto& seg : cs.segments()) {
        text_ += seg.text;
    }
    if (realized_) {
        handleExpose();
    }
}

void Label::setLabelPixmap(XPixmap pixmap, int w, int h) {
    labelPixmap_ = pixmap;
    pixmapWidth_ = w;
    pixmapHeight_ = h;
    if (realized_) {
        handleExpose();
    }
}

void Label::setAlignment(Alignment align) {
    alignment_ = align;
    resources_.set<int>("alignment", static_cast<int>(align));
    if (realized_) {
        handleExpose();
    }
}

std::vector<ResourceSpec> Label::resourceSpecs() const {
    auto specs = Primitive::resourceSpecs();
    specs.emplace_back("labelString", "LabelString", typeid(std::string), 0,
                       ResourceValue{std::string("")});
    specs.emplace_back("alignment", "Alignment", typeid(int), 0,
                       ResourceValue{static_cast<int>(Alignment::Center)});
    specs.emplace_back("marginWidth", "MarginWidth", typeid(int), 0,
                       ResourceValue{4});
    specs.emplace_back("marginHeight", "MarginHeight", typeid(int), 0,
                       ResourceValue{4});
    specs.emplace_back("labelType", "LabelType", typeid(int), 0,
                       ResourceValue{static_cast<int>(LabelType::String)});
    return specs;
}

void Label::onResourceChanged(const std::string& resourceName) {
    if (resourceName == "labelString") {
        auto val = resources_.get<std::string>("labelString", "");
        if (val != text_) {
            text_ = val;
            labelString_ = CompoundString::create(val);
        }
    } else if (resourceName == "alignment") {
        alignment_ = static_cast<Alignment>(resources_.get<int>("alignment", 1));
    } else if (resourceName == "marginWidth") {
        marginWidth_ = resources_.get<int>("marginWidth", 4);
    } else if (resourceName == "marginHeight") {
        marginHeight_ = resources_.get<int>("marginHeight", 4);
    } else if (resourceName == "labelType") {
        labelType_ = static_cast<LabelType>(resources_.get<int>("labelType", 0));
    }
    Primitive::onResourceChanged(resourceName);
}

void Label::expose() {
    if (!window_ || !gc_) return;
    auto* dpy = Application::instance().display();

    // Clear background
    XClearWindow(dpy, window_);

    // Draw shadow border
    drawShadow(true);

    if (labelType_ == LabelType::Pixmap) {
        drawPixmapLabel();
    } else {
        drawStringLabel();
    }

    // Draw highlight if focused
    if (focused_) {
        drawHighlight();
    }
}

void Label::drawPixmapLabel() {
    if (!window_ || !gc_) return;

    // Choose pixmap based on sensitivity
    XPixmap pix = sensitive_ ? labelPixmap_ : insensitivePixmap_;
    if (!pix) pix = labelPixmap_;  // fall back to normal pixmap
    if (!pix) return;

    auto* dpy = Application::instance().display();
    int border = highlightThickness_ + shadowThickness_;

    int availWidth = width_ - 2 * border - 2 * marginWidth_;
    int availHeight = height_ - 2 * border - 2 * marginHeight_;

    // Calculate position based on alignment
    int px;
    switch (alignment_) {
        case Alignment::Beginning:
            px = border + marginWidth_;
            break;
        case Alignment::End:
            px = border + marginWidth_ + availWidth - pixmapWidth_;
            break;
        case Alignment::Center:
        default:
            px = border + marginWidth_ + (availWidth - pixmapWidth_) / 2;
            break;
    }
    int py = border + marginHeight_ + (availHeight - pixmapHeight_) / 2;

    // Copy pixmap to window
    XCopyArea(dpy, pix, window_, gc_,
              0, 0,
              static_cast<unsigned int>(pixmapWidth_),
              static_cast<unsigned int>(pixmapHeight_),
              px, py);

    // If insensitive and no insensitive pixmap, stipple over the pixmap
    if (!sensitive_ && insensitivePixmap_ == 0 && labelPixmap_ != 0) {
        auto& app = Application::instance();
        XSetForeground(dpy, gc_, app.whitePixel());
        XSetFillStyle(dpy, gc_, FillStippled);
        XFillRectangle(dpy, window_, gc_, px, py, pixmapWidth_, pixmapHeight_);
        XSetFillStyle(dpy, gc_, FillSolid);
    }
}

void Label::drawStringLabel() {
    if (!window_ || !gc_ || text_.empty()) return;

    auto* dpy = Application::instance().display();
    int border = highlightThickness_ + shadowThickness_;

    // Use CompoundString rendering if available
    if (!labelString_.segments().empty()) {
        if (renderTable_.empty()) {
            renderTable_ = RenderTable::defaultTable(dpy);
        }

        auto [tw, th] = labelString_.measure(dpy, renderTable_);

        int availWidth = width_ - 2 * border - 2 * marginWidth_;
        int tx;
        switch (alignment_) {
            case Alignment::Beginning:
                tx = border + marginWidth_;
                break;
            case Alignment::End:
                tx = border + marginWidth_ + availWidth - tw;
                break;
            case Alignment::Center:
            default:
                tx = border + marginWidth_ + (availWidth - tw) / 2;
                break;
        }
        int ty = (height_ - th) / 2;

        XSetForeground(dpy, gc_, foreground_);
        labelString_.draw(dpy, window_, gc_, renderTable_, tx, ty);
    } else {
        // Fallback: plain XDrawString
        XFontStruct* font = XQueryFont(dpy, XGContextFromGC(gc_));
        if (!font) return;

        int textWidth = XTextWidth(font, text_.c_str(), static_cast<int>(text_.size()));
        int ascent = font->ascent;
        int descent = font->descent;
        int textHeight = ascent + descent;

        int availWidth = width_ - 2 * border - 2 * marginWidth_;
        int tx;
        switch (alignment_) {
            case Alignment::Beginning:
                tx = border + marginWidth_;
                break;
            case Alignment::End:
                tx = border + marginWidth_ + availWidth - textWidth;
                break;
            case Alignment::Center:
            default:
                tx = border + marginWidth_ + (availWidth - textWidth) / 2;
                break;
        }

        int ty = (height_ - textHeight) / 2 + ascent;

        XSetForeground(dpy, gc_, foreground_);
        XDrawString(dpy, window_, gc_, tx, ty,
                    text_.c_str(), static_cast<int>(text_.size()));

        XFreeFontInfo(nullptr, font, 0);
    }
}

} // namespace motif
