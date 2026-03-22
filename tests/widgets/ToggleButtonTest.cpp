#include <gtest/gtest.h>
#include <motif/widgets/ToggleButton.h>

using namespace motif;

TEST(ToggleButtonTest, DefaultState) {
    ToggleButton tb;
    EXPECT_FALSE(tb.isSelected());
}

TEST(ToggleButtonTest, SelectDeselect) {
    ToggleButton tb;
    tb.setSelected(true);
    EXPECT_TRUE(tb.isSelected());

    tb.setSelected(false);
    EXPECT_FALSE(tb.isSelected());
}

TEST(ToggleButtonTest, IndicatorType) {
    ToggleButton tb;
    tb.setIndicatorType(ToggleButton::IndicatorType::CheckBox);
    EXPECT_EQ(tb.indicatorType(), ToggleButton::IndicatorType::CheckBox);

    tb.setIndicatorType(ToggleButton::IndicatorType::RadioButton);
    EXPECT_EQ(tb.indicatorType(), ToggleButton::IndicatorType::RadioButton);
}

TEST(ToggleButtonTest, InheritLabel) {
    ToggleButton tb;
    tb.setText("Option 1");
    EXPECT_EQ(tb.text(), "Option 1");
}
