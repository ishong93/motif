#include <motif/Xm.h>
#include <motif/Builder.h>
#include <motif/Version.h>

#include <iostream>

/// Demonstrates the QuickApp builder pattern — minimal boilerplate
/// to get a functional Motif application running.
int main() {
    using namespace motif;

    auto& app = Application::instance();
    if (!app.open()) return 1;

    std::cout << "Motif C++ Toolkit v" << Version::string() << "\n";

    // One-liner app setup with fluent builder
    QuickApp qa("Quick Start Demo", 350, 280);
    auto& b = qa.builder;

    b.label("Motif C++ Quick Start", Label::Alignment::Center);
    b.separator();

    auto [nameLabel, nameField] = b.labeledField("Name:", 20);
    auto [emailLabel, emailField] = b.labeledField("Email:", 20);

    b.separator();

    b.toggle("Enable notifications", [](bool on) {
        std::cout << "Notifications: " << (on ? "ON" : "OFF") << "\n";
    });

    b.scale(0, 100, 50, true);

    b.separator();

    b.button("Submit", [&] {
        std::cout << "Name:  " << nameField->value() << "\n"
                  << "Email: " << emailField->value() << "\n";
    });

    b.button("Quit", [] { Application::instance().quit(); });

    qa.run();
    return 0;
}
