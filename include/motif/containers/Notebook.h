#pragma once

#include <motif/core/Manager.h>
#include <string>
#include <vector>

namespace motif {

class Notebook : public Manager {
public:
    Notebook() = default;
    explicit Notebook(Widget* parent);
    ~Notebook() override;

    static constexpr const char* PageChangedCallback = "pageChanged";

    struct Page {
        Widget* child = nullptr;
        Widget* tab   = nullptr;
        std::string label;
        int pageNumber = 0;
    };

    void addPage(Widget* child, Widget* tab, const std::string& label);
    void removePage(int pageNumber);
    void setCurrentPage(int pageNumber);
    int currentPage() const { return currentPage_; }
    int pageCount() const { return static_cast<int>(pages_.size()); }

    std::vector<ResourceSpec> resourceSpecs() const override;
    void onResourceChanged(const std::string& resourceName) override;

    void layout() override;
    void handleButtonPress(unsigned int button, int x, int y) override;

protected:
    void expose() override;

    std::vector<Page> pages_;
    int currentPage_ = 0;
    int tabHeight_ = 28;
};

} // namespace motif
