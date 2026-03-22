#include <gtest/gtest.h>
#include <motif/uil/Compiler.h>
#include <motif/uil/Lexer.h>
#include <motif/uil/Parser.h>

#include <fstream>
#include <cstdio>

using namespace motif::uil;

TEST(CompilerTest, CompileSimpleModule) {
    const char* source = R"(
        module test_module
            version = 'v1.0'
            names = case_sensitive

        value
            greeting : exported string "Hello, Motif!";
            count : integer 42;

        object main_window : XmBulletinBoard {
            arguments {
                XmNwidth = 400;
                XmNheight = 300;
            };
        };

        end module;
    )";

    Compiler::Options opts;
    opts.outputFile = "test_output.uid";
    opts.verbose = false;

    Compiler compiler(opts);
    bool ok = compiler.compileSource(source, "test.uil");

    if (!ok) {
        for (auto& err : compiler.errors()) {
            std::cerr << "  Error: " << err << "\n";
        }
    }

    EXPECT_TRUE(ok);
    EXPECT_TRUE(compiler.errors().empty());

    // Clean up
    std::remove("test_output.uid");
}

TEST(CompilerTest, ParseError) {
    const char* source = R"(
        module broken
        ! Missing "end module"
    )";

    Compiler::Options opts;
    opts.outputFile = "broken.uid";
    Compiler compiler(opts);
    bool ok = compiler.compileSource(source, "broken.uil");

    EXPECT_FALSE(ok);
    EXPECT_FALSE(compiler.errors().empty());

    std::remove("broken.uid");
}

TEST(CompilerTest, ExpressionEvaluation) {
    // Test that the compiler can handle value expressions
    const char* source = R"(
        module expr_test
            names = case_sensitive

        value
            pi_approx : exported float 3.14;
            label : exported string "Hello";
            flag : exported boolean true;

        end module;
    )";

    Compiler::Options opts;
    opts.outputFile = "expr_test.uid";
    Compiler compiler(opts);
    bool ok = compiler.compileSource(source, "expr.uil");

    EXPECT_TRUE(ok);
    std::remove("expr_test.uid");
}

TEST(LexerTest, TokenizeKeywords) {
    const char* source = "module test names = case_sensitive end module";
    Lexer lexer(source, "test.uil");

    auto tok = lexer.next();
    EXPECT_EQ(tok.type, TokenType::Module);

    tok = lexer.next();
    EXPECT_EQ(tok.type, TokenType::Identifier);
    EXPECT_EQ(tok.text, "test");

    tok = lexer.next();
    EXPECT_EQ(tok.type, TokenType::Names);
}

TEST(LexerTest, TokenizeStringLiteral) {
    Lexer lexer("\"hello world\"", "test.uil");
    auto tok = lexer.next();
    EXPECT_EQ(tok.type, TokenType::StringLiteral);
    EXPECT_EQ(tok.text, "hello world");
}

TEST(LexerTest, TokenizeNumbers) {
    Lexer lexer("42 3.14", "test.uil");

    auto tok = lexer.next();
    EXPECT_EQ(tok.type, TokenType::IntegerLiteral);
    EXPECT_EQ(tok.text, "42");

    tok = lexer.next();
    EXPECT_EQ(tok.type, TokenType::FloatLiteral);
    EXPECT_EQ(tok.text, "3.14");
}
