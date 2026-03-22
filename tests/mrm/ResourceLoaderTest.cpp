#include <gtest/gtest.h>
#include <motif/mrm/ResourceLoader.h>

using namespace motif;

TEST(ResourceLoaderTest, Singleton) {
    auto& rl1 = ResourceLoader::instance();
    auto& rl2 = ResourceLoader::instance();
    EXPECT_EQ(&rl1, &rl2);
}

TEST(ResourceLoaderTest, RegisterProcedure) {
    auto& rl = ResourceLoader::instance();

    bool called = false;
    rl.registerProcedure("testProc", [&](Widget*, void*) { called = true; });

    auto proc = rl.findProcedure("testProc");
    ASSERT_NE(proc, nullptr);
    proc(nullptr, nullptr);
    EXPECT_TRUE(called);
}

TEST(ResourceLoaderTest, FindMissingProcedure) {
    auto& rl = ResourceLoader::instance();
    auto proc = rl.findProcedure("nonexistent_proc_xyz");
    EXPECT_EQ(proc, nullptr);
}

TEST(ResourceLoaderTest, RegisterWidgetClass) {
    auto& rl = ResourceLoader::instance();

    rl.registerWidgetClass("TestWidget", [](Widget* parent, const std::string& name) -> Widget* {
        auto* w = new Widget(parent);
        w->setName(name);
        return w;
    });

    auto factory = rl.findWidgetFactory("TestWidget");
    ASSERT_NE(factory, nullptr);

    Widget* w = factory(nullptr, "myWidget");
    ASSERT_NE(w, nullptr);
    EXPECT_EQ(w->name(), "myWidget");
    delete w;
}

TEST(ResourceLoaderTest, FindMissingFactory) {
    auto& rl = ResourceLoader::instance();
    auto factory = rl.findWidgetFactory("NonexistentWidget_xyz");
    EXPECT_EQ(factory, nullptr);
}

TEST(ResourceLoaderTest, OpenNonexistentHierarchy) {
    auto& rl = ResourceLoader::instance();
    bool result = rl.openHierarchy({"/nonexistent/path.uid"});
    EXPECT_FALSE(result);
}

TEST(ResourceLoaderTest, FetchWidgetNoHierarchy) {
    auto& rl = ResourceLoader::instance();
    rl.close();
    Widget* w = rl.fetchWidget("test", nullptr);
    EXPECT_EQ(w, nullptr);
}
