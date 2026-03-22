#include <gtest/gtest.h>
#include <motif/dialogs/Command.h>

using namespace motif;

TEST(CommandDialogTest, DefaultState) {
    CommandDialog cmd;
    EXPECT_TRUE(cmd.command().empty());
    EXPECT_TRUE(cmd.history().empty());
}

TEST(CommandDialogTest, SetCommand) {
    CommandDialog cmd;
    cmd.setCommand("ls -la");
    EXPECT_EQ(cmd.command(), "ls -la");
}

TEST(CommandDialogTest, Callback) {
    CommandDialog cmd;
    std::string executed;
    cmd.setCommandCallback([&](const std::string& c) { executed = c; });
    EXPECT_TRUE(executed.empty());
}
