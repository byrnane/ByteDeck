#include "ui/screens/apps_screen.hpp"

#include "platform/logger.hpp"
#include "ui/navigation_input.hpp"
#include "ui/text_renderer.hpp"
#include <utility>

namespace bytedeck::ui
{
AppsScreen::AppsScreen(
    const data::LibraryData& library,
    std::filesystem::path root_path,
    const platform::TranslationCatalog& translations,
    LaunchAppCallback launch_app)
    : root_path_(std::move(root_path))
    , translations_(translations)
    , launch_app_(std::move(launch_app))
{
    for (const data::AppItem& app : library.apps)
    {
        apps_.push_back(&app);
    }
}


ScreenAction AppsScreen::handle_event(const SDL_Event& event)
{
    switch (navigation_input_from_event(event))
    {
    case NavigationInput::up:
        move_selection(-1);
        return {};
    case NavigationInput::down:
        move_selection(1);
        return {};
    case NavigationInput::accept:
        launch_selected_app();
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


std::string AppsScreen::screen_id() const
{
    return "apps";
}


UiBindings AppsScreen::build_bindings() const
{
    UiBindings bindings {
        { "title", translations_.translate("menu.apps") },
        { "subtitle", translations_.translate("apps.subtitle") },
        { "empty", apps_.empty() },
        { "has_items", !apps_.empty() },
        { "empty_title", translations_.translate("apps.empty_title") },
        { "empty_body", translations_.translate("apps.empty_body") },
        { "empty_hint", translations_.translate("apps.empty_hint") },
        { "items", UiBindings::array() }
    };

    const std::vector<const data::AppItem*> visible = visible_apps();
    for (const data::AppItem* app : visible)
    {
        bindings["items"].push_back({
            { "title", app->title },
            { "selected", app == apps_[selected_index_] }
        });
    }

    if (apps_.empty())
    {
        return bindings;
    }

    const data::AppItem& selected_app = *apps_[selected_index_];
    bindings["preview_title"] = translations_.translate("apps.preview_title");
    bindings["preview_path"] = selected_app.icon;
    bindings["preview_placeholder"] = selected_app.icon.empty() ? translations_.translate("apps.no_icon") : translations_.translate("apps.icon_missing");
    bindings["app_title"] = selected_app.title;
    bindings["target_line"] = translations_.translate("apps.target_prefix") + selected_app.launch_target;
    bindings["description_title"] = translations_.translate("common.description");
    bindings["description_text"] = selected_app.description.empty() ? translations_.translate("common.no_description") : selected_app.description;
    bindings["launch_status_success_visible"] = !launch_status_.empty() && launch_status_ok_;
    bindings["launch_status_error_visible"] = !launch_status_.empty() && !launch_status_ok_;
    bindings["launch_status_text"] = launch_status_;
    return bindings;
}


std::string AppsScreen::window_title() const
{
    if (apps_.empty())
    {
        return translations_.translate("menu.apps");
    }

    return translations_.translate("menu.apps") + " - " + apps_[selected_index_]->title;
}


void AppsScreen::move_selection(int delta)
{
    if (apps_.empty())
    {
        return;
    }

    const int item_count = static_cast<int>(apps_.size());
    const int current = static_cast<int>(selected_index_);
    selected_index_ = static_cast<std::size_t>((current + delta + item_count * 4) % item_count);
}


std::vector<const data::AppItem*> AppsScreen::visible_apps() const
{
    std::vector<const data::AppItem*> visible;
    if (apps_.empty())
    {
        return visible;
    }

    constexpr int kVisibleRows = 14;
    int start_index = 0;
    if (selected_index_ >= static_cast<std::size_t>(kVisibleRows))
    {
        start_index = static_cast<int>(selected_index_) - kVisibleRows + 1;
    }

    for (int row = 0; row < kVisibleRows; ++row)
    {
        const int item_index = start_index + row;
        if (item_index >= static_cast<int>(apps_.size()))
        {
            break;
        }

        visible.push_back(apps_[item_index]);
    }

    return visible;
}


void AppsScreen::launch_selected_app()
{
    if (apps_.empty())
    {
        return;
    }

    if (!launch_app_)
    {
        launch_status_ = translations_.translate("error.launch_callback_missing");
        launch_status_ok_ = false;
        platform::Logger::instance().error("Launch callback missing for apps screen");
        return;
    }

    const launch::LaunchResult result = launch_app_(*apps_[selected_index_]);
    launch_status_ = TextRenderer::truncate_to_width(result.message, 28);
    launch_status_ok_ = result.success;
}
}
