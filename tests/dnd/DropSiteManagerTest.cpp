#include <gtest/gtest.h>
#include <motif/dnd/DropSiteManager.h>
#include <motif/core/Widget.h>

using namespace motif;

TEST(DropSiteManagerTest, Singleton) {
    auto& mgr1 = DropSiteManager::instance();
    auto& mgr2 = DropSiteManager::instance();
    EXPECT_EQ(&mgr1, &mgr2);
}

TEST(DropSiteManagerTest, RegisterUnregister) {
    auto& mgr = DropSiteManager::instance();
    Widget w;

    DropSiteManager::DropSite site;
    site.widget = &w;
    site.importTargets = {"text/plain"};
    site.activity = DropSiteManager::DropActivity::Active;

    // Should not crash
    mgr.registerDropSite(&w, site);
    mgr.unregisterDropSite(&w);
}

TEST(DropSiteManagerTest, DropCallbacks) {
    DropSiteManager::DropSite site;
    int enterCount = 0;
    int leaveCount = 0;

    site.dragEnterCb = [&](DragContext&) { enterCount++; };
    site.dragLeaveCb = [&]() { leaveCount++; };
    site.dropCb = [](DragContext&, const std::string&) { return true; };

    EXPECT_EQ(enterCount, 0);
    EXPECT_EQ(leaveCount, 0);
}
