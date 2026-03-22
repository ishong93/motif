#include <gtest/gtest.h>
#include <motif/widgets/PushButton.h>

using namespace motif;

TEST(PushButtonTest, DefaultState) {
    PushButton btn;
    EXPECT_FALSE(btn.isArmed());
    EXPECT_FALSE(btn.showAsDefault());
}

TEST(PushButtonTest, ArmDisarm) {
    PushButton btn;
    btn.arm();
    EXPECT_TRUE(btn.isArmed());

    btn.disarm();
    EXPECT_FALSE(btn.isArmed());
}

TEST(PushButtonTest, ShowAsDefault) {
    PushButton btn;
    btn.setShowAsDefault(true);
    EXPECT_TRUE(btn.showAsDefault());
}

TEST(PushButtonTest, InheritLabel) {
    PushButton btn;
    btn.setText("Click Me");
    EXPECT_EQ(btn.text(), "Click Me");
}

TEST(PushButtonTest, ActivateCallback) {
    PushButton btn;
    int count = 0;
    btn.addCallback("activate", [&count](Widget&, void*) { count++; });
    btn.activate();
    EXPECT_EQ(count, 1);
}
