#include <gtest/gtest.h>
#include <motif/core/Manager.h>
#include <motif/core/Primitive.h>

using namespace motif;

TEST(ManagerTest, DefaultState) {
    Manager m;
    EXPECT_EQ(m.shadowThickness(), 0);
    EXPECT_EQ(m.children().size(), 0);
}

TEST(ManagerTest, AddChildren) {
    Manager m;
    Primitive c1(&m);
    Primitive c2(&m);
    EXPECT_EQ(m.children().size(), 2);
}

TEST(ManagerTest, ShadowThickness) {
    Manager m;
    m.setShadowThickness(3);
    EXPECT_EQ(m.shadowThickness(), 3);
}

TEST(ManagerTest, ChildRemoval) {
    Manager m;
    {
        Primitive child(&m);
        EXPECT_EQ(m.children().size(), 1);
    }
    EXPECT_EQ(m.children().size(), 0);
}
