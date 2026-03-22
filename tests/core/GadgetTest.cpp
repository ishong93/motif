#include <gtest/gtest.h>
#include <motif/core/Gadget.h>
#include <motif/core/Manager.h>

using namespace motif;

TEST(GadgetTest, IsGadget) {
    Gadget g;
    EXPECT_TRUE(g.isGadget());

    Widget w;
    EXPECT_FALSE(w.isGadget());
}

TEST(GadgetTest, ContainsPoint) {
    Gadget g;
    g.setPosition(10, 20, 50, 30);

    EXPECT_TRUE(g.containsPoint(10, 20));
    EXPECT_TRUE(g.containsPoint(35, 35));
    EXPECT_TRUE(g.containsPoint(59, 49));

    EXPECT_FALSE(g.containsPoint(9, 20));
    EXPECT_FALSE(g.containsPoint(10, 19));
    EXPECT_FALSE(g.containsPoint(60, 50));
}

TEST(GadgetTest, DefaultThickness) {
    Gadget g;
    EXPECT_EQ(g.highlightThickness(), 0);
    EXPECT_EQ(g.shadowThickness(), 0);
}

TEST(GadgetTest, SetThickness) {
    Gadget g;
    g.setHighlightThickness(2);
    g.setShadowThickness(3);
    EXPECT_EQ(g.highlightThickness(), 2);
    EXPECT_EQ(g.shadowThickness(), 3);
}

TEST(GadgetTest, ParentChild) {
    Manager parent;
    Gadget child(&parent);
    EXPECT_EQ(child.parent(), &parent);
    EXPECT_EQ(parent.children().size(), 1);
}
