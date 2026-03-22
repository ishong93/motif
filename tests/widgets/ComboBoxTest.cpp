#include <gtest/gtest.h>
#include <motif/widgets/ComboBox.h>

using namespace motif;

TEST(ComboBoxTest, DefaultState) {
    ComboBox cb;
    EXPECT_TRUE(cb.items().empty());
    EXPECT_TRUE(cb.selectedItem().empty());
}

TEST(ComboBoxTest, SetItems) {
    ComboBox cb;
    cb.setItems({"Red", "Green", "Blue"});
    EXPECT_EQ(cb.items().size(), 3);
}

TEST(ComboBoxTest, SelectByPosition) {
    ComboBox cb;
    cb.setItems({"Red", "Green", "Blue"});
    cb.setSelectedPosition(1);
    EXPECT_EQ(cb.selectedPosition(), 1);
    EXPECT_EQ(cb.selectedItem(), "Green");
}

TEST(ComboBoxTest, SelectByItem) {
    ComboBox cb;
    cb.setItems({"Red", "Green", "Blue"});
    cb.setSelectedItem("Blue");
    EXPECT_EQ(cb.selectedItem(), "Blue");
}

TEST(ComboBoxTest, VisibleItemCount) {
    ComboBox cb;
    cb.setVisibleItemCount(5);
    EXPECT_EQ(cb.visibleItemCount(), 5);
}

TEST(ComboBoxTest, Editable) {
    ComboBox cb;
    cb.setEditable(true);
    EXPECT_TRUE(cb.editable());
}
