#include <gtest/gtest.h>
#include <motif/containers/TabStack.h>
#include <motif/widgets/Label.h>

using namespace motif;

TEST(TabStackTest, DefaultState) {
    TabStack ts;
    EXPECT_EQ(ts.tabCount(), 0);
    EXPECT_EQ(ts.selectedTab(), 0);
}

TEST(TabStackTest, AddTab) {
    TabStack ts;
    Label child(&ts);
    ts.addTab(&child, "Tab 1");
    EXPECT_EQ(ts.tabCount(), 1);
}

TEST(TabStackTest, SetSelectedTab) {
    TabStack ts;
    Label c1(&ts), c2(&ts);
    ts.addTab(&c1, "Tab 1");
    ts.addTab(&c2, "Tab 2");

    ts.setSelectedTab(1);
    EXPECT_EQ(ts.selectedTab(), 1);
}

TEST(TabStackTest, RemoveTab) {
    TabStack ts;
    Label c1(&ts), c2(&ts);
    ts.addTab(&c1, "Tab 1");
    ts.addTab(&c2, "Tab 2");

    ts.removeTab(0);
    EXPECT_EQ(ts.tabCount(), 1);
}
