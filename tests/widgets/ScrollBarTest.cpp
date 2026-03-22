#include <gtest/gtest.h>
#include <motif/widgets/ScrollBar.h>

using namespace motif;

TEST(ScrollBarTest, DefaultState) {
    ScrollBar sb;
    EXPECT_EQ(sb.value(), 0);
    EXPECT_EQ(sb.minimum(), 0);
    EXPECT_EQ(sb.maximum(), 100);
}

TEST(ScrollBarTest, SetValue) {
    ScrollBar sb;
    sb.setValue(25);
    EXPECT_EQ(sb.value(), 25);
}

TEST(ScrollBarTest, SetRange) {
    ScrollBar sb;
    sb.setMinimum(5);
    sb.setMaximum(500);
    EXPECT_EQ(sb.minimum(), 5);
    EXPECT_EQ(sb.maximum(), 500);
}

TEST(ScrollBarTest, SliderSize) {
    ScrollBar sb;
    sb.setSliderSize(20);
    EXPECT_EQ(sb.sliderSize(), 20);
}

TEST(ScrollBarTest, Increment) {
    ScrollBar sb;
    sb.setIncrement(5);
    sb.setPageIncrement(25);
    EXPECT_EQ(sb.increment(), 5);
    EXPECT_EQ(sb.pageIncrement(), 25);
}

TEST(ScrollBarTest, Orientation) {
    ScrollBar sb;
    sb.setOrientation(ScrollBar::Orientation::Horizontal);
    EXPECT_EQ(sb.orientation(), ScrollBar::Orientation::Horizontal);
}
