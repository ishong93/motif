#pragma once

/// @file motif/Builder.h
/// Fluent builder API for rapid widget tree construction.
/// Provides a more concise alternative to manual widget instantiation.

#include <motif/core/Application.h>
#include <motif/core/Shell.h>
#include <motif/core/Widget.h>
#include <motif/containers/RowColumn.h>
#include <motif/containers/Form.h>
#include <motif/widgets/Label.h>
#include <motif/widgets/PushButton.h>
#include <motif/widgets/ToggleButton.h>
#include <motif/widgets/TextField.h>
#include <motif/widgets/Text.h>
#include <motif/widgets/Scale.h>
#include <motif/widgets/Separator.h>
#include <motif/menus/MenuBar.h>
#include <motif/menus/PopupMenu.h>
#include <motif/widgets/CascadeButton.h>

#include <memory>
#include <string>
#include <vector>
#include <functional>

namespace motif {

/// Manages widget lifetime for builder-created widgets
class WidgetBag {
public:
    template<typename T, typename... Args>
    T* create(Args&&... args) {
        auto ptr = std::make_unique<T>(std::forward<Args>(args)...);
        auto* raw = ptr.get();
        widgets_.push_back(std::move(ptr));
        return raw;
    }

    void clear() { widgets_.clear(); }

private:
    std::vector<std::unique_ptr<Widget>> widgets_;
};

/// Fluent builder for creating common UI patterns
class UIBuilder {
public:
    explicit UIBuilder(Widget* parent) : parent_(parent) {}

    /// Create a labeled text field pair (label + TextField)
    struct LabeledField {
        Label* label;
        TextField* field;
    };
    LabeledField labeledField(const std::string& labelText, int columns = 20) {
        auto* lbl = bag_.create<Label>(parent_);
        lbl->setText(labelText);
        lbl->setAlignment(Label::Alignment::End);
        auto* fld = bag_.create<TextField>(parent_);
        fld->setColumns(columns);
        return {lbl, fld};
    }

    /// Create a push button with callback
    PushButton* button(const std::string& text,
                       std::function<void()> onClick = nullptr) {
        auto* btn = bag_.create<PushButton>(parent_);
        btn->setText(text);
        if (onClick) {
            btn->addCallback(PushButton::ActivateCallback,
                [cb = std::move(onClick)](Widget&, void*) { cb(); });
        }
        return btn;
    }

    /// Create a toggle button with callback
    ToggleButton* toggle(const std::string& text,
                         std::function<void(bool)> onChanged = nullptr) {
        auto* tb = bag_.create<ToggleButton>(parent_);
        tb->setText(text);
        if (onChanged) {
            tb->addCallback(ToggleButton::ValueChangedCallback,
                [cb = std::move(onChanged)](Widget& w, void*) {
                    cb(static_cast<ToggleButton&>(w).isSelected());
                });
        }
        return tb;
    }

    /// Create a label
    Label* label(const std::string& text,
                 Label::Alignment align = Label::Alignment::Beginning) {
        auto* lbl = bag_.create<Label>(parent_);
        lbl->setText(text);
        lbl->setAlignment(align);
        return lbl;
    }

    /// Create a separator
    Separator* separator() {
        return bag_.create<Separator>(parent_);
    }

    /// Create a scale (slider)
    Scale* scale(int min, int max, int value, bool showValue = true) {
        auto* s = bag_.create<Scale>(parent_);
        s->setMinimum(min);
        s->setMaximum(max);
        s->setValue(value);
        s->setShowValue(showValue);
        return s;
    }

    /// Create a multi-line text editor
    Text* textArea(int rows = 10, int cols = 40, bool editable = true) {
        auto* t = bag_.create<Text>(parent_);
        t->setRows(rows);
        t->setColumns(cols);
        t->setEditable(editable);
        t->setWordWrap(true);
        return t;
    }

    /// Set the parent for subsequent widget creation
    UIBuilder& into(Widget* newParent) {
        parent_ = newParent;
        return *this;
    }

    /// Access the widget bag (for lifetime management)
    WidgetBag& bag() { return bag_; }

private:
    Widget* parent_;
    WidgetBag bag_;
};

/// Quick-start helper: create a Shell + vertical layout and return a builder
struct QuickApp {
    Shell shell;
    RowColumn layout{&shell};
    UIBuilder builder{&layout};

    QuickApp(const std::string& title, int width = 400, int height = 300) {
        shell.setTitle(title);
        shell.resize(width, height);
        layout.setOrientation(RowColumn::Orientation::Vertical);
        layout.setSpacing(8);
        layout.setMargin(12, 12);
    }

    void run() {
        shell.realize();
        Application::instance().run();
    }
};

} // namespace motif
