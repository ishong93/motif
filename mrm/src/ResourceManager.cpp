#include <motif/mrm/ResourceManager.h>
#include <motif/mrm/ResourceLoader.h>
#include <motif/core/Log.h>

namespace motif::mrm {

struct ResourceManager::Impl {
    bool open = false;
};

ResourceManager::ResourceManager() : impl_(std::make_unique<Impl>()) {}
ResourceManager::~ResourceManager() {
    closeHierarchy();
}

bool ResourceManager::openHierarchy(const std::string& path) {
    return openHierarchy(std::vector<std::string>{path});
}

bool ResourceManager::openHierarchy(const std::vector<std::string>& paths) {
    auto& loader = ResourceLoader::instance();

    if (!loader.openHierarchy(paths)) {
        MOTIF_LOG_ERROR << "MRM: failed to open UID hierarchy";
        return false;
    }

    impl_->open = true;
    MOTIF_LOG_INFO << "MRM: opened hierarchy with " << paths.size() << " file(s)";
    return true;
}

void ResourceManager::closeHierarchy() {
    if (impl_->open) {
        ResourceLoader::instance().close();
        impl_->open = false;
    }
}

bool ResourceManager::isOpen() const {
    return impl_->open;
}

Widget* ResourceManager::fetchWidget(const std::string& name, Widget* parent) {
    if (!impl_->open) {
        MOTIF_LOG_ERROR << "MRM: fetchWidget called with no open hierarchy";
        return nullptr;
    }

    auto* widget = ResourceLoader::instance().fetchWidget(name, parent);
    if (!widget) {
        MOTIF_LOG_WARNING << "MRM: widget '" << name << "' not found in hierarchy";
    }
    return widget;
}

bool ResourceManager::fetchSetValues(const std::string& name, Widget* target) {
    if (!impl_->open) {
        MOTIF_LOG_ERROR << "MRM: fetchSetValues called with no open hierarchy";
        return false;
    }
    return ResourceLoader::instance().fetchSetValues(name, target);
}

void ResourceManager::registerProcedure(const std::string& name, ProcedureCallback cb) {
    ResourceLoader::instance().registerProcedure(name, std::move(cb));
}

void ResourceManager::registerBuiltinWidgets() {
    // Delegate to WidgetFactory registration
    // (implemented in WidgetFactory.cpp)
    extern void registerAllBuiltinWidgets();
    registerAllBuiltinWidgets();
}

} // namespace motif::mrm
