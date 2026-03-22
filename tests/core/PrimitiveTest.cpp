#include <gtest/gtest.h>
#include <motif/core/Primitive.h>

using namespace motif;

TEST(PrimitiveTest, DefaultState) {
    Primitive p;
    EXPECT_EQ(p.highlightThickness(), 0);
    EXPECT_EQ(p.shadowThickness(), 0);
    EXPECT_FALSE(p.hasFocus());
}

TEST(PrimitiveTest, HighlightThickness) {
    Primitive p;
    p.setHighlightThickness(3);
    EXPECT_EQ(p.highlightThickness(), 3);
}

TEST(PrimitiveTest, ShadowThickness) {
    Primitive p;
    p.setShadowThickness(2);
    EXPECT_EQ(p.shadowThickness(), 2);
}

TEST(PrimitiveTest, AcceptsFocus) {
    Primitive p;
    EXPECT_TRUE(p.acceptsFocus());
}

TEST(PrimitiveTest, ParentChild) {
    Widget parent;
    Primitive child(&parent);
    EXPECT_EQ(child.parent(), &parent);
}
