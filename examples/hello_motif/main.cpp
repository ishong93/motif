#include <motif/core/Application.h>
#include <motif/core/Shell.h>
#include <motif/containers/RowColumn.h>
#include <motif/widgets/Label.h>
#include <motif/widgets/PushButton.h>
#include <motif/widgets/ToggleButton.h>

#include <iostream>

int main() {
    using namespace motif;

    // Open X11 connection
    auto& app = Application::instance();
    if (!app.open()) {
        std::cerr << "Failed to open display\n";
        return 1;
    }

    // Build widget tree
    Shell toplevel;
    toplevel.setTitle("Hello Motif C++");
    toplevel.resize(320, 240);

    RowColumn layout(&toplevel);
    layout.setOrientation(RowColumn::Orientation::Vertical);
    layout.setSpacing(8);
    layout.setMargin(12, 12);

    Label greeting(&layout);
    greeting.setText("Hello, Motif C++!");
    greeting.setAlignment(Label::Alignment::Center);

    ToggleButton toggle(&layout);
    toggle.setText("Enable feature");
    toggle.addCallback(ToggleButton::ValueChangedCallback,
        [](Widget& w, void*) {
            auto& tb = static_cast<ToggleButton&>(w);
            std::cout << "Toggle: " << (tb.isSelected() ? "ON" : "OFF") << "\n";
        }
    );

    PushButton quitButton(&layout);
    quitButton.setText("Quit");
    quitButton.addCallback(PushButton::ActivateCallback,
        [](Widget&, void*) {
            std::cout << "Quit pressed\n";
            Application::instance().quit();
        }
    );

    // Realize and run
    toplevel.realize();
    app.run();

    return 0;
}
