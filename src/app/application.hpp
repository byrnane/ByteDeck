#pragma once

#include "data/models.hpp"
#include "launch/launch_service.hpp"
#include "platform/paths.hpp"
#include "platform/status_service.hpp"
#include "platform/translation_catalog.hpp"
#include "platform/user_settings.hpp"
#include "ui/fixed_ui_renderer.hpp"
#include "ui/navigation_input.hpp"
#include "ui/screen_manager.hpp"
#include "ui/theme_manager.hpp"

#include <SDL.h>

#include <memory>
#include <optional>
#include <string>
#include <utility>
#include <vector>

namespace bytedeck
{
class Application
{
public:
    Application();
    ~Application();

    int run(int argc, char** argv);

private:
    void initialize();
    void shutdown();
    void handle_event(const SDL_Event& event, bool& running);
    void handle_repeat(bool& running);
    void apply_screen_action(const ui::ScreenAction& action, bool& running);
    void update_repeat_state(const SDL_Event& event);
    void clear_repeat_state();
    void render();
    void refresh_window_title();
    void toggle_fullscreen();
    void open_input_devices();
    void close_input_devices();
    void reload_theme();
    bool apply_language(const std::string& language);
    bool apply_theme(const std::string& theme_id);
    std::pair<bool, std::string> rescan_library();
    ui::UiBindings build_shell_bindings(const ui::Screen& screen, const ui::UiBindings& screen_bindings) const;

    bool initialized_ = false;
    bool fullscreen_ = false;
    SDL_Window* window_ = nullptr;
    SDL_Renderer* renderer_ = nullptr;
    std::vector<SDL_GameController*> game_controllers_;
    std::vector<SDL_Joystick*> joysticks_;
    ui::NavigationInput repeating_input_ = ui::NavigationInput::none;
    Uint32 next_repeat_tick_ = 0;
    bool pending_launcher_exit_ = false;
    std::optional<launch::LaunchRequest> pending_launch_request_;
    platform::Paths paths_;
    platform::UserSettings user_settings_;
    platform::TranslationCatalog translations_;
    std::unique_ptr<platform::StatusService> status_service_;
    data::LibraryData library_;
    std::unique_ptr<launch::LaunchService> launch_service_;
    std::unique_ptr<ui::ThemeManager> theme_manager_;
    std::unique_ptr<ui::FixedUiRenderer> ui_renderer_;
    ui::ScreenManager screen_manager_;
    std::string window_title_;
};
}
