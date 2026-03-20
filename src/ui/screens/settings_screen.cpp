#include "ui/screens/settings_screen.hpp"

#include "ui/navigation_input.hpp"

#include <algorithm>
#include <utility>

namespace bytedeck::ui
{
namespace
{
std::string language_display_name(const platform::TranslationCatalog& translations, const std::string& language_code)
{
    return translations.translate("language.name." + language_code);
}
}


SettingsScreen::SettingsScreen(
    const platform::TranslationCatalog& translations,
    std::string current_language,
    std::vector<std::string> available_themes,
    std::string current_theme,
    ApplyValueCallback apply_language,
    ApplyValueCallback apply_theme,
    RescanCallback rescan_library)
    : translations_(translations)
    , current_language_(std::move(current_language))
    , available_themes_(std::move(available_themes))
    , current_theme_(std::move(current_theme))
    , apply_language_(std::move(apply_language))
    , apply_theme_(std::move(apply_theme))
    , rescan_library_(std::move(rescan_library))
{
    if (available_themes_.empty())
    {
        available_themes_.push_back(current_theme_.empty() ? std::string("default") : current_theme_);
    }

    if (current_theme_.empty())
    {
        current_theme_ = available_themes_.front();
    }
}


ScreenAction SettingsScreen::handle_event(const SDL_Event& event)
{
    switch (navigation_input_from_event(event))
    {
    case NavigationInput::up:
        move_selection(-1);
        return {};
    case NavigationInput::down:
        move_selection(1);
        return {};
    case NavigationInput::left:
        if (items_[selected_index_] == SettingItem::language)
        {
            cycle_language(-1);
        }
        else if (items_[selected_index_] == SettingItem::theme)
        {
            cycle_theme(-1);
        }
        return {};
    case NavigationInput::right:
        if (items_[selected_index_] == SettingItem::language)
        {
            cycle_language(1);
        }
        else if (items_[selected_index_] == SettingItem::theme)
        {
            cycle_theme(1);
        }
        return {};
    case NavigationInput::accept:
        if (items_[selected_index_] == SettingItem::language)
        {
            cycle_language(1);
        }
        else if (items_[selected_index_] == SettingItem::theme)
        {
            cycle_theme(1);
        }
        else
        {
            run_rescan();
        }
        return {};
    case NavigationInput::back:
        return { ScreenActionType::pop };
    case NavigationInput::quit:
        return { ScreenActionType::quit };
    case NavigationInput::none:
    default:
        return {};
    }
}


std::string SettingsScreen::screen_id() const
{
    return "settings";
}


UiBindings SettingsScreen::build_bindings() const
{
    UiBindings items = UiBindings::array();
    for (std::size_t index = 0; index < items_.size(); ++index)
    {
        const SettingItem item = items_[index];
        items.push_back({
            { "title", translations_.translate("settings.item." + std::string(
                item == SettingItem::language ? "language" :
                item == SettingItem::theme ? "theme" :
                "rescan")) },
            { "value", setting_value_text(item) },
            { "hint", setting_hint_text(item, index == selected_index_) },
            { "selected", index == selected_index_ }
        });
    }

    return UiBindings {
        { "title", translations_.translate("menu.settings") },
        { "subtitle", translations_.translate("settings.subtitle") },
        { "items", items },
        { "detail_title", detail_title() },
        { "detail_body", detail_body() },
        { "status_success_visible", !status_message_.empty() && status_success_ },
        { "status_error_visible", !status_message_.empty() && !status_success_ },
        { "status_text", status_message_ }
    };
}


std::string SettingsScreen::window_title() const
{
    return translations_.translate("menu.settings");
}


void SettingsScreen::move_selection(int delta)
{
    const int item_count = static_cast<int>(items_.size());
    const int current = static_cast<int>(selected_index_);
    selected_index_ = static_cast<std::size_t>((current + delta + item_count) % item_count);
}


void SettingsScreen::cycle_language(int delta)
{
    const std::vector<std::string> languages = { "en", "ru" };
    auto current = std::find(languages.begin(), languages.end(), current_language_);
    std::size_t index = current != languages.end() ? static_cast<std::size_t>(current - languages.begin()) : 0;
    index = (index + languages.size() + static_cast<std::size_t>(delta < 0 ? languages.size() - 1 : 1)) % languages.size();
    const std::string next_language = languages[index];

    if (next_language == current_language_)
    {
        return;
    }

    if (apply_language_ && apply_language_(next_language))
    {
        current_language_ = next_language;
        set_status(translations_.translate("settings.status.language_applied"), true);
        return;
    }

    set_status(translations_.translate("settings.status.language_failed"), false);
}


void SettingsScreen::cycle_theme(int delta)
{
    if (available_themes_.empty())
    {
        set_status(translations_.translate("settings.status.theme_failed"), false);
        return;
    }

    auto current = std::find(available_themes_.begin(), available_themes_.end(), current_theme_);
    std::size_t index = current != available_themes_.end() ? static_cast<std::size_t>(current - available_themes_.begin()) : 0;
    if (delta < 0)
    {
        index = (index + available_themes_.size() - 1) % available_themes_.size();
    }
    else
    {
        index = (index + 1) % available_themes_.size();
    }

    const std::string next_theme = available_themes_[index];
    if (next_theme == current_theme_)
    {
        set_status(translations_.translate("settings.status.theme_applied"), true);
        return;
    }

    if (apply_theme_ && apply_theme_(next_theme))
    {
        current_theme_ = next_theme;
        set_status(translations_.translate("settings.status.theme_applied"), true);
        return;
    }

    set_status(translations_.translate("settings.status.theme_failed"), false);
}


void SettingsScreen::run_rescan()
{
    if (!rescan_library_)
    {
        set_status(translations_.translate("settings.status.rescan_failed"), false);
        return;
    }

    const auto [success, message] = rescan_library_();
    if (!message.empty())
    {
        set_status(message, success);
        return;
    }

    set_status(
        success ? translations_.translate("settings.status.rescan_success")
                : translations_.translate("settings.status.rescan_failed"),
        success);
}


void SettingsScreen::set_status(std::string message, bool success)
{
    status_message_ = std::move(message);
    status_success_ = success;
}


std::string SettingsScreen::setting_value_text(SettingItem item) const
{
    switch (item)
    {
    case SettingItem::language:
        return language_display_name(translations_, current_language_);
    case SettingItem::theme:
        return current_theme_;
    case SettingItem::rescan:
        return translations_.translate("settings.rescan_value");
    }

    return {};
}


std::string SettingsScreen::setting_hint_text(SettingItem item, bool selected) const
{
    if (!selected)
    {
        return translations_.translate("common.ready");
    }

    switch (item)
    {
    case SettingItem::language:
    case SettingItem::theme:
        return translations_.translate("settings.hint.cycle");
    case SettingItem::rescan:
        return translations_.translate("settings.hint.run");
    }

    return translations_.translate("common.ready");
}


std::string SettingsScreen::detail_title() const
{
    switch (items_[selected_index_])
    {
    case SettingItem::language:
        return translations_.translate("settings.item.language");
    case SettingItem::theme:
        return translations_.translate("settings.item.theme");
    case SettingItem::rescan:
        return translations_.translate("settings.item.rescan");
    }

    return translations_.translate("menu.settings");
}


std::string SettingsScreen::detail_body() const
{
    switch (items_[selected_index_])
    {
    case SettingItem::language:
        return translations_.translate("settings.detail.language");
    case SettingItem::theme:
        return translations_.translate("settings.detail.theme");
    case SettingItem::rescan:
        return translations_.translate("settings.detail.rescan");
    }

    return {};
}
}
