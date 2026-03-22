#include <gtest/gtest.h>
#include <motif/containers/PanedWindow.h>
#include <motif/widgets/Label.h>

using namespace motif;

TEST(PanedWindowTest, DefaultState) {
    PanedWindow pw;
    EXPECT_EQ(pw.children().size(), 0);
}

TEST(PanedWindowTest, Orientation) {
    PanedWindow pw;
    pw.setOrientation(PanedWindow::Orientation::Horizontal);
    EXPECT_EQ(pw.orientation(), PanedWindow::Orientation::Horizontal);
}

TEST(PanedWindowTest, SashDimensions) {
    PanedWindow pw;
    pw.setSashWidth(12);
    pw.setSashHeight(8);
    EXPECT_EQ(pw.sashWidth(), 12);
    EXPECT_EQ(pw.sashHeight(), 8);
}

TEST(PanedWindowTest, Spacing) {
    PanedWindow pw;
    pw.setSpacing(5);
    EXPECT_EQ(pw.spacing(), 5);
}

TEST(PanedWindowTest, AddPanes) {
    PanedWindow pw;
    Label l1(&pw);
    Label l2(&pw);
    EXPECT_EQ(pw.children().size(), 2);
}
