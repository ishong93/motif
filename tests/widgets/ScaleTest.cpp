#include <gtest/gtest.h>
#include <motif/widgets/Scale.h>

using namespace motif;

TEST(ScaleTest, DefaultState) {
    Scale s;
    EXPECT_EQ(s.value(), 0);
    EXPECT_EQ(s.minimum(), 0);
    EXPECT_EQ(s.maximum(), 100);
}

TEST(ScaleTest, SetValue) {
    Scale s;
    s.setValue(50);
    EXPECT_EQ(s.value(), 50);
}

TEST(ScaleTest, SetRange) {
    Scale s;
    s.setMinimum(10);
    s.setMaximum(200);
    EXPECT_EQ(s.minimum(), 10);
    EXPECT_EQ(s.maximum(), 200);
}

TEST(ScaleTest, Orientation) {
    Scale s;
    s.setOrientation(Scale::Orientation::Horizontal);
    EXPECT_EQ(s.orientation(), Scale::Orientation::Horizontal);

    s.setOrientation(Scale::Orientation::Vertical);
    EXPECT_EQ(s.orientation(), Scale::Orientation::Vertical);
}

TEST(ScaleTest, ShowValue) {
    Scale s;
    s.setShowValue(true);
    EXPECT_TRUE(s.showValue());

    s.setShowValue(false);
    EXPECT_FALSE(s.showValue());
}

TEST(ScaleTest, Title) {
    Scale s;
    s.setTitle("Volume");
    EXPECT_EQ(s.title(), "Volume");
}

TEST(ScaleTest, DecimalPoints) {
    Scale s;
    s.setDecimalPoints(2);
    EXPECT_EQ(s.decimalPoints(), 2);
}
