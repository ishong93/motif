#include <gtest/gtest.h>
#include <motif/containers/Notebook.h>
#include <motif/widgets/Label.h>

using namespace motif;

TEST(NotebookTest, DefaultState) {
    Notebook nb;
    EXPECT_EQ(nb.pageCount(), 0);
    EXPECT_EQ(nb.currentPage(), 0);
}

TEST(NotebookTest, AddPage) {
    Notebook nb;
    Label page1(&nb);
    Label tab1(&nb);
    nb.addPage(&page1, &tab1, "Page 1");
    EXPECT_EQ(nb.pageCount(), 1);
}

TEST(NotebookTest, SetCurrentPage) {
    Notebook nb;
    Label p1(&nb), p2(&nb);
    Label t1(&nb), t2(&nb);
    nb.addPage(&p1, &t1, "Page 1");
    nb.addPage(&p2, &t2, "Page 2");

    nb.setCurrentPage(1);
    EXPECT_EQ(nb.currentPage(), 1);
}

TEST(NotebookTest, RemovePage) {
    Notebook nb;
    Label p1(&nb), p2(&nb);
    Label t1(&nb), t2(&nb);
    nb.addPage(&p1, &t1, "Page 1");
    nb.addPage(&p2, &t2, "Page 2");

    nb.removePage(0);
    EXPECT_EQ(nb.pageCount(), 1);
}
