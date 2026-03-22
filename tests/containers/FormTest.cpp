#include <gtest/gtest.h>
#include <motif/containers/Form.h>
#include <motif/widgets/PushButton.h>

using namespace motif;

TEST(FormTest, DefaultState) {
    Form form;
    EXPECT_EQ(form.fractionBase(), 100);
    EXPECT_EQ(form.children().size(), 0);
}

TEST(FormTest, FractionBase) {
    Form form;
    form.setFractionBase(200);
    EXPECT_EQ(form.fractionBase(), 200);
}

TEST(FormTest, SetConstraint) {
    Form form;
    PushButton btn(&form);

    Form::Constraint c;
    c.topAttachment = Form::Attachment::Form;
    c.leftAttachment = Form::Attachment::Form;
    c.topOffset = 10;
    c.leftOffset = 20;

    form.setConstraint(&btn, c);

    auto* retrieved = form.constraint(&btn);
    ASSERT_NE(retrieved, nullptr);
    EXPECT_EQ(retrieved->topAttachment, Form::Attachment::Form);
    EXPECT_EQ(retrieved->topOffset, 10);
    EXPECT_EQ(retrieved->leftOffset, 20);
}

TEST(FormTest, RemoveConstraint) {
    Form form;
    PushButton btn(&form);

    Form::Constraint c;
    form.setConstraint(&btn, c);
    EXPECT_NE(form.constraint(&btn), nullptr);

    form.removeConstraint(&btn);
    EXPECT_EQ(form.constraint(&btn), nullptr);
}
