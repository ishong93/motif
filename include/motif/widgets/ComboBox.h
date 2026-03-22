#pragma once

#include <motif/core/Manager.h>
#include <string>
#include <vector>

namespace motif {

class TextField;
class List;
class ArrowButton;

class ComboBox : public Manager {
public:
    ComboBox() = default;
    explicit ComboBox(Widget* parent);
    ~ComboBox() override;

    static constexpr const char* SelectionCallback = "selection";

    enum class Type { DropDown, DropDownList, Static };

    void setComboBoxType(Type type) { type_ = type; }
    Type comboBoxType() const { return type_; }

    void setItems(const std::vector<std::string>& items);
    const std::vector<std::string>& items() const { return items_; }

    void setSelectedItem(const std::string& item);
    std::string selectedItem() const { return selectedItem_; }

    void setSelectedPosition(int pos);
    int selectedPosition() const { return selectedPos_; }

    void setEditable(bool editable) { editable_ = editable; }
    bool editable() const { return editable_; }

    void setVisibleItemCount(int count) { visibleCount_ = count; }
    int visibleItemCount() const { return visibleCount_; }

    std::vector<ResourceSpec> resourceSpecs() const override;
    void onResourceChanged(const std::string& resourceName) override;

    void layout() override;
    void handleButtonPress(unsigned int button, int x, int y) override;

protected:
    void expose() override;
    void showDropdown();
    void hideDropdown();

    Type type_ = Type::DropDown;
    std::vector<std::string> items_;
    std::string selectedItem_;
    int selectedPos_ = -1;
    bool editable_ = true;
    int visibleCount_ = 5;
    bool listVisible_ = false;
};

} // namespace motif
