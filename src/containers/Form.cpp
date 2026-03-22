#include <motif/containers/Form.h>
#include <algorithm>
#include <unordered_set>

namespace motif {

Form::Form(Widget* parent) : Manager(parent) {
    resources_.set<int>("fractionBase", 100);
}
Form::~Form() = default;

std::vector<ResourceSpec> Form::resourceSpecs() const {
    auto specs = Manager::resourceSpecs();
    specs.emplace_back("fractionBase", "FractionBase", typeid(int), 0,
                       ResourceValue{100});
    return specs;
}

void Form::onResourceChanged(const std::string& resourceName) {
    if (resourceName == "fractionBase") {
        fractionBase_ = resources_.get<int>("fractionBase", 100);
        layout();
    }
    Manager::onResourceChanged(resourceName);
}

void Form::setConstraint(Widget* child, const Constraint& constraint) {
    constraints_[child] = constraint;
    if (realized_) layout();
}

const Form::Constraint* Form::constraint(Widget* child) const {
    auto it = constraints_.find(child);
    return it != constraints_.end() ? &it->second : nullptr;
}

void Form::removeConstraint(Widget* child) {
    constraints_.erase(child);
}

// ── Constraint Resolver ─────────────────────────────────────

int Form::resolveLeft(Widget* child, const Constraint& c) {
    switch (c.leftAttachment) {
        case Attachment::Form:
            return c.leftOffset;
        case Attachment::Position:
            return width_ * c.leftPosition / fractionBase_ + c.leftOffset;
        case Attachment::Widget:
            if (c.leftWidget) {
                return c.leftWidget->x() + c.leftWidget->width() + c.leftOffset;
            }
            return c.leftOffset;
        case Attachment::OppositeSide:
            if (c.leftWidget) {
                return c.leftWidget->x() + c.leftOffset;
            }
            return c.leftOffset;
        case Attachment::Self:
            return child->x();
        case Attachment::None:
        default:
            return -1; // unresolved
    }
}

int Form::resolveRight(Widget* child, const Constraint& c) {
    switch (c.rightAttachment) {
        case Attachment::Form:
            return width_ - c.rightOffset;
        case Attachment::Position:
            return width_ * c.rightPosition / fractionBase_ - c.rightOffset;
        case Attachment::Widget:
            if (c.rightWidget) {
                return c.rightWidget->x() - c.rightOffset;
            }
            return width_ - c.rightOffset;
        case Attachment::OppositeSide:
            if (c.rightWidget) {
                return c.rightWidget->x() + c.rightWidget->width() - c.rightOffset;
            }
            return width_ - c.rightOffset;
        case Attachment::Self:
            return child->x() + child->width();
        case Attachment::None:
        default:
            return -1;
    }
}

int Form::resolveTop(Widget* child, const Constraint& c) {
    switch (c.topAttachment) {
        case Attachment::Form:
            return c.topOffset;
        case Attachment::Position:
            return height_ * c.topPosition / fractionBase_ + c.topOffset;
        case Attachment::Widget:
            if (c.topWidget) {
                return c.topWidget->y() + c.topWidget->height() + c.topOffset;
            }
            return c.topOffset;
        case Attachment::OppositeSide:
            if (c.topWidget) {
                return c.topWidget->y() + c.topOffset;
            }
            return c.topOffset;
        case Attachment::Self:
            return child->y();
        case Attachment::None:
        default:
            return -1;
    }
}

int Form::resolveBottom(Widget* child, const Constraint& c) {
    switch (c.bottomAttachment) {
        case Attachment::Form:
            return height_ - c.bottomOffset;
        case Attachment::Position:
            return height_ * c.bottomPosition / fractionBase_ - c.bottomOffset;
        case Attachment::Widget:
            if (c.bottomWidget) {
                return c.bottomWidget->y() - c.bottomOffset;
            }
            return height_ - c.bottomOffset;
        case Attachment::OppositeSide:
            if (c.bottomWidget) {
                return c.bottomWidget->y() + c.bottomWidget->height() - c.bottomOffset;
            }
            return height_ - c.bottomOffset;
        case Attachment::Self:
            return child->y() + child->height();
        case Attachment::None:
        default:
            return -1;
    }
}

void Form::layout() {
    if (children_.empty() || width_ <= 0 || height_ <= 0) return;

    // Multi-pass solver: resolve constraints iteratively.
    // Widgets attached to Form edges resolve first; widgets attached to
    // other widgets resolve once their dependency is placed.

    constexpr int kMaxPasses = 8;
    constexpr int kDefaultSize = 30;

    // Initialize children that have no constraints to a default position
    for (auto* child : children_) {
        if (constraints_.find(child) == constraints_.end()) {
            // No constraints: stack at origin with default size
            if (child->width() <= 0) child->resize(kDefaultSize, kDefaultSize);
        }
    }

    std::unordered_set<Widget*> resolved;

    for (int pass = 0; pass < kMaxPasses; ++pass) {
        bool progress = false;

        for (auto* child : children_) {
            auto it = constraints_.find(child);
            if (it == constraints_.end()) continue;

            const Constraint& c = it->second;

            // Check if dependencies are resolved
            auto depReady = [&](Widget* dep) {
                return !dep || resolved.count(dep) > 0;
            };

            if (!depReady(c.leftWidget) || !depReady(c.rightWidget) ||
                !depReady(c.topWidget) || !depReady(c.bottomWidget)) {
                continue; // dependencies not yet placed
            }

            int left   = resolveLeft(child, c);
            int right  = resolveRight(child, c);
            int top    = resolveTop(child, c);
            int bottom = resolveBottom(child, c);

            int cx = child->x();
            int cy = child->y();
            int cw = child->width();
            int ch = child->height();

            // Horizontal
            if (left >= 0 && right >= 0) {
                cx = left;
                cw = std::max(1, right - left);
            } else if (left >= 0) {
                cx = left;
                if (cw <= 0) cw = kDefaultSize;
            } else if (right >= 0) {
                if (cw <= 0) cw = kDefaultSize;
                cx = right - cw;
            }

            // Vertical
            if (top >= 0 && bottom >= 0) {
                cy = top;
                ch = std::max(1, bottom - top);
            } else if (top >= 0) {
                cy = top;
                if (ch <= 0) ch = kDefaultSize;
            } else if (bottom >= 0) {
                if (ch <= 0) ch = kDefaultSize;
                cy = bottom - ch;
            }

            if (cw <= 0) cw = kDefaultSize;
            if (ch <= 0) ch = kDefaultSize;

            child->setPosition(cx, cy, cw, ch);

            if (resolved.insert(child).second) {
                progress = true;
            }
        }

        if (!progress) break; // no new widgets resolved this pass
        if (resolved.size() == constraints_.size()) break; // all done
    }
}

} // namespace motif
