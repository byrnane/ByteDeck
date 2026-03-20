#pragma once

#include "platform/translation_catalog.hpp"
#include "ui/screen.hpp"

#include <functional>
#include <string>
#include <utility>
#include <vector>

namespace bytedeck::ui
{
class SettingsScreen final : public Screen
{
public:
    using ApplyValueCallback = std::function<bool(const std::string&)>;
    using RescanCallback = std::function<std::pair<bool, std::string>()>;

    SettingsScreen(
        const platform::TranslationCatalog& translations,
        std::string current_language,
        std::vector<std::string> available_themes,
        std::string current_theme,
        ApplyValueCallback apply_language,
        ApplyValueCallback apply_theme,
        RescanCallback rescan_library);

    ScreenAction handle_event(const SDL_Event& event) override;
    std::string screen_id() const override;
    UiBindings build_bindings() const override;
    std::string window_title() const override;

private:
    enum class SettingItem
    {
        language,
        theme,
        rescan
    };

    enum class FocusArea
    {
        left,
        right
    };

    enum class EditorMode
    {
        list,
        action_result
    };

    struct EditorOption
    {
        std::string value;
        std::string title;
        std::string description;
        bool current = false;
    };

    void move_selection(int delta);
    void move_editor_selection(int delta);
    void enter_editor();
    void leave_editor();
    void apply_editor_selection();
    void run_rescan();
    void set_status(std::string message, bool success, EditorMode mode = EditorMode::list);
    void sync_editor_selection_to_current_value();
    std::vector<EditorOption> editor_options() const;
    std::string setting_key(SettingItem item) const;
    std::string display_theme_name(const std::string& theme_id) const;
    std::string translated_or_fallback(const std::string& key, const std::string& fallback) const;
    std::string setting_value_text(SettingItem item) const;
    std::string setting_hint_text(std::size_t index) const;
    std::string editor_title() const;
    std::string editor_description() const;
    std::string editor_hint() const;

    const platform::TranslationCatalog& translations_;
    std::vector<SettingItem> items_ {
        SettingItem::language,
        SettingItem::theme,
        SettingItem::rescan
    };
    std::string current_language_;
    std::vector<std::string> available_themes_;
    std::string current_theme_;
    ApplyValueCallback apply_language_;
    ApplyValueCallback apply_theme_;
    RescanCallback rescan_library_;
    FocusArea focus_area_ = FocusArea::left;
    EditorMode editor_mode_ = EditorMode::list;
    std::size_t selected_index_ = 0;
    std::size_t editor_selection_index_ = 0;
    std::string status_message_;
    bool status_success_ = false;
};
}
