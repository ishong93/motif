#include <gtest/gtest.h>
#include <motif/dialogs/MessageBox.h>

using namespace motif;

TEST(MessageBoxTest, DefaultType) {
    MessageBox mb;
    // Default should construct without crash
    EXPECT_NO_THROW(mb.setType(MessageBox::Type::Information));
}

TEST(MessageBoxTest, SetMessage) {
    MessageBox mb;
    mb.setMessage("Test message");
    EXPECT_EQ(mb.messageLabel().text(), "Test message");
}

TEST(MessageBoxTest, SetLabels) {
    MessageBox mb;
    mb.setOkLabel("Yes");
    EXPECT_EQ(mb.okButton().text(), "Yes");
}

TEST(MessageBoxTest, ShowCancel) {
    MessageBox mb;
    mb.setShowCancel(true);
    mb.setShowHelp(false);
    // No crash
}

TEST(MessageBoxTest, Callbacks) {
    MessageBox mb;
    int okCount = 0, cancelCount = 0;
    mb.setOkCallback([&]() { okCount++; });
    mb.setCancelCallback([&]() { cancelCount++; });
    // Callbacks stored but not fired without popup
    EXPECT_EQ(okCount, 0);
}

TEST(MessageBoxTest, AllTypes) {
    MessageBox mb;
    mb.setType(MessageBox::Type::Warning);
    mb.setType(MessageBox::Type::Error);
    mb.setType(MessageBox::Type::Question);
    mb.setType(MessageBox::Type::Working);
}
