#include <gtest/gtest.h>
#include <motif/containers/BulletinBoard.h>
#include <motif/widgets/PushButton.h>

using namespace motif;

TEST(BulletinBoardTest, DefaultState) {
    BulletinBoard bb;
    EXPECT_EQ(bb.marginWidth(), 0);
    EXPECT_EQ(bb.marginHeight(), 0);
}

TEST(BulletinBoardTest, SetMargins) {
    BulletinBoard bb;
    bb.setMarginWidth(10);
    bb.setMarginHeight(15);
    EXPECT_EQ(bb.marginWidth(), 10);
    EXPECT_EQ(bb.marginHeight(), 15);
}

TEST(BulletinBoardTest, AddChildren) {
    BulletinBoard bb;
    PushButton b1(&bb);
    b1.setPosition(10, 10, 80, 30);
    PushButton b2(&bb);
    b2.setPosition(10, 50, 80, 30);
    EXPECT_EQ(bb.children().size(), 2);
}
