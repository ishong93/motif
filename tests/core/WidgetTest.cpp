#include <gtest/gtest.h>
#include <motif/core/Widget.h>

using namespace motif;

// Test widget without X11 (no Application::open needed for non-realize tests)

TEST(WidgetTest, DefaultConstruction) {
    Widget w;
    EXPECT_EQ(w.parent(), nullptr);
    EXPECT_EQ(w.x(), 0);
    EXPECT_EQ(w.y(), 0);
    EXPECT_EQ(w.width(), 0);
    EXPECT_EQ(w.height(), 0);
    EXPECT_FALSE(w.isRealized());
    EXPECT_TRUE(w.isVisible());
    EXPECT_TRUE(w.isSensitive());
    EXPECT_EQ(w.window(), 0UL);
}

TEST(WidgetTest, ParentChildRelationship) {
    Widget parent;
    Widget child(&parent);

    EXPECT_EQ(child.parent(), &parent);
    EXPECT_EQ(parent.children().size(), 1);
    EXPECT_EQ(parent.children()[0], &child);
}

TEST(WidgetTest, Geometry) {
    Widget w;
    w.setPosition(10, 20, 100, 50);

    EXPECT_EQ(w.x(), 10);
    EXPECT_EQ(w.y(), 20);
    EXPECT_EQ(w.width(), 100);
    EXPECT_EQ(w.height(), 50);
}

TEST(WidgetTest, Visibility) {
    Widget w;
    EXPECT_TRUE(w.isVisible());

    w.hide();
    EXPECT_FALSE(w.isVisible());

    w.show();
    EXPECT_TRUE(w.isVisible());
}

TEST(WidgetTest, Sensitivity) {
    Widget w;
    EXPECT_TRUE(w.isSensitive());

    w.setSensitive(false);
    EXPECT_FALSE(w.isSensitive());
}

TEST(WidgetTest, Name) {
    Widget w;
    w.setName("myWidget");
    EXPECT_EQ(w.name(), "myWidget");
}

TEST(WidgetTest, CallbackRegistration) {
    Widget w;
    int callCount = 0;

    w.addCallback("activate", [&callCount](Widget&, void*) {
        callCount++;
    });

    // Callbacks are invoked internally; verify registration doesn't crash
    EXPECT_EQ(callCount, 0);
}

TEST(WidgetTest, ChildRemovalOnDestroy) {
    Widget parent;
    {
        Widget child(&parent);
        EXPECT_EQ(parent.children().size(), 1);
    }
    // child destroyed, should be removed from parent
    EXPECT_EQ(parent.children().size(), 0);
}
