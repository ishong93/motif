#include <gtest/gtest.h>
#include <motif/core/CompoundString.h>

using namespace motif;

TEST(CompoundStringTest, DefaultEmpty) {
    CompoundString cs;
    EXPECT_TRUE(cs.empty());
    EXPECT_EQ(cs.segmentCount(), 0);
}

TEST(CompoundStringTest, CreateFromString) {
    CompoundString cs("Hello");
    EXPECT_FALSE(cs.empty());
    EXPECT_EQ(cs.segmentCount(), 1);
    EXPECT_EQ(cs.toUtf8(), "Hello");
}

TEST(CompoundStringTest, CreateWithFontTag) {
    CompoundString cs("Text", "bold");
    EXPECT_EQ(cs.segmentCount(), 1);
    EXPECT_EQ(cs.segment(0).text, "Text");
    EXPECT_EQ(cs.segment(0).fontTag, "bold");
}

TEST(CompoundStringTest, StaticCreate) {
    auto cs = CompoundString::create("World");
    EXPECT_EQ(cs.toUtf8(), "World");
}

TEST(CompoundStringTest, Append) {
    CompoundString cs("Hello");
    cs.append(" World");
    EXPECT_EQ(cs.segmentCount(), 2);
    EXPECT_EQ(cs.toUtf8(), "Hello World");
}

TEST(CompoundStringTest, AppendSeparator) {
    CompoundString cs("Line1");
    cs.appendSeparator();
    cs.append("Line2");
    EXPECT_EQ(cs.segmentCount(), 3);
}

TEST(CompoundStringTest, Concatenation) {
    CompoundString a("Hello");
    CompoundString b(" World");
    auto c = a + b;
    EXPECT_EQ(c.toUtf8(), "Hello World");
}

TEST(CompoundStringTest, PlusEquals) {
    CompoundString cs("Hello");
    cs += CompoundString(" World");
    EXPECT_EQ(cs.toUtf8(), "Hello World");
}

TEST(CompoundStringTest, Equality) {
    CompoundString a("Hello");
    CompoundString b("Hello");
    CompoundString c("World");
    EXPECT_TRUE(a == b);
    EXPECT_FALSE(a == c);
}

TEST(CompoundStringTest, SerializeDeserialize) {
    CompoundString original("Test String");
    auto data = original.serialize();
    EXPECT_FALSE(data.empty());

    auto restored = CompoundString::deserialize(data.data(), data.size());
    EXPECT_EQ(original.toUtf8(), restored.toUtf8());
}
