#include <gtest/gtest.h>
#include <motif/dnd/DragContext.h>

using namespace motif;

TEST(DragContextTest, Construction) {
    Widget source;
    DragContext ctx(&source, {"text/plain", "UTF8_STRING"});

    EXPECT_EQ(ctx.source(), &source);
    EXPECT_EQ(ctx.exportTargets().size(), 2);
    EXPECT_EQ(ctx.exportTargets()[0], "text/plain");
    EXPECT_FALSE(ctx.isDragging());
}

TEST(DragContextTest, DefaultAction) {
    Widget source;
    DragContext ctx(&source, {"text/plain"});
    EXPECT_EQ(ctx.dragAction(), DragContext::DragAction::Copy);
}

TEST(DragContextTest, SetAction) {
    Widget source;
    DragContext ctx(&source, {"text/plain"});
    ctx.setDragAction(DragContext::DragAction::Move);
    EXPECT_EQ(ctx.dragAction(), DragContext::DragAction::Move);
}

TEST(DragContextTest, Threshold) {
    Widget source;
    DragContext ctx(&source, {"text/plain"});
    EXPECT_EQ(ctx.dragThreshold(), 5);

    ctx.setDragThreshold(10);
    EXPECT_EQ(ctx.dragThreshold(), 10);
}

TEST(DragContextTest, ConvertCallback) {
    Widget source;
    DragContext ctx(&source, {"text/plain"});

    ctx.setConvertCallback([](const std::string& target, std::string& data) -> bool {
        if (target == "text/plain") {
            data = "hello";
            return true;
        }
        return false;
    });

    std::string result;
    EXPECT_TRUE(ctx.convertData("text/plain", result));
    EXPECT_EQ(result, "hello");
    EXPECT_FALSE(ctx.convertData("image/png", result));
}
