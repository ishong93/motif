#include <motif/print/PrintContext.h>
#include <motif/core/Widget.h>

#ifdef HAVE_CAIRO
#include <cairo/cairo.h>
#include <cairo/cairo-pdf.h>
#include <cairo/cairo-ps.h>
#endif

#include <cmath>

namespace motif {

PrintContext::PrintContext() = default;

PrintContext::~PrintContext() {
    if (active_) {
        endJob();
    }
}

double PrintContext::printableWidth() const {
    return setup_.paperWidth - setup_.marginLeft - setup_.marginRight;
}

double PrintContext::printableHeight() const {
    return setup_.paperHeight - setup_.marginTop - setup_.marginBottom;
}

bool PrintContext::beginJob(const std::string& outputPath) {
#ifdef HAVE_CAIRO
    double w = setup_.paperWidth;
    double h = setup_.paperHeight;

    if (setup_.orientation == Orientation::Landscape) {
        std::swap(w, h);
    }

    cairo_surface_t* surface = nullptr;
    switch (setup_.format) {
        case OutputFormat::PDF:
            surface = cairo_pdf_surface_create(outputPath.c_str(), w, h);
            break;
        case OutputFormat::PostScript:
            surface = cairo_ps_surface_create(outputPath.c_str(), w, h);
            break;
    }

    if (!surface || cairo_surface_status(surface) != CAIRO_STATUS_SUCCESS) {
        if (surface) cairo_surface_destroy(surface);
        return false;
    }

    auto* cr = cairo_create(surface);
    if (cairo_status(cr) != CAIRO_STATUS_SUCCESS) {
        cairo_destroy(cr);
        cairo_surface_destroy(surface);
        return false;
    }

    surface_ = surface;
    cr_ = cr;
    active_ = true;
    currentPage_ = 0;

    return true;
#else
    (void)outputPath;
    return false;
#endif
}

bool PrintContext::beginPage() {
#ifdef HAVE_CAIRO
    if (!active_ || !cr_) return false;

    auto* cr = static_cast<cairo_t*>(cr_);

    // Apply margin offset
    cairo_save(cr);
    double mx = setup_.marginLeft;
    double my = setup_.marginTop;
    if (setup_.orientation == Orientation::Landscape) {
        std::swap(mx, my);
    }
    cairo_translate(cr, mx, my);

    ++currentPage_;
    return true;
#else
    return false;
#endif
}

void PrintContext::endPage() {
#ifdef HAVE_CAIRO
    if (!active_ || !cr_) return;
    auto* cr = static_cast<cairo_t*>(cr_);
    cairo_restore(cr);
    cairo_show_page(cr);
#endif
}

void PrintContext::endJob() {
#ifdef HAVE_CAIRO
    if (cr_) {
        cairo_destroy(static_cast<cairo_t*>(cr_));
        cr_ = nullptr;
    }
    if (surface_) {
        cairo_surface_finish(static_cast<cairo_surface_t*>(surface_));
        cairo_surface_destroy(static_cast<cairo_surface_t*>(surface_));
        surface_ = nullptr;
    }
    active_ = false;
#endif
}

void PrintContext::setColor(double r, double g, double b, double a) {
#ifdef HAVE_CAIRO
    if (!cr_) return;
    cairo_set_source_rgba(static_cast<cairo_t*>(cr_), r, g, b, a);
#else
    (void)r; (void)g; (void)b; (void)a;
#endif
}

void PrintContext::setLineWidth(double w) {
#ifdef HAVE_CAIRO
    if (!cr_) return;
    cairo_set_line_width(static_cast<cairo_t*>(cr_), w);
#else
    (void)w;
#endif
}

void PrintContext::setFontFace(const std::string& family, bool bold, bool italic) {
#ifdef HAVE_CAIRO
    if (!cr_) return;
    cairo_select_font_face(
        static_cast<cairo_t*>(cr_),
        family.c_str(),
        italic ? CAIRO_FONT_SLANT_ITALIC : CAIRO_FONT_SLANT_NORMAL,
        bold ? CAIRO_FONT_WEIGHT_BOLD : CAIRO_FONT_WEIGHT_NORMAL
    );
#else
    (void)family; (void)bold; (void)italic;
#endif
}

void PrintContext::setFontSize(double size) {
#ifdef HAVE_CAIRO
    if (!cr_) return;
    cairo_set_font_size(static_cast<cairo_t*>(cr_), size);
#else
    (void)size;
#endif
}

void PrintContext::drawLine(double x1, double y1, double x2, double y2) {
#ifdef HAVE_CAIRO
    if (!cr_) return;
    auto* cr = static_cast<cairo_t*>(cr_);
    cairo_move_to(cr, x1, y1);
    cairo_line_to(cr, x2, y2);
    cairo_stroke(cr);
#else
    (void)x1; (void)y1; (void)x2; (void)y2;
#endif
}

void PrintContext::drawRect(double x, double y, double w, double h, bool fill) {
#ifdef HAVE_CAIRO
    if (!cr_) return;
    auto* cr = static_cast<cairo_t*>(cr_);
    cairo_rectangle(cr, x, y, w, h);
    if (fill) {
        cairo_fill(cr);
    } else {
        cairo_stroke(cr);
    }
#else
    (void)x; (void)y; (void)w; (void)h; (void)fill;
#endif
}

void PrintContext::drawText(double x, double y, const std::string& text) {
#ifdef HAVE_CAIRO
    if (!cr_) return;
    auto* cr = static_cast<cairo_t*>(cr_);
    cairo_move_to(cr, x, y);
    cairo_show_text(cr, text.c_str());
#else
    (void)x; (void)y; (void)text;
#endif
}

void PrintContext::drawCircle(double cx, double cy, double radius, bool fill) {
#ifdef HAVE_CAIRO
    if (!cr_) return;
    auto* cr = static_cast<cairo_t*>(cr_);
    cairo_arc(cr, cx, cy, radius, 0, 2 * M_PI);
    if (fill) {
        cairo_fill(cr);
    } else {
        cairo_stroke(cr);
    }
#else
    (void)cx; (void)cy; (void)radius; (void)fill;
#endif
}

void PrintContext::renderWidget(Widget* widget, double offsetX, double offsetY) {
#ifdef HAVE_CAIRO
    if (!cr_ || !widget) return;

    auto* cr = static_cast<cairo_t*>(cr_);
    double x = offsetX;
    double y = offsetY;
    double w = static_cast<double>(widget->width());
    double h = static_cast<double>(widget->height());

    // Draw widget background
    setColor(0.95, 0.95, 0.95);
    drawRect(x, y, w, h, true);

    // Draw widget border
    setColor(0.5, 0.5, 0.5);
    setLineWidth(0.5);
    drawRect(x, y, w, h, false);

    // Draw widget name/label
    setColor(0.0, 0.0, 0.0);
    setFontFace("sans-serif", false, false);
    setFontSize(9.0);
    drawText(x + 2, y + 12, widget->name());

    // Render children recursively
    for (auto* child : widget->children()) {
        if (child && child->isRealized()) {
            renderWidget(child,
                         x + static_cast<double>(child->x()),
                         y + static_cast<double>(child->y()));
        }
    }
#else
    (void)widget; (void)offsetX; (void)offsetY;
#endif
}

void PrintContext::printPages(const std::string& outputPath, int totalPages, PageCallback cb) {
    if (!beginJob(outputPath)) return;

    for (int i = 1; i <= totalPages; ++i) {
        if (!beginPage()) break;
        if (!cb(*this, i)) {
            endPage();
            break;
        }
        endPage();
    }

    endJob();
}

} // namespace motif
