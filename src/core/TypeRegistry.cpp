#include <motif/core/TypeRegistry.h>

#include <algorithm>
#include <cctype>

namespace motif {

// ── Utility ─────────────────────────────────────────────────────────

static std::string toLower(const std::string& s) {
    std::string result = s;
    std::transform(result.begin(), result.end(), result.begin(),
                   [](unsigned char c) { return std::tolower(c); });
    return result;
}

// ── RepTypeEntry ────────────────────────────────────────────────────

std::optional<uint8_t> RepTypeEntry::nameToValue(const std::string& valueName) const {
    std::string lower = toLower(valueName);
    for (size_t i = 0; i < valueNames.size(); ++i) {
        if (toLower(valueNames[i]) == lower) {
            return isMapped() ? valueMap[i] : static_cast<uint8_t>(i);
        }
    }
    return std::nullopt;
}

std::optional<std::string> RepTypeEntry::valueToName(uint8_t value) const {
    if (isMapped()) {
        for (size_t i = 0; i < valueMap.size(); ++i) {
            if (valueMap[i] == value && i < valueNames.size()) {
                return valueNames[i];
            }
        }
    } else {
        if (value < valueNames.size()) {
            return valueNames[value];
        }
    }
    return std::nullopt;
}

// ── TypeRegistry ────────────────────────────────────────────────────

TypeRegistry& TypeRegistry::instance() {
    static TypeRegistry reg;
    return reg;
}

RepTypeId TypeRegistry::registerType(const std::string& typeName,
                                     const std::vector<std::string>& valueNames) {
    return registerMappedType(typeName, valueNames, {});
}

RepTypeId TypeRegistry::registerMappedType(const std::string& typeName,
                                            const std::vector<std::string>& valueNames,
                                            const std::vector<uint8_t>& values) {
    std::string key = toLower(typeName);

    // If already registered, return existing ID
    auto it = nameToId_.find(key);
    if (it != nameToId_.end()) return it->second;

    RepTypeId id = static_cast<RepTypeId>(types_.size());
    if (id >= REP_TYPE_INVALID) return REP_TYPE_INVALID;

    RepTypeEntry entry;
    entry.name = typeName;
    entry.valueNames = valueNames;
    entry.valueMap = values;
    entry.id = id;

    types_.push_back(std::move(entry));
    nameToId_[key] = id;
    return id;
}

const RepTypeEntry* TypeRegistry::findType(const std::string& typeName) const {
    auto it = nameToId_.find(toLower(typeName));
    if (it == nameToId_.end()) return nullptr;
    return &types_[it->second];
}

const RepTypeEntry* TypeRegistry::findType(RepTypeId id) const {
    if (id >= types_.size()) return nullptr;
    return &types_[id];
}

RepTypeId TypeRegistry::getId(const std::string& typeName) const {
    auto it = nameToId_.find(toLower(typeName));
    return (it != nameToId_.end()) ? it->second : REP_TYPE_INVALID;
}

std::optional<uint8_t> TypeRegistry::stringToValue(const std::string& typeName,
                                                    const std::string& valueName) const {
    auto* entry = findType(typeName);
    return entry ? entry->nameToValue(valueName) : std::nullopt;
}

std::optional<uint8_t> TypeRegistry::stringToValue(RepTypeId id,
                                                    const std::string& valueName) const {
    auto* entry = findType(id);
    return entry ? entry->nameToValue(valueName) : std::nullopt;
}

std::optional<std::string> TypeRegistry::valueToString(const std::string& typeName,
                                                        uint8_t value) const {
    auto* entry = findType(typeName);
    return entry ? entry->valueToName(value) : std::nullopt;
}

std::optional<std::string> TypeRegistry::valueToString(RepTypeId id,
                                                        uint8_t value) const {
    auto* entry = findType(id);
    return entry ? entry->valueToName(value) : std::nullopt;
}

// ── Built-in Motif representation types ─────────────────────────────

void TypeRegistry::registerBuiltins() {
    if (builtinsRegistered_) return;
    builtinsRegistered_ = true;

    // ── Alignment ──
    registerType("Alignment", {
        "alignment_beginning", "alignment_center", "alignment_end"
    });

    // ── ArrowDirection ──
    registerType("ArrowDirection", {
        "arrow_up", "arrow_down", "arrow_left", "arrow_right"
    });

    // ── ArrowLayout ──
    registerType("ArrowLayout", {
        "arrows_end", "arrows_beginning", "arrows_split",
        "arrows_flat_end", "arrows_flat_beginning"
    });

    // ── Attachment ──
    registerType("Attachment", {
        "attach_none", "attach_form", "attach_opposite_form",
        "attach_widget", "attach_opposite_widget", "attach_position", "attach_self"
    });

    // ── AudibleWarning ──
    registerType("AudibleWarning", {
        "none", "bell"
    });

    // ── BindingType ──
    registerType("BindingType", {
        "none", "pixmap", "solid", "spiral", "pixmap_overlap_only"
    });

    // ── ChildPlacement ──
    registerType("ChildPlacement", {
        "place_top", "place_above_selection", "place_below_selection"
    });

    // ── ChildType (Frame) ──
    registerType("ChildType", {
        "frame_generic_child", "frame_workarea_child", "frame_title_child"
    });

    // ── ChildVerticalAlignment (mapped) ──
    registerMappedType("ChildVerticalAlignment", {
        "alignment_baseline_top", "alignment_center",
        "alignment_baseline_bottom",
        "alignment_widget_top", "alignment_widget_bottom",
        "alignment_child_top", "alignment_child_bottom"
    }, { 0, 1, 2, 3, 4, 5, 6 });

    // ── ComboBoxType ──
    registerType("ComboBoxType", {
        "combo_box", "drop_down_combo_box", "drop_down_list"
    });

    // ── CommandWindowLocation ──
    registerType("CommandWindowLocation", {
        "command_above_workspace", "command_below_workspace"
    });

    // ── DefaultButtonType (mapped) ──
    registerMappedType("DefaultButtonType", {
        "dialog_none", "dialog_cancel_button",
        "dialog_ok_button", "dialog_help_button"
    }, { 0, 2, 1, 3 });

    // ── DeleteResponse ──
    registerType("DeleteResponse", {
        "destroy", "unmap", "do_nothing"
    });

    // ── DialogStyle (mapped) ──
    registerMappedType("DialogStyle", {
        "dialog_modeless", "dialog_work_area",
        "dialog_primary_application_modal",
        "dialog_application_modal",
        "dialog_full_application_modal",
        "dialog_system_modal"
    }, { 0, 0, 1, 1, 2, 3 });

    // ── DialogType ──
    registerType("DialogType", {
        "dialog_template", "dialog_error", "dialog_information",
        "dialog_message", "dialog_question", "dialog_warning", "dialog_working"
    });

    // ── Direction (mapped) ──
    registerMappedType("Direction", {
        "left_to_right", "right_to_left",
        "left_to_right_top_to_bottom", "right_to_left_top_to_bottom",
        "left_to_right_bottom_to_top", "right_to_left_bottom_to_top",
        "top_to_bottom_left_to_right", "top_to_bottom_right_to_left",
        "bottom_to_top_left_to_right", "bottom_to_top_right_to_left"
    }, { 0, 1, 0, 1, 2, 3, 4, 5, 6, 7 });

    // ── EditMode ──
    registerType("EditMode", {
        "multi_line_edit", "single_line_edit"
    });

    // ── FileTypeMask ──
    registerType("FileTypeMask", {
        "file_regular", "file_directory", "file_any_type"
    });

    // ── FontType ──
    registerType("FontType", {
        "font_is_font", "font_is_fontset", "font_is_xft", "as_is"
    });

    // ── IndicatorType ──
    registerType("IndicatorType", {
        "n_of_many", "one_of_many", "one_of_many_round",
        "one_of_many_diamond"
    });

    // ── KeyboardFocusPolicy ──
    registerType("KeyboardFocusPolicy", {
        "explicit", "pointer"
    });

    // ── LabelType ──
    registerType("LabelType", {
        "string", "pixmap"
    });

    // ── LayoutType ──
    registerType("LayoutType", {
        "outline", "spatial", "detail"
    });

    // ── LineStyle ──
    registerType("LineStyle", {
        "line_solid", "line_on_off_dash", "line_double_dash"
    });

    // ── ListSizePolicy ──
    registerType("ListSizePolicy", {
        "variable", "constant", "resize_if_possible"
    });

    // ── MultiClick ──
    registerType("MultiClick", {
        "multiclick_discard", "multiclick_keep"
    });

    // ── NavigationType ──
    registerType("NavigationType", {
        "none", "tab_group", "sticky_tab_group", "exclusive_tab_group"
    });

    // ── Orientation ──
    registerType("Orientation", {
        "no_orientation", "vertical", "horizontal"
    });

    // ── OutlineState ──
    registerType("OutlineState", {
        "collapsed", "expanded"
    });

    // ── Packing ──
    registerType("Packing", {
        "no_packing", "pack_tight", "pack_column", "pack_none"
    });

    // ── ProcessingDirection ──
    registerType("ProcessingDirection", {
        "max_on_top", "max_on_bottom", "max_on_left", "max_on_right"
    });

    // ── ResizePolicy ──
    registerType("ResizePolicy", {
        "resize_none", "resize_grow", "resize_any"
    });

    // ── RowColumnType ──
    registerType("RowColumnType", {
        "work_area", "menu_bar", "menu_pulldown",
        "menu_popup", "menu_option"
    });

    // ── ScrollBarDisplayPolicy ──
    registerType("ScrollBarDisplayPolicy", {
        "static", "as_needed"
    });

    // ── ScrollBarPlacement ──
    registerType("ScrollBarPlacement", {
        "bottom_right", "top_right", "bottom_left", "top_left"
    });

    // ── ScrollingPolicy ──
    registerType("ScrollingPolicy", {
        "automatic", "application_defined"
    });

    // ── SelectionPolicy ──
    registerType("SelectionPolicy", {
        "single_select", "multiple_select",
        "extended_select", "browse_select"
    });

    // ── SelectionType ──
    registerType("SelectionType", {
        "dialog_work_area", "dialog_prompt",
        "dialog_selection", "dialog_command",
        "dialog_file_selection"
    });

    // ── SeparatorType ──
    registerType("SeparatorType", {
        "no_line", "single_line", "double_line",
        "single_dashed_line", "double_dashed_line",
        "shadow_etched_in", "shadow_etched_out",
        "shadow_etched_in_dash", "shadow_etched_out_dash"
    });

    // ── ShadowType ──
    registerType("ShadowType", {
        "shadow_etched_in", "shadow_etched_out",
        "shadow_in", "shadow_out"
    });

    // ── ShowArrows ──
    registerType("ShowArrows", {
        "none", "each_side", "max_side", "min_side"
    });

    // ── ShowValue ──
    registerType("ShowValue", {
        "none", "near_slider", "near_border"
    });

    // ── SlidingMode ──
    registerType("SlidingMode", {
        "slider", "thermometer"
    });

    // ── StringDirection ──
    registerType("StringDirection", {
        "string_direction_l_to_r", "string_direction_r_to_l"
    });

    // ── TearOffModel ──
    registerType("TearOffModel", {
        "tear_off_enabled", "tear_off_disabled"
    });

    // ── ToggleMode ──
    registerType("ToggleMode", {
        "toggle_boolean", "toggle_indeterminate"
    });

    // ── UnitType ──
    registerType("UnitType", {
        "pixels", "100th_millimeters", "1000th_inches",
        "100th_points", "100th_font_units", "inches",
        "centimeters", "millimeters", "points", "font_units"
    });

    // ── UnpostBehavior ──
    registerType("UnpostBehavior", {
        "unpost", "unpost_and_replay"
    });

    // ── VerticalAlignment ──
    registerType("VerticalAlignment", {
        "alignment_baseline_top", "alignment_center",
        "alignment_baseline_bottom",
        "alignment_contents_top", "alignment_contents_bottom"
    });

    // ── ViewType ──
    registerType("ViewType", {
        "large_icon", "small_icon"
    });

    // ── VisualEmphasis ──
    registerType("VisualEmphasis", {
        "selected", "not_selected"
    });

    // ── SpatialStyle ──
    registerType("SpatialStyle", {
        "none", "grid", "cells"
    });

    // ── SpatialIncludeModel ──
    registerType("SpatialIncludeModel", {
        "append", "closest", "first_fit"
    });

    // ── SpatialResizeModel ──
    registerType("SpatialResizeModel", {
        "grow_minor", "grow_major", "grow_balanced"
    });

    // ── SpatialSnapModel ──
    registerType("SpatialSnapModel", {
        "none", "snap_to_grid", "center"
    });

    // ── SpinBoxChildType ──
    registerType("SpinBoxChildType", {
        "string", "numeric"
    });

    // ── InputPolicy ──
    registerType("InputPolicy", {
        "per_shell", "per_widget", "inherit_policy"
    });

    // ── AutoDragModel ──
    registerType("AutoDragModel", {
        "auto_drag_enabled", "auto_drag_disabled"
    });

    // ── BlendModel ──
    registerType("BlendModel", {
        "blend_all", "blend_state_source", "blend_just_source", "blend_none"
    });

    // ── DropSiteActivity ──
    registerType("DropSiteActivity", {
        "drop_site_active", "drop_site_inactive",
        "drop_site_ignore"
    });

    // ── DropSiteType ──
    registerType("DropSiteType", {
        "drop_site_simple", "drop_site_composite"
    });

    // ── PathMode ──
    registerType("PathMode", {
        "path_mode_full", "path_mode_relative"
    });

    // ── PositionMode ──
    registerType("PositionMode", {
        "zero_based", "one_based"
    });

    // ── AnimationStyle ──
    registerType("AnimationStyle", {
        "drag_under_none", "drag_under_pixmap",
        "drag_under_shadow_in", "drag_under_shadow_out",
        "drag_under_highlight"
    });

    // ── BitmapConversionModel ──
    registerType("BitmapConversionModel", {
        "match_depth", "dynamic_depth"
    });

    // ── NbChildType ──
    registerType("NbChildType", {
        "none", "page", "major_tab", "minor_tab",
        "status_area", "page_scroller"
    });

    // ── OutlineButtonPolicy ──
    registerType("OutlineButtonPolicy", {
        "outline_button_present", "outline_button_absent"
    });

    // ── PrimaryOwnership ──
    registerType("PrimaryOwnership", {
        "own_never", "own_always", "own_multiple",
        "own_possible_multiple"
    });

    // ── SelectionTechnique ──
    registerType("SelectionTechnique", {
        "marquee", "marquee_extend_start", "marquee_extend_both",
        "touch_only", "touch_over"
    });

    // ── MatchBehavior ──
    registerType("MatchBehavior", {
        "none", "quick_navigate"
    });

    // ── DefaultButtonEmphasis ──
    registerType("DefaultButtonEmphasis", {
        "external_highlight", "internal_highlight"
    });

    // ── EnableWarp ──
    registerType("EnableWarp", {
        "enable_warp_on", "enable_warp_off"
    });

    // ── EntryViewType ──
    registerType("EntryViewType", {
        "large_icon", "small_icon", "any_icon"
    });

    // ── FileFilterStyle ──
    registerType("FileFilterStyle", {
        "filter_none", "filter_hidden_files"
    });

    // ── IconAttachment ──
    registerType("IconAttachment", {
        "icon_top", "icon_bottom", "icon_left", "icon_right"
    });

    // ── ArrowSensitivity ──
    registerType("ArrowSensitivity", {
        "arrows_insensitive", "arrows_increment_sensitive",
        "arrows_decrement_sensitive", "arrows_sensitive",
        "arrows_default_sensitivity"
    });

    // ── ArrowOrientation ──
    registerType("ArrowOrientation", {
        "arrows_vertical", "arrows_horizontal"
    });

    // ── AutomaticSelection (mapped) ──
    registerMappedType("AutomaticSelection", {
        "no_auto_select", "auto_select",
        "off", "false", "no", "0",
        "on", "true", "yes", "1"
    }, { 0, 1, 0, 0, 0, 0, 1, 1, 1, 1 });

    // ── IndicatorOn (mapped) ──
    registerMappedType("IndicatorOn", {
        "indicator_none", "indicator_fill", "indicator_box",
        "indicator_check", "indicator_check_box", "indicator_cross",
        "indicator_cross_box",
        "off", "false", "no", "0",
        "on", "true", "yes", "1"
    }, { 0, 1, 2, 3, 4, 5, 6, 0, 0, 0, 0, 1, 1, 1, 1 });

    // ── Set (mapped: boolean-like) ──
    registerMappedType("Set", {
        "unset", "set", "indeterminate",
        "off", "false", "no", "0",
        "on", "true", "yes", "1"
    }, { 0, 1, 2, 0, 0, 0, 0, 1, 1, 1, 1 });

    // ── SliderMark ──
    registerType("SliderMark", {
        "none", "etched", "thumb_mark", "round_mark"
    });

    // ── SliderVisual ──
    registerType("SliderVisual", {
        "background", "foreground", "trough_color",
        "shadowed_background"
    });

    // ── LoadModel ──
    registerType("LoadModel", {
        "load_deferred", "load_immediate"
    });

    // ── EnableBtn1Transfer ──
    registerMappedType("EnableBtn1Transfer", {
        "off", "button2_adjust", "button2_transfer",
        "false", "no", "0",
        "true", "yes", "1"
    }, { 0, 1, 2, 0, 0, 0, 1, 1, 1 });

    // ── PositionType ──
    registerType("PositionType", {
        "position_index", "position_value"
    });

    // ── LineType ──
    registerType("LineType", {
        "no_line", "single_line", "double_line",
        "single_dashed_line", "double_dashed_line"
    });

    // ── ScrolledWindowChildType ──
    registerType("ScrolledWindowChildType", {
        "work_area", "scroll_hor", "scroll_vert",
        "no_scroll", "clip_window", "generic_child"
    });
}

} // namespace motif
