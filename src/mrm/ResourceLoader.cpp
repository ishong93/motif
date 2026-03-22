#include <motif/mrm/ResourceLoader.h>
#include <motif/core/Widget.h>

#include <algorithm>
#include <cstring>
#include <fstream>

namespace motif {

// ── UidFile ─────────────────────────────────────────────────────────

// IDB record/field validation constants (from legacy IDB.h)
static constexpr uint32_t IDB_RECORD_HEADER_VALID = 0xBEADCADE;
static constexpr uint32_t IDB_DATA_ENTRY_VALID    = 0xCEADCADE;

// Record types
static constexpr uint8_t IDB_RT_HEADER     = 1;
static constexpr uint8_t IDB_RT_INDEX_LEAF = 2;
static constexpr uint8_t IDB_RT_INDEX_NODE = 3;
static constexpr uint8_t IDB_RT_RID_MAP    = 4;
static constexpr uint8_t IDB_RT_DATA       = 5;

// Widget record validation
static constexpr uint32_t URM_WIDGET_RECORD_VALID = 0xABCDABCD;

UidFile::~UidFile() = default;

std::unique_ptr<UidFile> UidFile::open(const std::string& path) {
    std::ifstream file(path, std::ios::binary | std::ios::ate);
    if (!file.is_open()) return nullptr;

    auto size = file.tellg();
    if (size < static_cast<std::streamoff>(RECORD_SIZE * 2)) return nullptr;

    file.seekg(0, std::ios::beg);

    auto uid = std::unique_ptr<UidFile>(new UidFile());
    uid->path_ = path;
    uid->fileData_.resize(static_cast<size_t>(size));
    file.read(reinterpret_cast<char*>(uid->fileData_.data()),
              static_cast<std::streamsize>(size));

    if (!uid->parseHeader()) return nullptr;
    return uid;
}

std::unique_ptr<UidFile> UidFile::openFromMemory(const uint8_t* data, size_t size) {
    if (!data || size < RECORD_SIZE * 2) return nullptr;

    auto uid = std::unique_ptr<UidFile>(new UidFile());
    uid->path_ = "<memory>";
    uid->fileData_.assign(data, data + size);

    if (!uid->parseHeader()) return nullptr;
    return uid;
}

bool UidFile::parseHeader() {
    std::vector<uint8_t> rec;
    if (!readRecord(HEADER_RECORD, rec)) return false;

    // Record header: validation(4) + record_type(2) + record_num(4)
    uint32_t validation = read32(rec.data());
    if (validation != IDB_RECORD_HEADER_VALID) {
        // Try byte-swapped
        byteSwapped_ = true;
        validation = read32(rec.data());
        if (validation != IDB_RECORD_HEADER_VALID) return false;
    }

    // After record header (10 bytes): header-specific fields
    // db_version at offset 10, length 10
    size_t off = 10;
    dbVersion_.assign(reinterpret_cast<const char*>(rec.data() + off), 10);
    dbVersion_.erase(dbVersion_.find('\0'));
    off += 10;

    // creator at offset 20, length 30
    creator_.assign(reinterpret_cast<const char*>(rec.data() + off), 30);
    creator_.erase(creator_.find('\0'));
    off += 30;

    // creator_version(10) + creation_date(30) + module(30) + module_version(10)
    off += 10 + 30 + 30 + 10;

    // index_root (4 bytes)
    indexRoot_ = read32(rec.data() + off); off += 4;
    // num_indexed (4 bytes)
    numIndexed_ = read32(rec.data() + off); off += 4;
    // num_RID (4 bytes)
    off += 4;
    // next_RID (8 bytes)
    off += 8;
    // last_record (4 bytes)
    lastRecord_ = read32(rec.data() + off);

    return indexRoot_ > 0;
}

bool UidFile::readRecord(uint32_t recordNum, std::vector<uint8_t>& buf) const {
    if (recordNum == 0) return false;
    size_t offset = static_cast<size_t>(recordNum - 1) * RECORD_SIZE;
    if (offset + RECORD_SIZE > fileData_.size()) return false;

    buf.assign(fileData_.begin() + offset,
               fileData_.begin() + offset + RECORD_SIZE);
    return true;
}

uint32_t UidFile::read32(const uint8_t* p) const {
    uint32_t val;
    std::memcpy(&val, p, 4);
    if (byteSwapped_) {
        val = ((val >> 24) & 0xFF) |
              ((val >> 8) & 0xFF00) |
              ((val << 8) & 0xFF0000) |
              ((val << 24) & 0xFF000000);
    }
    return val;
}

uint16_t UidFile::read16(const uint8_t* p) const {
    uint16_t val;
    std::memcpy(&val, p, 2);
    if (byteSwapped_) {
        val = static_cast<uint16_t>((val >> 8) | (val << 8));
    }
    return val;
}

bool UidFile::searchIndex(uint32_t nodeRecord, const std::string& key,
                           uint32_t& dataRecord, uint32_t& dataOffset) const {
    if (nodeRecord == 0) return false;

    std::vector<uint8_t> rec;
    if (!readRecord(nodeRecord, rec)) return false;

    uint32_t validation = read32(rec.data());
    if (validation != IDB_RECORD_HEADER_VALID) return false;

    uint8_t recordType = rec[4];

    if (recordType == IDB_RT_INDEX_LEAF) {
        // Leaf node: scan entries for matching key
        // Layout: header(10) + num_entries(2) + heap_start(2) + free_bytes(2)
        // then entry table: [string_offset(2), data_record(4), data_offset(4)] per entry
        uint16_t numEntries = read16(rec.data() + 10);
        size_t entryStart = 16;

        for (uint16_t i = 0; i < numEntries; ++i) {
            size_t eoff = entryStart + i * 10;
            if (eoff + 10 > RECORD_SIZE) break;

            uint16_t strOff = read16(rec.data() + eoff);
            uint32_t dRec   = read32(rec.data() + eoff + 2);
            uint32_t dOff   = read32(rec.data() + eoff + 6);

            if (strOff >= RECORD_SIZE) continue;

            // String is null-terminated at strOff within the record
            std::string entryKey(reinterpret_cast<const char*>(rec.data() + strOff));

            if (entryKey == key) {
                dataRecord = dRec;
                dataOffset = dOff;
                return true;
            }
        }
        return false;

    } else if (recordType == IDB_RT_INDEX_NODE) {
        // Internal node: binary search, descend into children
        // Layout: header(10) + num_entries(2) + child pointers + keys
        uint16_t numEntries = read16(rec.data() + 10);
        size_t entryStart = 14;

        // Each entry: key_offset(2) + left_child(4)
        // Plus one extra right child pointer at the end
        uint32_t child = 0;

        for (uint16_t i = 0; i < numEntries; ++i) {
            size_t eoff = entryStart + i * 6;
            if (eoff + 6 > RECORD_SIZE) break;

            uint16_t keyOff    = read16(rec.data() + eoff);
            uint32_t leftChild = read32(rec.data() + eoff + 2);

            if (keyOff >= RECORD_SIZE) continue;

            std::string nodeKey(reinterpret_cast<const char*>(rec.data() + keyOff));
            int cmp = key.compare(nodeKey);

            if (cmp == 0) {
                // Exact match — descend left to find the data
                child = leftChild;
                break;
            } else if (cmp < 0) {
                child = leftChild;
                break;
            }

            // If last entry, use right child
            if (i == numEntries - 1) {
                size_t rightOff = entryStart + numEntries * 6;
                if (rightOff + 4 <= RECORD_SIZE) {
                    child = read32(rec.data() + rightOff);
                }
            }
        }

        if (child > 0) {
            return searchIndex(child, key, dataRecord, dataOffset);
        }
        return false;
    }

    return false;
}

bool UidFile::fetchIndexed(const std::string& index,
                            std::vector<uint8_t>& dataOut,
                            uint8_t& groupOut, uint8_t& typeOut) const {
    uint32_t dataRecord = 0, dataOffset = 0;
    if (!searchIndex(indexRoot_, index, dataRecord, dataOffset)) return false;

    std::vector<uint8_t> rec;
    if (!readRecord(dataRecord, rec)) return false;

    // Data entry header at dataOffset:
    //   validation(4) + entry_type(2) + resource_group(2) + resource_type(2)
    //   + access(2) + lock(2) + entry_size(4) + prev_entry(4)
    //   Total: 20 bytes
    if (dataOffset + 20 > RECORD_SIZE) return false;

    const uint8_t* ep = rec.data() + dataOffset;
    uint32_t entryValid = read32(ep);
    if (entryValid != IDB_DATA_ENTRY_VALID) return false;

    groupOut = static_cast<uint8_t>(read16(ep + 6));
    typeOut  = static_cast<uint8_t>(read16(ep + 8));
    uint32_t entrySize = read32(ep + 14);

    size_t dataStart = dataOffset + 20;
    if (dataStart + entrySize > RECORD_SIZE) {
        // Entry spans multiple records — simplified: read what fits
        entrySize = static_cast<uint32_t>(RECORD_SIZE - dataStart);
    }

    dataOut.assign(rec.begin() + dataStart,
                   rec.begin() + dataStart + entrySize);
    return true;
}

bool UidFile::hasIndex(const std::string& index) const {
    uint32_t dr = 0, do_ = 0;
    return searchIndex(indexRoot_, index, dr, do_);
}

void UidFile::collectIndices(uint32_t nodeRecord,
                              std::vector<std::string>& out) const {
    if (nodeRecord == 0) return;

    std::vector<uint8_t> rec;
    if (!readRecord(nodeRecord, rec)) return;

    uint32_t validation = read32(rec.data());
    if (validation != IDB_RECORD_HEADER_VALID) return;

    uint8_t recordType = rec[4];

    if (recordType == IDB_RT_INDEX_LEAF) {
        uint16_t numEntries = read16(rec.data() + 10);
        size_t entryStart = 16;
        for (uint16_t i = 0; i < numEntries; ++i) {
            size_t eoff = entryStart + i * 10;
            if (eoff + 2 > RECORD_SIZE) break;
            uint16_t strOff = read16(rec.data() + eoff);
            if (strOff < RECORD_SIZE) {
                out.emplace_back(reinterpret_cast<const char*>(rec.data() + strOff));
            }
        }
    } else if (recordType == IDB_RT_INDEX_NODE) {
        uint16_t numEntries = read16(rec.data() + 10);
        size_t entryStart = 14;
        for (uint16_t i = 0; i < numEntries; ++i) {
            size_t eoff = entryStart + i * 6;
            if (eoff + 6 > RECORD_SIZE) break;
            uint32_t leftChild = read32(rec.data() + eoff + 2);
            collectIndices(leftChild, out);

            uint16_t keyOff = read16(rec.data() + eoff);
            if (keyOff < RECORD_SIZE) {
                out.emplace_back(reinterpret_cast<const char*>(rec.data() + keyOff));
            }
        }
        // Right-most child
        size_t rightOff = entryStart + numEntries * 6;
        if (rightOff + 4 <= RECORD_SIZE) {
            uint32_t rightChild = read32(rec.data() + rightOff);
            collectIndices(rightChild, out);
        }
    }
}

std::vector<std::string> UidFile::allIndices() const {
    std::vector<std::string> result;
    collectIndices(indexRoot_, result);
    return result;
}

// ── ResourceHierarchy ───────────────────────────────────────────────

bool ResourceHierarchy::open(const std::vector<std::string>& uidPaths) {
    close();
    for (const auto& path : uidPaths) {
        auto file = UidFile::open(path);
        if (file) {
            files_.push_back(std::move(file));
        }
    }
    return !files_.empty();
}

void ResourceHierarchy::close() {
    files_.clear();
}

const UidFile* ResourceHierarchy::findResource(const std::string& index) const {
    for (const auto& file : files_) {
        if (file->hasIndex(index)) return file.get();
    }
    return nullptr;
}

bool ResourceHierarchy::hasResource(const std::string& index) const {
    return findResource(index) != nullptr;
}

std::vector<std::string> ResourceHierarchy::allIndices() const {
    std::vector<std::string> result;
    for (const auto& file : files_) {
        auto indices = file->allIndices();
        result.insert(result.end(), indices.begin(), indices.end());
    }
    // Remove duplicates
    std::sort(result.begin(), result.end());
    result.erase(std::unique(result.begin(), result.end()), result.end());
    return result;
}

std::optional<WidgetDef> ResourceHierarchy::fetchWidget(const std::string& index) const {
    for (const auto& file : files_) {
        std::vector<uint8_t> data;
        uint8_t group, type;
        if (file->fetchIndexed(index, data, group, type)) {
            if (group == static_cast<uint8_t>(ResourceGroup::Widget)) {
                return parseWidgetRecord(data);
            }
        }
    }
    return std::nullopt;
}

std::optional<LiteralValue> ResourceHierarchy::fetchLiteral(const std::string& index) const {
    for (const auto& file : files_) {
        std::vector<uint8_t> data;
        uint8_t group, type;
        if (file->fetchIndexed(index, data, group, type)) {
            if (group == static_cast<uint8_t>(ResourceGroup::Literal)) {
                return parseLiteralRecord(data);
            }
        }
    }
    return std::nullopt;
}

std::optional<WidgetDef> ResourceHierarchy::parseWidgetRecord(const std::vector<uint8_t>& data) {
    // Widget record format (simplified from legacy RGMWidgetRecord):
    //   validation(4) + size(4) + access(2) + lock(2) + type(2)
    //   + name_offs(4) + class_offs(4) + arglist_offs(4) + children_offs(4)
    //   + comment_offs(4) + creation_offs(4) + variety(4)
    // Total fixed header: 38 bytes

    if (data.size() < 38) return std::nullopt;

    const uint8_t* p = data.data();

    uint32_t validation;
    std::memcpy(&validation, p, 4);
    if (validation != URM_WIDGET_RECORD_VALID) return std::nullopt;

    uint32_t nameOff, classOff, argOff, childOff;
    std::memcpy(&nameOff, p + 14, 4);
    std::memcpy(&classOff, p + 18, 4);
    std::memcpy(&argOff, p + 22, 4);
    std::memcpy(&childOff, p + 26, 4);

    WidgetDef def;

    // Read name
    if (nameOff > 0 && nameOff < data.size()) {
        def.name = reinterpret_cast<const char*>(p + nameOff);
    }

    // Read class name
    if (classOff > 0 && classOff < data.size()) {
        def.className = reinterpret_cast<const char*>(p + classOff);
    }

    // Argument list parsing (simplified)
    if (argOff > 0 && argOff + 4 < data.size()) {
        uint16_t numArgs;
        std::memcpy(&numArgs, p + argOff, 2);

        size_t aoff = argOff + 4;
        for (uint16_t i = 0; i < numArgs && aoff + 12 < data.size(); ++i) {
            WidgetDef::Argument arg;

            uint16_t tagOff;
            std::memcpy(&tagOff, p + aoff, 2);
            if (tagOff > 0 && tagOff < data.size()) {
                arg.name = reinterpret_cast<const char*>(p + tagOff);
            }

            uint8_t valType = p[aoff + 2];
            arg.type = static_cast<ResourceType>(valType);

            // Value at aoff + 4, length depends on type
            if (valType == static_cast<uint8_t>(ResourceType::Integer)) {
                int32_t val;
                std::memcpy(&val, p + aoff + 4, 4);
                arg.value = static_cast<int>(val);
            } else if (valType == static_cast<uint8_t>(ResourceType::Boolean)) {
                arg.value = (p[aoff + 4] != 0);
            } else if (valType == static_cast<uint8_t>(ResourceType::CString) ||
                       valType == static_cast<uint8_t>(ResourceType::Char8)) {
                uint32_t strOff;
                std::memcpy(&strOff, p + aoff + 4, 4);
                if (strOff > 0 && strOff < data.size()) {
                    arg.value = std::string(reinterpret_cast<const char*>(p + strOff));
                }
            } else if (valType == static_cast<uint8_t>(ResourceType::Float) ||
                       valType == static_cast<uint8_t>(ResourceType::SingleFloat)) {
                float val;
                std::memcpy(&val, p + aoff + 4, 4);
                arg.value = val;
            }

            def.arguments.push_back(std::move(arg));
            aoff += 12; // fixed argument entry size
        }
    }

    // Children list parsing (simplified)
    if (childOff > 0 && childOff + 4 < data.size()) {
        uint16_t numChildren;
        std::memcpy(&numChildren, p + childOff, 2);

        size_t coff = childOff + 4;
        for (uint16_t i = 0; i < numChildren && coff + 8 < data.size(); ++i) {
            WidgetDef::ChildRef child;

            uint32_t nameOff2;
            std::memcpy(&nameOff2, p + coff, 4);
            if (nameOff2 > 0 && nameOff2 < data.size()) {
                child.name = reinterpret_cast<const char*>(p + nameOff2);
            }

            child.managed = (p[coff + 4] != 0);
            def.children.push_back(std::move(child));
            coff += 8;
        }
    }

    return def;
}

std::optional<LiteralValue> ResourceHierarchy::parseLiteralRecord(const std::vector<uint8_t>& data) {
    if (data.size() < 4) return std::nullopt;

    LiteralValue lit;

    // Literal data is raw — type was in the entry header
    // Simplified: treat as string or raw bytes
    uint8_t typeCode = data[0];
    lit.type = static_cast<ResourceType>(typeCode);

    const uint8_t* payload = data.data() + 1;
    size_t payloadSize = data.size() - 1;

    switch (lit.type) {
        case ResourceType::Boolean:
            if (payloadSize >= 1) lit.value = (payload[0] != 0);
            break;
        case ResourceType::Integer:
            if (payloadSize >= 4) {
                int32_t val;
                std::memcpy(&val, payload, 4);
                lit.value = static_cast<int>(val);
            }
            break;
        case ResourceType::Float:
        case ResourceType::SingleFloat:
            if (payloadSize >= 4) {
                float val;
                std::memcpy(&val, payload, 4);
                lit.value = val;
            }
            break;
        case ResourceType::CString:
        case ResourceType::Char8:
            lit.value = std::string(reinterpret_cast<const char*>(payload), payloadSize);
            break;
        default:
            lit.value = std::vector<uint8_t>(payload, payload + payloadSize);
            break;
    }

    return lit;
}

// ── ResourceLoader ──────────────────────────────────────────────────

ResourceLoader& ResourceLoader::instance() {
    static ResourceLoader loader;
    return loader;
}

void ResourceLoader::registerProcedure(const std::string& name, ProcedureCallback cb) {
    procedures_[name] = std::move(cb);
}

ResourceLoader::ProcedureCallback ResourceLoader::findProcedure(const std::string& name) const {
    auto it = procedures_.find(name);
    return (it != procedures_.end()) ? it->second : nullptr;
}

void ResourceLoader::registerWidgetClass(const std::string& className, WidgetFactory factory) {
    widgetFactories_[className] = std::move(factory);
}

ResourceLoader::WidgetFactory ResourceLoader::findWidgetFactory(const std::string& className) const {
    auto it = widgetFactories_.find(className);
    return (it != widgetFactories_.end()) ? it->second : nullptr;
}

bool ResourceLoader::openHierarchy(const std::vector<std::string>& uidPaths) {
    return hierarchy_.open(uidPaths);
}

void ResourceLoader::close() {
    hierarchy_.close();
}

std::optional<LiteralValue> ResourceLoader::fetchLiteral(const std::string& index) {
    return hierarchy_.fetchLiteral(index);
}

Widget* ResourceLoader::fetchWidget(const std::string& index, Widget* parent) {
    auto def = hierarchy_.fetchWidget(index);
    if (!def) return nullptr;
    return createWidgetTree(*def, parent);
}

bool ResourceLoader::fetchSetValues(const std::string& index, Widget* target) {
    if (!target) return false;
    auto def = hierarchy_.fetchWidget(index);
    if (!def) return false;
    applyArguments(target, def->arguments);
    bindCallbacks(target, def->callbacks);
    return true;
}

Widget* ResourceLoader::createWidgetTree(const WidgetDef& def, Widget* parent) {
    // Find factory for this widget class
    auto factory = findWidgetFactory(def.className);
    if (!factory) return nullptr;

    Widget* widget = factory(parent, def.name);
    if (!widget) return nullptr;

    // Apply arguments
    applyArguments(widget, def.arguments);

    // Bind callbacks
    bindCallbacks(widget, def.callbacks);

    // Create children recursively
    for (const auto& childRef : def.children) {
        auto childDef = hierarchy_.fetchWidget(childRef.name);
        if (childDef) {
            Widget* child = createWidgetTree(*childDef, widget);
            if (child && childRef.managed) {
                child->manage();
            }
        }
    }

    return widget;
}

void ResourceLoader::applyArguments(Widget* widget,
                                     const std::vector<WidgetDef::Argument>& args) {
    if (!widget) return;

    for (const auto& arg : args) {
        std::visit([&](const auto& val) {
            using T = std::decay_t<decltype(val)>;
            if constexpr (std::is_same_v<T, bool>) {
                widget->setResource(arg.name, val);
            } else if constexpr (std::is_same_v<T, int>) {
                widget->setResource(arg.name, val);
            } else if constexpr (std::is_same_v<T, float>) {
                widget->setResource(arg.name, static_cast<double>(val));
            } else if constexpr (std::is_same_v<T, std::string>) {
                widget->setResource(arg.name, val);
            }
            // vector<uint8_t> — skip for now (binary data)
        }, arg.value);
    }
}

void ResourceLoader::bindCallbacks(Widget* widget,
                                    const std::vector<WidgetDef::CallbackEntry>& callbacks) {
    if (!widget) return;

    for (const auto& cb : callbacks) {
        auto proc = findProcedure(cb.procedureName);
        if (proc) {
            widget->addCallback(cb.callbackName, [proc](Widget* w, void* callData) {
                proc(w, callData);
            });
        }
    }
}

} // namespace motif
