#include <gtest/gtest.h>
#include <motif/containers/Frame.h>
#include <motif/widgets/Label.h>

using namespace motif;

TEST(FrameTest, DefaultState) {
    Frame f;
    EXPECT_EQ(f.children().size(), 0);
}

TEST(FrameTest, ShadowType) {
    Frame f;
    f.setShadowType(Frame::ShadowType::In);
    EXPECT_EQ(f.shadowType(), Frame::ShadowType::In);

    f.setShadowType(Frame::ShadowType::Out);
    EXPECT_EQ(f.shadowType(), Frame::ShadowType::Out);
}

TEST(FrameTest, AddChild) {
    Frame f;
    Label child(&f);
    EXPECT_EQ(f.children().size(), 1);
}
