#include <motif/input/Traversal.h>
#include <motif/core/Application.h>
#include <X11/Xlib.h>

namespace motif {
namespace traversal {

bool isTraversable(Widget* w) {
    if (!w) return false;
    return w->isRealized() && w->isVisible() && w->isSensitive();
}

Widget* findInitialFocus(Widget* root) {
    if (!root) return nullptr;
    auto& children = root->children();
    for (auto* child : children) {
        if (isTraversable(child)) return child;
        Widget* found = findInitialFocus(child);
        if (found) return found;
    }
    return nullptr;
}

Widget* traverse(Widget* current, Direction direction) {
    if (!current || !current->parent()) return current;

    auto& siblings = current->parent()->children();
    int n = static_cast<int>(siblings.size());
    if (n == 0) return current;

    int idx = -1;
    for (int i = 0; i < n; ++i) {
        if (siblings[i] == current) { idx = i; break; }
    }
    if (idx < 0) return current;

    switch (direction) {
        case Direction::Next:
        case Direction::Right:
        case Direction::Down: {
            for (int i = 1; i < n; ++i) {
                int ni = (idx + i) % n;
                if (isTraversable(siblings[ni])) return siblings[ni];
            }
            break;
        }
        case Direction::Previous:
        case Direction::Left:
        case Direction::Up: {
            for (int i = 1; i < n; ++i) {
                int ni = (idx - i + n) % n;
                if (isTraversable(siblings[ni])) return siblings[ni];
            }
            break;
        }
        case Direction::Home:
            for (int i = 0; i < n; ++i) {
                if (isTraversable(siblings[i])) return siblings[i];
            }
            break;
        case Direction::End:
            for (int i = n - 1; i >= 0; --i) {
                if (isTraversable(siblings[i])) return siblings[i];
            }
            break;
    }
    return current;
}

void setFocus(Widget* widget) {
    if (!widget || !widget->isRealized()) return;
    auto* dpy = Application::instance().display();
    if (dpy && widget->window()) {
        XSetInputFocus(dpy, widget->window(), RevertToParent, CurrentTime);
    }
}

} // namespace traversal
} // namespace motif
