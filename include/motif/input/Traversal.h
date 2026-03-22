#pragma once

#include <motif/core/Widget.h>

namespace motif {

// Keyboard focus traversal among widgets.
namespace traversal {

enum class Direction { Next, Previous, Home, End, Up, Down, Left, Right };

// Move focus in the given direction from the current focus widget.
Widget* traverse(Widget* current, Direction direction);

// Set the initial focus within a widget tree.
Widget* findInitialFocus(Widget* root);

// Check if a widget can receive focus.
bool isTraversable(Widget* w);

// Explicitly move focus to a widget.
void setFocus(Widget* widget);

} // namespace traversal
} // namespace motif
