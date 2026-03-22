#include <gtest/gtest.h>
#include <motif/core/Resource.h>

using namespace motif;

TEST(ResourceTest, SetAndGet) {
    Resource res;
    res.set("width", 100);
    res.set("label", std::string("Hello"));

    EXPECT_EQ(res.get<int>("width"), 100);
    EXPECT_EQ(res.get<std::string>("label"), "Hello");
}

TEST(ResourceTest, DefaultValue) {
    Resource res;
    EXPECT_EQ(res.get<int>("missing", 42), 42);
    EXPECT_EQ(res.get<std::string>("missing", "default"), "default");
}

TEST(ResourceTest, Has) {
    Resource res;
    EXPECT_FALSE(res.has("key"));

    res.set("key", 1);
    EXPECT_TRUE(res.has("key"));
}

TEST(ResourceTest, Remove) {
    Resource res;
    res.set("key", 1);
    EXPECT_TRUE(res.has("key"));

    res.remove("key");
    EXPECT_FALSE(res.has("key"));
}

TEST(ResourceTest, TypeMismatchReturnsDefault) {
    Resource res;
    res.set("number", 42);

    // Requesting wrong type returns default
    EXPECT_EQ(res.get<std::string>("number", "fallback"), "fallback");
}
