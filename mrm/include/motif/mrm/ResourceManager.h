#pragma once

#include <memory>
#include <string>
#include <vector>
#include <functional>

namespace motif {

class Widget;

namespace mrm {

/// High-level MRM Resource Manager API.
/// Wraps ResourceLoader and provides the traditional MRM interface
/// (MrmOpenHierarchy / MrmFetchWidget / MrmCloseHierarchy).
class ResourceManager {
public:
    ResourceManager();
    ~ResourceManager();

    /// Open a UID file hierarchy (one or more .uid paths)
    bool openHierarchy(const std::string& path);
    bool openHierarchy(const std::vector<std::string>& paths);

    /// Close all open hierarchies
    void closeHierarchy();

    /// Is a hierarchy currently open?
    bool isOpen() const;

    /// Fetch and instantiate a widget tree by index name
    /// Returns nullptr on failure.
    Widget* fetchWidget(const std::string& name, Widget* parent);

    /// Fetch and apply resource values to an existing widget
    bool fetchSetValues(const std::string& name, Widget* target);

    /// Register a callback procedure by name (MrmRegisterNames equivalent)
    using ProcedureCallback = std::function<void(Widget*, void*)>;
    void registerProcedure(const std::string& name, ProcedureCallback cb);

    /// Register all built-in Motif widget factories
    void registerBuiltinWidgets();

private:
    struct Impl;
    std::unique_ptr<Impl> impl_;
};

} // namespace mrm
} // namespace motif
