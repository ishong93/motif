#include <motif/widgets/TextField.h>
#include <motif/core/Application.h>
#include <X11/keysym.h>
#include <X11/XKBlib.h>

namespace motif {

TextField::TextField(Widget* parent) : Text(parent) {
    rows_ = 1;
    resources_.set<int>("rows", 1);
}
TextField::~TextField() = default;

std::vector<ResourceSpec> TextField::resourceSpecs() const {
    auto specs = Text::resourceSpecs();
    // TextField inherits Text resources; rows is fixed to 1
    return specs;
}

void TextField::expose() {
    // Delegate to Text (single-line mode)
    Text::expose();
}

void TextField::handleKeyPress(unsigned int keycode, unsigned int state) {
    // Intercept Return as Activate instead of newline
    auto* dpy = Application::instance().display();
    KeySym keysym = XkbKeycodeToKeysym(dpy, keycode, 0, 0);

    if (keysym == XK_Return || keysym == XK_KP_Enter) {
        invokeCallbacks(ActivateCallback);
        return;
    }

    Text::handleKeyPress(keycode, state);
}

} // namespace motif
