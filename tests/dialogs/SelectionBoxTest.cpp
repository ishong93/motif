#include <gtest/gtest.h>
#include <motif/dialogs/SelectionBox.h>

using namespace motif;

TEST(SelectionBoxTest, DefaultState) {
    SelectionBox sb;
    EXPECT_TRUE(sb.selectedItem().empty());
}

TEST(SelectionBoxTest, SetItems) {
    SelectionBox sb;
    sb.setItems({"Item A", "Item B", "Item C"});
    // Items stored without crash
}

TEST(SelectionBoxTest, SetSelectedItem) {
    SelectionBox sb;
    sb.setItems({"Red", "Green", "Blue"});
    sb.setSelectedItem("Green");
    EXPECT_EQ(sb.selectedItem(), "Green");
}

TEST(SelectionBoxTest, Callbacks) {
    SelectionBox sb;
    std::string result;
    sb.setOkCallback([&](const std::string& item) { result = item; });
    sb.setCancelCallback([]() {});
    EXPECT_TRUE(result.empty());
}
