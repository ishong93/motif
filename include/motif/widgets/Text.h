#pragma once

#include <motif/core/Primitive.h>
#include <string>
#include <functional>

namespace motif {

class Text : public Primitive {
public:
    Text() = default;
    explicit Text(Widget* parent);
    ~Text() override;

    static constexpr const char* ValueChangedCallback  = "valueChanged";
    static constexpr const char* ModifyVerifyCallback   = "modifyVerify";
    static constexpr const char* MotionVerifyCallback   = "motionVerify";
    static constexpr const char* FocusCallback          = "focus";
    static constexpr const char* LosingFocusCallback    = "losingFocus";

    void setValue(const std::string& value);
    const std::string& value() const { return value_; }

    void setEditable(bool editable) { editable_ = editable; }
    bool editable() const { return editable_; }

    void setMaxLength(int maxLen) { maxLength_ = maxLen; }
    int maxLength() const { return maxLength_; }

    void setRows(int rows) { rows_ = rows; }
    int rows() const { return rows_; }

    void setColumns(int cols) { columns_ = cols; }
    int columns() const { return columns_; }

    void setWordWrap(bool wrap) { wordWrap_ = wrap; }
    bool wordWrap() const { return wordWrap_; }

    // Cursor position
    void setCursorPosition(int pos);
    int cursorPosition() const { return cursorPos_; }

    // Selection
    void setSelection(int start, int end);
    std::string selectedText() const;

    void insert(int pos, const std::string& text);
    void replace(int from, int to, const std::string& text);

    // Resource system
    std::vector<ResourceSpec> resourceSpecs() const override;
    void onResourceChanged(const std::string& resourceName) override;

    void handleKeyPress(unsigned int keycode, unsigned int state) override;
    void handleButtonPress(unsigned int button, int x, int y) override;

protected:
    void expose() override;

    std::string value_;
    bool editable_  = true;
    int maxLength_  = 0; // 0 = unlimited
    int rows_       = 1;
    int columns_    = 20;
    bool wordWrap_  = false;
    int cursorPos_  = 0;
    int selStart_   = -1;
    int selEnd_     = -1;
    int scrollOffset_ = 0;
};

} // namespace motif
