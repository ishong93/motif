#include <gtest/gtest.h>
#include <motif/dialogs/FileSelectionBox.h>

using namespace motif;

TEST(FileSelectionBoxTest, DefaultState) {
    FileSelectionBox fsb;
    EXPECT_TRUE(fsb.selectedFile().empty());
}

TEST(FileSelectionBoxTest, SetDirectory) {
    FileSelectionBox fsb;
    fsb.setDirectory("/tmp");
    // No crash, directory stored
}

TEST(FileSelectionBoxTest, SetPattern) {
    FileSelectionBox fsb;
    fsb.setPattern("*.txt");
    // Pattern stored
}

TEST(FileSelectionBoxTest, Callbacks) {
    FileSelectionBox fsb;
    std::string selected;
    fsb.setOkCallback([&](const std::string& path) { selected = path; });
    fsb.setCancelCallback([]() {});
    EXPECT_TRUE(selected.empty());
}
