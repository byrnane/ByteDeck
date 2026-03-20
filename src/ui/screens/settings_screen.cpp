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

    sync_editor_selection_to_current_value();
}


ScreenAction SettingsScreen::handle_event(const SDL_Event& event)
{
    switch (navigation_input_from_event(event))
    {
    case NavigationInput::up:
        if (focus_area_ == FocusArea::left)
        {
            move_selection(-1);
        }
        else
        {
            move_editor_selection(-1);
        }
        return {};
    case NavigationInput::down:
        if (focus_area_ == FocusArea::left)
        {
            move_selection(1);
        }
        else
        {
            move_editor_selection(1);
        }
        return {};
    case NavigationInput::left:
        if (focus_area_ == FocusArea::right)
        {
            leave_editor();
        }
        return {};
    case NavigationInput::right:
        if (focus_area_ == FocusArea::left)
        {
            enter_editor();
        }
        return {};
    case NavigationInput::accept:
        if (focus_area_ == FocusArea::left)
        {
            enter_editor();
        }
        else
        {
            apply_editor_selection();
        }
        return {};
    case NavigationInput::back:
        if (focus_area_ == FocusArea::right)
        {
            leave_editor();
            return {};
        }
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
        items.push_back({
            { "title", translations_.translate("settings.item." + setting_key(items_[index])) },
            { "value", setting_value_text(items_[index]) },
            { "hint", setting_hint_text(index) },
            { "selected", index == selected_index_ },
            { "active", index == selected_index_ && focus_area_ == FocusArea::left }
        });
    }

    UiBindings editor_items = UiBindings::array();
    const std::vector<EditorOption> options = editor_options();
    for (std::size_t index = 0; index < options.size(); ++index)
    {
        editor_items.push_back({
            { "title", options[index].title },
            { "description", options[index].description },
            { "selected", index == editor_selection_index_ },
            { "active", options[index].current }
        });
    }

    return UiBindings {
        { "title", translations_.translate("menu.settings") },
        { "subtitle", translations_.translate("settings.subtitle") },
        { "focus_left", focus_area_ == FocusArea::left },
        { "focus_right", focus_area_ == FocusArea::right },
        { "items", items },
        { "editor_title", editor_title() },
        { "editor_description", editor_description() },
        { "editor_hint", editor_hint() },
        { "editor_items", editor_items },
        { "editor_has_items", !options.empty() },
        { "editor_empty", options.empty() },
        { "editor_status_text", status_message_ },
        { "editor_status_success", !status_message_.empty() && status_success_ },
        { "editor_status_error", !status_message_.empty() && !status_success_ },
        { "editor_mode_action_result", editor_mode_ == EditorMode::action_result }
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
    status_message_.clear();
    editor_mode_ = EditorMode::list;
    sync_editor_selection_to_current_value();
}


void SettingsScreen::move_editor_selection(int delta)
{
    const std::vector<EditorOption> options = editor_options();
    if (options.empty())
    {
        return;
    }

    const int item_count = static_cast<int>(options.size());
    const int current = static_cast<int>(editor_selection_index_);
    editor_selection_index_ = static_cast<std::size_t>((current + delta + item_count) % item_count);
    editor_mode_ = EditorMode::list;
}


void SettingsScreen::enter_editor()
{
    focus_area_ = FocusArea::right;
    editor_mode_ = EditorMode::list;
    sync_editor_selection_to_current_value();
}


void SettingsScreen::leave_editor()
{
    focus_area_ = FocusArea::left;
    editor_mode_ = EditorMode::list;
}


void SettingsScreen::apply_editor_selection()
{
    const SettingItem current_item = items_[selected_index_];
    const std::vector<EditorOption> options = editor_options();
    if (options.empty() || editor_selection_index_ >= options.size())
    {
        return;
    }

    const EditorOption& option = options[editor_selection_index_];
    if (current_item == SettingItem::language)
    {
        if (option.value == current_language_)
        {
            set_status(translations_.translate("settings.status.language_applied"), true);
            return;
        }

        if (apply_language_ && apply_language_(option.value))
        {
            current_language_ = option.value;
            set_status(translations_.translate("settings.status.language_applied"), true);
        }
        else
        {
            set_status(translations_.translate("settings.status.language_failed"), false);
        }
        return;
    }

    if (current_item == SettingItem::theme)
    {
        if (option.value == current_theme_)
        {
            set_status(translations_.translate("settings.status.theme_applied"), true);
            return;
        }

        if (apply_theme_ && apply_theme_(option.value))
        {
            current_theme_ = option.value;
            set_status(translations_.translate("settings.status.theme_applied"), true);
        }
        else
        {
            set_status(translations_.translate("settings.status.theme_failed"), false);
        }
        return;
    }

    run_rescan();
}


void SettingsScreen::run_rescan()
{
    if (!rescan_library_)
    {
        set_status(translations_.translate("settings.status.rescan_failed"), false, EditorMode::action_result);
        return;
    }

    const auto [success, message] = rescan_library_();
    if (!message.empty())
    {
        set_status(message, success, EditorMode::action_result);
        return;
    }

    set_status(
        success ? translations_.translate("settings.status.rescan_success")
                : translations_.translate("settings.status.rescan_failed"),
        success,
        EditorMode::action_result);
}


void SettingsScreen::set_status(std::string message, bool success, EditorMode mode)
{
    status_message_ = std::move(message);
    status_success_ = success;
    editor_mode_ = mode;
}


void SettingsScreen::sync_editor_selection_to_current_value()
{
    editor_selection_index_ = 0;
    const std::vector<EditorOption> options = editor_options();
    if (options.empty())
    {
        return;
    }

    for (std::size_t index = 0; index < options.size(); ++index)
    {
        if (options[index].current)
        {
            editor_selection_index_ = index;
            return;
        }
    }
}


std::vector<SettingsScreen::EditorOption> SettingsScreen::editor_options() const
{
    const SettingItem current_item = items_[selected_index_];
    if (current_item == SettingItem::language)
    {
        return {
            { "en", language_display_name(translations_, "en"), translations_.translate("settings.option.language.en"), current_language_ == "en" },
            { "ru", language_display_name(translations_, "ru"), translations_.translate("settings.option.language.ru"), current_language_ == "ru" }
        };
    }

    if (current_item == SettingItem::theme)
    {
        std::vector<EditorOption> options;
        for (const std::string& theme_id : available_themes_)
        {
            options.push_back({
                theme_id,
                display_theme_name(theme_id),
                translated_or_fallback("settings.option.theme." + theme_id, theme_id),
                current_theme_ == theme_id
            });
        }
        return options;
    }

    return {
        {
            "run",
            translations_.translate("settings.option.rescan.title"),
            translations_.translate("settings.option.rescan.description"),
            false
        }
    };
}


std::string SettingsScreen::setting_key(SettingItem item) const
{
    switch (item)
    {
    case SettingItem::language:
        return "language";
    case SettingItem::theme:
        return "theme";
    case SettingItem::rescan:
        return "rescan";
    }

    return {};
}


std::string SettingsScreen::display_theme_name(const std::string& theme_id) const
{
    return translated_or_fallback("theme." + theme_id, theme_id);
}


std::string SettingsScreen::translated_or_fallback(const std::string& key, const std::string& fallback) const
{
    const std::string translated = translations_.translate(key);
    return translated == key ? fallback : translated;
}


std::string SettingsScreen::setting_value_text(SettingItem item) const
{
    switch (item)
    {
    case SettingItem::language:
        return language_display_name(translations_, current_language_);
    case SettingItem::theme:
        return display_theme_name(current_theme_);
    case SettingItem::rescan:
        return translations_.translate("settings.rescan_value");
    }

    return {};
}


std::string SettingsScreen::setting_hint_text(std::size_t index) const
{
    if (index != selected_index_)
    {
        return translations_.translate("common.ready");
    }

    if (focus_area_ == FocusArea::left)
    {
        return translations_.translate("settings.hint.enter_editor");
    }

    return translations_.translate("settings.hint.editing");
}


std::string SettingsScreen::editor_title() const
{
    return translations_.translate("settings.editor." + setting_key(items_[selected_index_]) + ".title");
}


std::string SettingsScreen::editor_description() const
{
    return translations_.translate("settings.editor." + setting_key(items_[selected_index_]) + ".description");
}


std::string SettingsScreen::editor_hint() const
{
    if (focus_area_ == FocusArea::left)
    {
        return translations_.translate("settings.hint.enter_editor");
    }

    return translations_.translate("settings.hint.apply_choice");
}
}
