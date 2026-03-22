#pragma once

#include <motif/core/Manager.h>
#include <unordered_map>

namespace motif {

class Form : public Manager {
public:
    Form() = default;
    explicit Form(Widget* parent);
    ~Form() override;

    enum class Attachment { None, Form, Widget, Position, OppositeSide, Self };

    struct Constraint {
        Attachment topAttachment    = Attachment::None;
        Attachment bottomAttachment = Attachment::None;
        Attachment leftAttachment   = Attachment::None;
        Attachment rightAttachment  = Attachment::None;

        Widget* topWidget    = nullptr;
        Widget* bottomWidget = nullptr;
        Widget* leftWidget   = nullptr;
        Widget* rightWidget  = nullptr;

        int topOffset    = 0;
        int bottomOffset = 0;
        int leftOffset   = 0;
        int rightOffset  = 0;

        // Position values (0..fractionBase)
        int topPosition    = 0;
        int bottomPosition = 0;
        int leftPosition   = 0;
        int rightPosition  = 0;
    };

    void setConstraint(Widget* child, const Constraint& constraint);
    const Constraint* constraint(Widget* child) const;
    void removeConstraint(Widget* child);

    void setFractionBase(int base) { fractionBase_ = base; }
    int fractionBase() const { return fractionBase_; }

    std::vector<ResourceSpec> resourceSpecs() const override;
    void onResourceChanged(const std::string& resourceName) override;

    void layout() override;

protected:
    int resolveLeft(Widget* child, const Constraint& c);
    int resolveRight(Widget* child, const Constraint& c);
    int resolveTop(Widget* child, const Constraint& c);
    int resolveBottom(Widget* child, const Constraint& c);

    std::unordered_map<Widget*, Constraint> constraints_;
    int fractionBase_ = 100;
};

} // namespace motif
