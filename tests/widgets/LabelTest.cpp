#include <gtest/gtest.h>
#include <motif/widgets/Label.h>

using namespace motif;

TEST(LabelTest, DefaultState) {
    Label label;
    EXPECT_TRUE(label.text().empty());
    EXPECT_EQ(label.alignment(), Label::Alignment::Center);
}

TEST(LabelTest, SetText) {
    Label label;
    label.setText("Hello Motif");
    EXPECT_EQ(label.text(), "Hello Motif");
}

TEST(LabelTest, Alignment) {
    Label label;
    label.setAlignment(Label::Alignment::Beginning);
    EXPECT_EQ(label.alignment(), Label::Alignment::Beginning);

    label.setAlignment(Label::Alignment::End);
    EXPECT_EQ(label.alignment(), Label::Alignment::End);
}
