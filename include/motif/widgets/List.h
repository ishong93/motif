#pragma once

#include <motif/core/Primitive.h>
#include <string>
#include <vector>

namespace motif {

class List : public Primitive {
public:
    List() = default;
    explicit List(Widget* parent);
    ~List() override;

    static constexpr const char* SingleSelectionCallback   = "singleSelection";
    static constexpr const char* MultipleSelectionCallback = "multipleSelection";
    static constexpr const char* BrowseSelectionCallback   = "browseSelection";
    static constexpr const char* DefaultActionCallback     = "defaultAction";

    enum class SelectionPolicy { Single, Multiple, Extended, Browse };

    void setSelectionPolicy(SelectionPolicy policy) { policy_ = policy; }
    SelectionPolicy selectionPolicy() const { return policy_; }

    void setItems(const std::vector<std::string>& items);
    const std::vector<std::string>& items() const { return items_; }

    void addItem(const std::string& item, int position = -1);
    void removeItem(int position);
    void removeAllItems();

    void selectItem(int position);
    void deselectItem(int position);
    void deselectAll();
    std::vector<int> selectedPositions() const;

    void setVisibleItemCount(int count) { visibleCount_ = count; }
    int visibleItemCount() const { return visibleCount_; }

    void setTopItemPosition(int pos) { topItem_ = pos; }
    int topItemPosition() const { return topItem_; }

    std::vector<ResourceSpec> resourceSpecs() const override;
    void onResourceChanged(const std::string& resourceName) override;

    void handleButtonPress(unsigned int button, int x, int y) override;
    void handleKeyPress(unsigned int keycode, unsigned int state) override;

protected:
    int itemAtY(int y) const;
    void expose() override;

    std::vector<std::string> items_;
    std::vector<bool> selected_;
    SelectionPolicy policy_ = SelectionPolicy::Browse;
    int visibleCount_ = 8;
    int topItem_ = 0;
};

} // namespace motif
