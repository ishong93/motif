#include <motif/containers/MainWindow.h>

namespace motif {

MainWindow::MainWindow(Widget* parent) : ScrolledWindow(parent) {}
MainWindow::~MainWindow() = default;

std::vector<ResourceSpec> MainWindow::resourceSpecs() const {
    auto specs = ScrolledWindow::resourceSpecs();
    // menuBar, commandWindow, messageWindow are set programmatically, not via resources
    return specs;
}

void MainWindow::layout() {
    int y = 0;

    if (menuBar_) {
        int mbH = 28; // default menu bar height
        menuBar_->setPosition(0, y, width_, mbH);
        y += mbH;
    }

    if (commandWindow_) {
        int cmdH = 30;
        commandWindow_->setPosition(0, y, width_, cmdH);
        y += cmdH;
    }

    int msgH = 0;
    if (messageWindow_) {
        msgH = 24;
    }

    // Work area fills remaining space
    int workH = height_ - y - msgH;
    if (workWindow_) {
        workWindow_->setPosition(0, y, width_, workH);
    }

    if (messageWindow_) {
        messageWindow_->setPosition(0, height_ - msgH, width_, msgH);
    }

    ScrolledWindow::layout();
}

} // namespace motif
