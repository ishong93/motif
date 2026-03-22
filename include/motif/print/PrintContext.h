#pragma once

#include <string>
#include <functional>
#include <vector>

namespace motif {

class Widget;

/// Print output backend.
/// Wraps Cairo to produce PDF/PS output from widget trees.
/// Compile-time optional via HAVE_CAIRO.
class PrintContext {
public:
    enum class OutputFormat { PDF, PostScript };
    enum class Orientation { Portrait, Landscape };

    struct PageSetup {
        double paperWidth  = 612.0;  // US Letter in points (8.5")
        double paperHeight = 792.0;  // US Letter in points (11")
        double marginTop    = 36.0;
        double marginBottom = 36.0;
        double marginLeft   = 36.0;
        double marginRight  = 36.0;
        Orientation orientation = Orientation::Portrait;
        OutputFormat format = OutputFormat::PDF;
    };

    /// Callback invoked per page. Return false to stop printing.
    using PageCallback = std::function<bool(PrintContext& ctx, int pageNum)>;

    PrintContext();
    ~PrintContext();

    /// Configure page setup
    void setPageSetup(const PageSetup& setup) { setup_ = setup; }
    const PageSetup& pageSetup() const { return setup_; }

    /// Begin a print job to file
    bool beginJob(const std::string& outputPath);

    /// Begin a new page
    bool beginPage();

    /// End the current page
    void endPage();

    /// Finish the job and close the file
    void endJob();

    bool isActive() const { return active_; }

    /// Drawing API (wraps Cairo)
    void setColor(double r, double g, double b, double a = 1.0);
    void setLineWidth(double w);
    void setFontFace(const std::string& family, bool bold = false, bool italic = false);
    void setFontSize(double size);

    void drawLine(double x1, double y1, double x2, double y2);
    void drawRect(double x, double y, double w, double h, bool fill = false);
    void drawText(double x, double y, const std::string& text);
    void drawCircle(double cx, double cy, double radius, bool fill = false);

    /// High-level: render a widget subtree to print
    void renderWidget(Widget* widget, double offsetX = 0, double offsetY = 0);

    /// Printable area dimensions (paper minus margins)
    double printableWidth() const;
    double printableHeight() const;

    /// Convenience: print using page callback
    void printPages(const std::string& outputPath, int totalPages, PageCallback cb);

private:
    PageSetup setup_;
    bool active_ = false;
    int currentPage_ = 0;

    // Cairo handles (void* to avoid header dependency)
    void* surface_ = nullptr;  // cairo_surface_t*
    void* cr_ = nullptr;       // cairo_t*
};

} // namespace motif
