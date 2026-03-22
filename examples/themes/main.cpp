#include <motif/core/Application.h>
#include <motif/core/Shell.h>
#include <motif/core/Theme.h>
#include <motif/containers/RowColumn.h>
#include <motif/widgets/Label.h>
#include <motif/widgets/PushButton.h>
#include <motif/widgets/Scale.h>
#include <motif/widgets/ToggleButton.h>

#include <iostream>

/// Demonstrates the Theme/Style system with live theme switching.
int main() {
    using namespace motif;
    using namespace motif::core;

    auto& app = Application::instance();
    if (!app.open()) return 1;

    // Realize the built-in themes for the current display
    auto& mgr = ThemeManager::instance();
    mgr.activeTheme().realize(app.display(), app.screen());

    Shell toplevel;
    toplevel.setTitle("Theme Switcher");
    toplevel.resize(350, 300);

    RowColumn layout(&toplevel);
    layout.setOrientation(RowColumn::Orientation::Vertical);
    layout.setSpacing(8);
    layout.setMargin(16, 16);

    Label title(&layout);
    title.setText("Theme Demo");
    title.setAlignment(Label::Alignment::Center);

    // Theme switch buttons
    Label themeLabel(&layout);
    themeLabel.setText("Select theme:");

    PushButton classicBtn(&layout);
    classicBtn.setText("Motif Classic");
    classicBtn.addCallback(PushButton::ActivateCallback,
        [&](Widget&, void*) {
            mgr.setActiveTheme("MotifClassic");
            mgr.activeTheme().realize(app.display(), app.screen());
            std::cout << "Switched to: MotifClassic\n";
        });

    PushButton modernBtn(&layout);
    modernBtn.setText("Motif Modern");
    modernBtn.addCallback(PushButton::ActivateCallback,
        [&](Widget&, void*) {
            mgr.setActiveTheme("MotifModern");
            mgr.activeTheme().realize(app.display(), app.screen());
            std::cout << "Switched to: MotifModern\n";
        });

    PushButton darkBtn(&layout);
    darkBtn.setText("Motif Dark");
    darkBtn.addCallback(PushButton::ActivateCallback,
        [&](Widget&, void*) {
            mgr.setActiveTheme("MotifDark");
            mgr.activeTheme().realize(app.display(), app.screen());
            std::cout << "Switched to: MotifDark\n";
        });

    PushButton hcBtn(&layout);
    hcBtn.setText("High Contrast");
    hcBtn.addCallback(PushButton::ActivateCallback,
        [&](Widget&, void*) {
            mgr.setActiveTheme("MotifHighContrast");
            mgr.activeTheme().realize(app.display(), app.screen());
            std::cout << "Switched to: MotifHighContrast\n";
        });

    // Load custom theme from string
    PushButton customBtn(&layout);
    customBtn.setText("Custom (Blue Accent)");
    customBtn.addCallback(PushButton::ActivateCallback,
        [&](Widget&, void*) {
            Theme custom("Custom");
            custom.loadFromString(
                "*background: #F0F0F0\n"
                "*foreground: #333333\n"
                "*selectColor: #2196F3\n"
                "*highlightColor: #2196F3\n"
                "*fontList: Sans-11\n"
                "*XmPushButton.background: #E3F2FD\n"
            );
            custom.realize(app.display(), app.screen());
            mgr.setActiveTheme(std::move(custom));
            std::cout << "Switched to: Custom\n";
        });

    // Sample widgets to see theme effects
    ToggleButton toggle(&layout);
    toggle.setText("Sample toggle");

    Scale scale(&layout);
    scale.setMinimum(0);
    scale.setMaximum(100);
    scale.setValue(42);
    scale.setShowValue(true);

    PushButton quitBtn(&layout);
    quitBtn.setText("Quit");
    quitBtn.addCallback(PushButton::ActivateCallback,
        [](Widget&, void*) { Application::instance().quit(); });

    // Print available themes
    std::cout << "Available themes:";
    for (const auto& name : mgr.themeNames()) {
        std::cout << " " << name;
    }
    std::cout << "\n";

    toplevel.realize();
    app.run();
    return 0;
}
