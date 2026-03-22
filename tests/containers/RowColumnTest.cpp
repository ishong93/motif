#include <gtest/gtest.h>
#include <motif/containers/RowColumn.h>
#include <motif/widgets/PushButton.h>

using namespace motif;

TEST(RowColumnTest, DefaultState) {
    RowColumn rc;
    EXPECT_EQ(rc.numColumns(), 1);
    EXPECT_EQ(rc.spacing(), 0);
}

TEST(RowColumnTest, Orientation) {
    RowColumn rc;
    rc.setOrientation(RowColumn::Orientation::Horizontal);
    EXPECT_EQ(rc.orientation(), RowColumn::Orientation::Horizontal);

    rc.setOrientation(RowColumn::Orientation::Vertical);
    EXPECT_EQ(rc.orientation(), RowColumn::Orientation::Vertical);
}

TEST(RowColumnTest, NumColumns) {
    RowColumn rc;
    rc.setNumColumns(3);
    EXPECT_EQ(rc.numColumns(), 3);
}

TEST(RowColumnTest, Spacing) {
    RowColumn rc;
    rc.setSpacing(10);
    EXPECT_EQ(rc.spacing(), 10);
}

TEST(RowColumnTest, AddChildren) {
    RowColumn rc;
    PushButton b1(&rc);
    PushButton b2(&rc);
    PushButton b3(&rc);
    EXPECT_EQ(rc.children().size(), 3);
}
