#pragma once

#include <cstdint>
#include <functional>
#include <memory>
#include <optional>
#include <string>
#include <unordered_map>
#include <variant>
#include <vector>

namespace motif {

class Widget;

// ── UidFile: one compiled UID binary resource database ──────────────
// Replaces the IDB (Indexed Database) layer in legacy Mrm.
// UID files are produced by the UIL compiler and contain widget
// definitions, literals, icons, and callbacks in a B-tree indexed
// binary format with 4096-byte records.

class UidFile {
public:
    ~UidFile();

    // Open a UID file from disk (returns nullptr on failure)
    static std::unique_ptr<UidFile> open(const std::string& path);

    // Open a UID file from an in-memory buffer
    static std::unique_ptr<UidFile> openFromMemory(const uint8_t* data, size_t size);

    const std::string& path() const { return path_; }
    const std::string& dbVersion() const { return dbVersion_; }
    bool isByteSwapped() const { return byteSwapped_; }

    // Fetch raw resource data by string index
    bool fetchIndexed(const std::string& index,
                      std::vector<uint8_t>& dataOut,
                      uint8_t& groupOut, uint8_t& typeOut) const;

    // Check if an index exists
    bool hasIndex(const std::string& index) const;

    // Get all index names in the file
    std::vector<std::string> allIndices() const;

private:
    UidFile() = default;

    bool parseHeader();
    bool readRecord(uint32_t recordNum, std::vector<uint8_t>& buf) const;
    bool searchIndex(uint32_t nodeRecord, const std::string& key,
                     uint32_t& dataRecord, uint32_t& dataOffset) const;
    void collectIndices(uint32_t nodeRecord, std::vector<std::string>& out) const;

    uint32_t read32(const uint8_t* p) const;
    uint16_t read16(const uint8_t* p) const;

    std::string path_;
    std::string dbVersion_;
    std::string creator_;
    bool byteSwapped_ = false;

    // File content (memory-mapped or loaded)
    std::vector<uint8_t> fileData_;

    // Header fields
    uint32_t indexRoot_ = 0;
    uint32_t numIndexed_ = 0;
    uint32_t lastRecord_ = 0;

    static constexpr uint32_t RECORD_SIZE = 4096;
    static constexpr uint32_t HEADER_RECORD = 1;
};

// ── Resource groups and types ───────────────────────────────────────

enum class ResourceGroup : uint8_t {
    Widget   = 1,
    Literal  = 2,
    Callback = 3,
    Max      = 4
};

enum class ResourceType : uint8_t {
    None           = 0,
    Boolean        = 1,
    Char8          = 2,
    Char8Vector    = 3,
    CString        = 4,
    Float          = 5,
    Pixmap         = 6,
    Integer        = 7,
    Null           = 8,
    Callback       = 9,
    Widget         = 10,
    Color          = 11,
    IconImage      = 12,
    Font           = 13,
    FontList       = 14,
    ClassRecord    = 15,
    IntegerVector  = 16,
    XBitmapFile    = 17,
    CountedVector  = 18,
    Keysym         = 19,
    SingleFloat    = 20,
    WideChar       = 21,
    FontSet        = 22,
    TransTable     = 23,
    ClassRecName   = 24,
    IntTable       = 25,
    HorizontalFloat = 26,
    VerticalFloat  = 27,
    HorizontalInt  = 28,
    VerticalInt    = 29
};

// ── WidgetDef: parsed widget definition from UID ────────────────────

struct WidgetDef {
    std::string name;
    std::string className;

    struct Argument {
        std::string name;
        std::variant<bool, int, float, std::string, std::vector<uint8_t>> value;
        ResourceType type = ResourceType::None;
    };

    struct ChildRef {
        std::string name;           // child widget name (index into UID)
        bool managed = true;
    };

    struct CallbackEntry {
        std::string callbackName;   // e.g. "activateCallback"
        std::string procedureName;  // registered procedure name
        std::variant<std::monostate, int, std::string> clientData;
    };

    std::vector<Argument> arguments;
    std::vector<ChildRef> children;
    std::vector<CallbackEntry> callbacks;
};

// ── LiteralValue: a fetched literal ─────────────────────────────────

struct LiteralValue {
    ResourceType type = ResourceType::None;
    std::variant<bool, int, float, std::string, std::vector<uint8_t>> value;
};

// ── ResourceHierarchy: search path of UID files ─────────────────────
// Replaces legacy MrmHierarchy — manages multiple UID files
// searched in priority order.

class ResourceHierarchy {
public:
    ResourceHierarchy() = default;
    ~ResourceHierarchy() = default;

    // Open a hierarchy from a list of UID file paths
    // Files are searched in order; first match wins.
    bool open(const std::vector<std::string>& uidPaths);

    // Close all files
    void close();

    bool isOpen() const { return !files_.empty(); }

    // Fetch a widget definition by index name
    std::optional<WidgetDef> fetchWidget(const std::string& index) const;

    // Fetch a literal value by index name
    std::optional<LiteralValue> fetchLiteral(const std::string& index) const;

    // Check if a resource exists
    bool hasResource(const std::string& index) const;

    // Get all indices across all files
    std::vector<std::string> allIndices() const;

private:
    // Search all files for an indexed resource
    const UidFile* findResource(const std::string& index) const;

    // Parse a raw widget record into WidgetDef
    static std::optional<WidgetDef> parseWidgetRecord(const std::vector<uint8_t>& data);

    // Parse a raw literal record into LiteralValue
    static std::optional<LiteralValue> parseLiteralRecord(const std::vector<uint8_t>& data);

    std::vector<std::unique_ptr<UidFile>> files_;
};

// ── ResourceLoader: high-level API for loading UID resources ────────
// Replaces MrmFetchWidget / MrmFetchSetValues / MrmFetchLiteral.
// Integrates with the motif widget system to instantiate widget trees
// from compiled UIL resources.

class ResourceLoader {
public:
    static ResourceLoader& instance();

    // Callback procedure registry (replaces MrmRegisterNames)
    using ProcedureCallback = std::function<void(Widget*, void* callData)>;

    void registerProcedure(const std::string& name, ProcedureCallback cb);
    ProcedureCallback findProcedure(const std::string& name) const;

    // Widget class creation registry (replaces MrmRegisterClass)
    using WidgetFactory = std::function<Widget*(Widget* parent, const std::string& name)>;

    void registerWidgetClass(const std::string& className, WidgetFactory factory);
    WidgetFactory findWidgetFactory(const std::string& className) const;

    // Open a hierarchy of UID files
    bool openHierarchy(const std::vector<std::string>& uidPaths);

    // Fetch and create a widget tree
    // Returns the top-level created widget, or nullptr on failure.
    Widget* fetchWidget(const std::string& index, Widget* parent);

    // Fetch and apply resource values to an existing widget
    bool fetchSetValues(const std::string& index, Widget* target);

    // Fetch a literal value
    std::optional<LiteralValue> fetchLiteral(const std::string& index);

    // Close the hierarchy
    void close();

private:
    ResourceLoader() = default;

    // Recursively create widget tree from WidgetDef
    Widget* createWidgetTree(const WidgetDef& def, Widget* parent);

    // Apply arguments from WidgetDef to a widget
    void applyArguments(Widget* widget, const std::vector<WidgetDef::Argument>& args);

    // Bind callbacks from WidgetDef to a widget
    void bindCallbacks(Widget* widget, const std::vector<WidgetDef::CallbackEntry>& callbacks);

    ResourceHierarchy hierarchy_;
    std::unordered_map<std::string, ProcedureCallback> procedures_;
    std::unordered_map<std::string, WidgetFactory> widgetFactories_;
};

} // namespace motif
