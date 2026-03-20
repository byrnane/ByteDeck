#include "ui/screens/settings_stub_screen.hpp"

#include "ui/navigation_input.hpp"

namespace bytedeck::ui
{
ScreenAction SettingsStubScreen::handle_event(const SDL_Event& event)
{
    const NavigationInput input = navigation_input_from_event(event);
    if (input == NavigationInput::back)
    {
        return { ScreenActionType::pop };
    }

    if (input == NavigationInput::quit)
    {
        return { ScreenActionType::quit };
    }

    return {};
}


std::string SettingsStubScreen::screen_id() const
{
    return "settings_stub";
}


UiBindings SettingsStubScreen::build_bindings() const
{
    return UiBindings {
        { "title", "SETTINGS" },
        { "subtitle", "CONFIGURATION STUB" },
        { "body", "THEME SUPPORT IS READY IN ARCHITECTURE. SETTINGS UI COMES NEXT." }
    };
}


std::string SettingsStubScreen::window_title() const
{
    return "Settings";
}
}
