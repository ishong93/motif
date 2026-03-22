#include <gtest/gtest.h>
#include <motif/widgets/Text.h>

using namespace motif;

TEST(TextTest, DefaultState) {
    Text t;
    EXPECT_TRUE(t.value().empty());
    EXPECT_TRUE(t.editable());
    EXPECT_EQ(t.cursorPosition(), 0);
}

TEST(TextTest, SetValue) {
    Text t;
    t.setValue("Hello World");
    EXPECT_EQ(t.value(), "Hello World");
}

TEST(TextTest, Editable) {
    Text t;
    t.setEditable(false);
    EXPECT_FALSE(t.editable());
}

TEST(TextTest, MaxLength) {
    Text t;
    t.setMaxLength(100);
    EXPECT_EQ(t.maxLength(), 100);
}

TEST(TextTest, CursorPosition) {
    Text t;
    t.setValue("Hello");
    t.setCursorPosition(3);
    EXPECT_EQ(t.cursorPosition(), 3);
}

TEST(TextTest, Insert) {
    Text t;
    t.setValue("Hello World");
    t.insert(5, ",");
    EXPECT_EQ(t.value(), "Hello, World");
}

TEST(TextTest, Replace) {
    Text t;
    t.setValue("Hello World");
    t.replace(0, 5, "Hi");
    EXPECT_EQ(t.value(), "Hi World");
}

TEST(TextTest, RowsColumns) {
    Text t;
    t.setRows(10);
    t.setColumns(80);
    EXPECT_EQ(t.rows(), 10);
    EXPECT_EQ(t.columns(), 80);
}

TEST(TextTest, WordWrap) {
    Text t;
    t.setWordWrap(true);
    EXPECT_TRUE(t.wordWrap());
}
