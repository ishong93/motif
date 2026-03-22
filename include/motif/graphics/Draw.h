#pragma once

#include <motif/core/Widget.h>

namespace motif {

// Low-level drawing utilities for shadow/highlight rendering.
namespace draw {

void drawShadow(Widget* w, int x, int y, int width, int height,
                int thickness, bool raised);

void drawEtchedShadow(Widget* w, int x, int y, int width, int height,
                       int thickness, bool raised);

void drawArrow(Widget* w, int x, int y, int size,
               int direction, bool raised);

void drawHighlight(Widget* w, int x, int y, int width, int height,
                   int thickness);

void drawSeparator(Widget* w, int x1, int y1, int x2, int y2, int type);

} // namespace draw
} // namespace motif
