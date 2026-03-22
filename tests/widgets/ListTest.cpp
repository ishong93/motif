#include <gtest/gtest.h>
#include <motif/widgets/List.h>

using namespace motif;

TEST(ListTest, DefaultState) {
    List list;
    EXPECT_TRUE(list.items().empty());
    EXPECT_TRUE(list.selectedPositions().empty());
}

TEST(ListTest, SetItems) {
    List list;
    list.setItems({"Alpha", "Beta", "Gamma"});
    EXPECT_EQ(list.items().size(), 3);
    EXPECT_EQ(list.items()[0], "Alpha");
    EXPECT_EQ(list.items()[2], "Gamma");
}

TEST(ListTest, AddItem) {
    List list;
    list.addItem("First");
    list.addItem("Second");
    EXPECT_EQ(list.items().size(), 2);
}

TEST(ListTest, RemoveItem) {
    List list;
    list.setItems({"A", "B", "C"});
    list.removeItem(1);
    EXPECT_EQ(list.items().size(), 2);
}

TEST(ListTest, RemoveAllItems) {
    List list;
    list.setItems({"A", "B", "C"});
    list.removeAllItems();
    EXPECT_TRUE(list.items().empty());
}

TEST(ListTest, SelectItem) {
    List list;
    list.setItems({"A", "B", "C"});
    list.selectItem(0);
    auto sel = list.selectedPositions();
    EXPECT_EQ(sel.size(), 1);
    EXPECT_EQ(sel[0], 0);
}

TEST(ListTest, DeselectItem) {
    List list;
    list.setItems({"A", "B", "C"});
    list.selectItem(0);
    list.deselectItem(0);
    EXPECT_TRUE(list.selectedPositions().empty());
}

TEST(ListTest, DeselectAll) {
    List list;
    list.setSelectionPolicy(List::SelectionPolicy::Multiple);
    list.setItems({"A", "B", "C"});
    list.selectItem(0);
    list.selectItem(1);
    list.deselectAll();
    EXPECT_TRUE(list.selectedPositions().empty());
}

TEST(ListTest, SelectionPolicy) {
    List list;
    list.setSelectionPolicy(List::SelectionPolicy::Multiple);
    EXPECT_EQ(list.selectionPolicy(), List::SelectionPolicy::Multiple);
}

TEST(ListTest, VisibleItemCount) {
    List list;
    list.setVisibleItemCount(10);
    EXPECT_EQ(list.visibleItemCount(), 10);
}
