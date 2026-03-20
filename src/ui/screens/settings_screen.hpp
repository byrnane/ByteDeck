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

    void move_selection(int delta);
    void cycle_language(int delta);
    void cycle_theme(int delta);
    void run_rescan();
    void set_status(std::string message, bool success);
    std::string setting_value_text(SettingItem item) const;
    std::string setting_hint_text(SettingItem item, bool selected) const;
    std::string detail_title() const;
    std::string detail_body() const;

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
    std::size_t selected_index_ = 0;
    std::string status_message_;
    bool status_success_ = false;
};
}
