#pragma once

#include <motif/core/Primitive.h>
#include <motif/core/CompoundString.h>
#include <string>

// Forward declare X11 Pixmap type
using XPixmap = unsigned long;

namespace motif {

class Label : public Primitive {
public:
    Label() = default;
    explicit Label(Widget* parent);
    ~Label() override;

    // Simple text (creates single-segment CompoundString internally)
    void setText(const std::string& text);
    const std::string& text() const { return text_; }

    // CompoundString for multi-font/multi-direction labels
    void setLabelString(const CompoundString& cs);
    const CompoundString& labelString() const { return labelString_; }

    enum class LabelType { String, Pixmap };
    void setLabelType(LabelType t) { labelType_ = t; }
    LabelType labelType() const { return labelType_; }

    // Pixmap label support
    void setLabelPixmap(XPixmap pixmap, int w, int h);
    void setInsensitivePixmap(XPixmap pixmap) { insensitivePixmap_ = pixmap; }

    enum class Alignment { Beginning, Center, End };
    void setAlignment(Alignment align);
    Alignment alignment() const { return alignment_; }

    void setMargin(int w, int h) { marginWidth_ = w; marginHeight_ = h; }

    // Resource system
    std::vector<ResourceSpec> resourceSpecs() const override;
    void onResourceChanged(const std::string& resourceName) override;

protected:
    void expose() override;
    void drawPixmapLabel();
    void drawStringLabel();

    std::string text_;
    CompoundString labelString_;
    LabelType labelType_ = LabelType::String;
    Alignment alignment_ = Alignment::Center;
    int marginWidth_ = 4;
    int marginHeight_ = 4;
    RenderTable renderTable_;

    // Pixmap label
    XPixmap labelPixmap_ = 0;
    XPixmap insensitivePixmap_ = 0;
    int pixmapWidth_ = 0;
    int pixmapHeight_ = 0;
};

} // namespace motif
