#include "app/application.hpp"

#include "core/library_scanner.hpp"
#include "platform/logger.hpp"
#include "ui/screens/apps_screen.hpp"
#include "ui/navigation_input.hpp"
#include "ui/screens/game_browser_screen.hpp"
#include "ui/screens/games_screen.hpp"
#include "ui/screens/main_menu_screen.hpp"
#include "ui/screens/placeholder_screen.hpp"
#include "ui/screens/settings_stub_screen.hpp"

#include <SDL.h>

#include <stdexcept>
#include <string>

namespace bytedeck
{
namespace
{
constexpr int kWindowWidth = 1280;
constexpr int kWindowHeight = 720;
constexpr const char* kBaseWindowTitle = "ByteDeck";
constexpr Uint32 kRepeatDelayMs = 350;
constexpr Uint32 kRepeatIntervalMs = 90;

SDL_Keycode keycode_for_navigation_input(ui::NavigationInput input)
{
    switch (input)
    {
    case ui::NavigationInput::up:
        return SDLK_UP;
    case ui::NavigationInput::down:
        return SDLK_DOWN;
    case ui::NavigationInput::left:
        return SDLK_LEFT;
    case ui::NavigationInput::right:
        return SDLK_RIGHT;
    case ui::NavigationInput::accept:
        return SDLK_RETURN;
    case ui::NavigationInput::back:
        return SDLK_ESCAPE;
    case ui::NavigationInput::quit:
        return SDLK_q;
    case ui::NavigationInput::none:
    default:
        return SDLK_UNKNOWN;
    }
}
}


Application::Application() = default;


Application::~Application()
{
    shutdown();
}


int Application::run(int /*argc*/, char** /*argv*/)
{
    initialize();

    screen_manager_.push(std::make_unique<ui::MainMenuScreen>(library_));
    refresh_window_title();

    bool running = true;
    while (running)
    {
        SDL_Event event;
        while (SDL_PollEvent(&event) == 1)
        {
            handle_event(event, running);
        }

        if (pending_launcher_exit_)
        {
            pending_launcher_exit_ = false;
            running = false;
        }

        if (!running)
        {
            break;
        }

        handle_repeat(running);

        if (screen_manager_.empty())
        {
            running = false;
            break;
        }

        render();
        SDL_Delay(16);
    }

    const bool should_execute_launch = pending_launch_request_.has_value();
    const std::optional<launch::LaunchRequest> pending_request = pending_launch_request_;
    pending_launch_request_.reset();
    shutdown();

    if (should_execute_launch && pending_request.has_value())
    {
        return launch_service_->execute_prepared(*pending_request) ? 0 : 1;
    }

    return 0;
}


void Application::initialize()
{
    if (initialized_)
    {
        return;
    }

    paths_ = platform::Paths::discover();
    platform::Logger::instance().initialize(paths_.log_file_path());
    platform::Logger::instance().info("Starting ByteDeck");

    user_settings_ = platform::UserSettings::load(paths_.user_settings_path());
    launch_service_ = std::make_unique<launch::LaunchService>(paths_);
    layout_registry_ = std::make_unique<ui::LayoutRegistry>(paths_.ui_screens_root());
    layout_registry_->load();
    theme_manager_ = std::make_unique<ui::ThemeManager>(paths_.themes_root(), user_settings_.theme);
    theme_manager_->load();

    core::LibraryScanner scanner(paths_);
    library_ = scanner.scan();
    scanner.save_cache(library_);

    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_GAMECONTROLLER | SDL_INIT_JOYSTICK) != 0)
    {
        throw std::runtime_error(std::string("SDL_Init failed: ") + SDL_GetError());
    }

    SDL_GameControllerEventState(SDL_ENABLE);
    SDL_JoystickEventState(SDL_ENABLE);

    window_ = SDL_CreateWindow(
        kBaseWindowTitle,
        SDL_WINDOWPOS_CENTERED,
        SDL_WINDOWPOS_CENTERED,
        kWindowWidth,
        kWindowHeight,
        SDL_WINDOW_SHOWN | SDL_WINDOW_RESIZABLE
    );
    if (window_ == nullptr)
    {
        throw std::runtime_error(std::string("SDL_CreateWindow failed: ") + SDL_GetError());
    }

    renderer_ = SDL_CreateRenderer(window_, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
    if (renderer_ == nullptr)
    {
        throw std::runtime_error(std::string("SDL_CreateRenderer failed: ") + SDL_GetError());
    }

    ui_renderer_ = std::make_unique<ui::UiRenderer>(paths_, *layout_registry_, *theme_manager_);
    open_input_devices();
    initialized_ = true;
}


void Application::shutdown()
{
    close_input_devices();

    if (renderer_ != nullptr)
    {
        SDL_DestroyRenderer(renderer_);
        renderer_ = nullptr;
    }

    if (window_ != nullptr)
    {
        SDL_DestroyWindow(window_);
        window_ = nullptr;
    }

    if (initialized_)
    {
        SDL_Quit();
        initialized_ = false;
    }
}


void Application::handle_event(const SDL_Event& event, bool& running)
{
    if (event.type == SDL_QUIT)
    {
        running = false;
        return;
    }

    if (event.type == SDL_KEYDOWN && event.key.keysym.sym == SDLK_F11)
    {
        toggle_fullscreen();
        return;
    }

    if (ui::navigation_input_from_event(event) == ui::NavigationInput::quit)
    {
        running = false;
        return;
    }

    update_repeat_state(event);

    auto* current = screen_manager_.current();
    if (current == nullptr)
    {
        running = false;
        return;
    }

    const ui::ScreenAction action = current->handle_event(event);
    apply_screen_action(action, running);
}


void Application::render()
{
    SDL_SetRenderDrawBlendMode(renderer_, SDL_BLENDMODE_BLEND);
    SDL_SetRenderDrawColor(renderer_, 15, 18, 24, 255);
    SDL_RenderClear(renderer_);

    auto* current = screen_manager_.current();
    if (current != nullptr)
    {
        ui_renderer_->render_screen(*renderer_, current->screen_id(), current->build_bindings());
    }

    SDL_RenderPresent(renderer_);
}


void Application::refresh_window_title()
{
    auto* current = screen_manager_.current();
    window_title_ = kBaseWindowTitle;
    if (current != nullptr)
    {
        window_title_ += " | ";
        window_title_ += current->window_title();
    }
    if (window_ != nullptr)
    {
        SDL_SetWindowTitle(window_, window_title_.c_str());
    }
}


void Application::toggle_fullscreen()
{
    fullscreen_ = !fullscreen_;
    const Uint32 flags = fullscreen_ ? SDL_WINDOW_FULLSCREEN_DESKTOP : 0;
    SDL_SetWindowFullscreen(window_, flags);
}


void Application::handle_repeat(bool& running)
{
    if (!ui::is_repeatable_navigation_input(repeating_input_))
    {
        return;
    }

    const Uint32 now = SDL_GetTicks();
    if (now < next_repeat_tick_)
    {
        return;
    }

    SDL_Event synthetic_event {};
    synthetic_event.type = SDL_KEYDOWN;
    synthetic_event.key.type = SDL_KEYDOWN;
    synthetic_event.key.state = SDL_PRESSED;
    synthetic_event.key.repeat = 0;
    synthetic_event.key.keysym.sym = keycode_for_navigation_input(repeating_input_);

    auto* current = screen_manager_.current();
    if (current == nullptr)
    {
        running = false;
        return;
    }

    const ui::ScreenAction action = current->handle_event(synthetic_event);
    apply_screen_action(action, running);
    next_repeat_tick_ = now + kRepeatIntervalMs;
}


void Application::apply_screen_action(const ui::ScreenAction& action, bool& running)
{
    switch (action.type)
    {
    case ui::ScreenActionType::none:
        break;
    case ui::ScreenActionType::pop:
        screen_manager_.pop();
        clear_repeat_state();
        break;
    case ui::ScreenActionType::quit:
        running = false;
        clear_repeat_state();
        break;
    case ui::ScreenActionType::open_games:
        screen_manager_.push(std::make_unique<ui::GamesScreen>(library_));
        clear_repeat_state();
        break;
    case ui::ScreenActionType::open_apps:
        screen_manager_.push(std::make_unique<ui::AppsScreen>(
            library_,
            paths_.root(),
            [this](const data::AppItem& item)
            {
                const launch::LaunchResult result = launch_service_->launch_app(item);
                if (result.success && result.should_exit_launcher)
                {
                    pending_launcher_exit_ = true;
                    pending_launch_request_ = result.request;
                }
                return result;
            }
        ));
        clear_repeat_state();
        break;
    case ui::ScreenActionType::open_settings_stub:
        screen_manager_.push(std::make_unique<ui::SettingsStubScreen>());
        clear_repeat_state();
        break;
    case ui::ScreenActionType::open_placeholder:
        screen_manager_.push(std::make_unique<ui::PlaceholderScreen>(action.value.empty() ? "Placeholder" : action.value));
        clear_repeat_state();
        break;
    case ui::ScreenActionType::open_game_browser:
        screen_manager_.push(std::make_unique<ui::GameBrowserScreen>(
            library_,
            paths_.root(),
            action.value,
            [this](const data::GameItem& item)
            {
                const launch::LaunchResult result = launch_service_->launch_game(item);
                if (result.success && result.should_exit_launcher)
                {
                    pending_launcher_exit_ = true;
                    pending_launch_request_ = result.request;
                }
                return result;
            }
        ));
        clear_repeat_state();
        break;
    }

    refresh_window_title();
}


void Application::update_repeat_state(const SDL_Event& event)
{
    const ui::NavigationInput pressed_input = ui::navigation_input_from_event(event);
    const ui::NavigationInput released_input = ui::navigation_input_release_from_event(event);

    if (released_input != ui::NavigationInput::none && released_input == repeating_input_)
    {
        clear_repeat_state();
        return;
    }

    if (event.type == SDL_JOYHATMOTION || event.type == SDL_JOYAXISMOTION)
    {
        if (ui::is_repeatable_navigation_input(pressed_input))
        {
            repeating_input_ = pressed_input;
            next_repeat_tick_ = SDL_GetTicks() + kRepeatDelayMs;
        }
        else
        {
            clear_repeat_state();
        }
        return;
    }

    if (ui::is_repeatable_navigation_input(pressed_input))
    {
        repeating_input_ = pressed_input;
        next_repeat_tick_ = SDL_GetTicks() + kRepeatDelayMs;
    }
}


void Application::clear_repeat_state()
{
    repeating_input_ = ui::NavigationInput::none;
    next_repeat_tick_ = 0;
}


void Application::open_input_devices()
{
    const int device_count = SDL_NumJoysticks();
    platform::Logger::instance().info("SDL input devices detected: " + std::to_string(device_count));

    for (int index = 0; index < device_count; ++index)
    {
        if (SDL_IsGameController(index) == SDL_TRUE)
        {
            SDL_GameController* controller = SDL_GameControllerOpen(index);
            if (controller != nullptr)
            {
                game_controllers_.push_back(controller);
                const char* name = SDL_GameControllerName(controller);
                platform::Logger::instance().info(
                    "Opened SDL game controller #" + std::to_string(index) + ": " + (name != nullptr ? std::string(name) : std::string("unknown"))
                );
                continue;
            }

            platform::Logger::instance().error(
                "Failed to open SDL game controller #" + std::to_string(index) + ": " + SDL_GetError()
            );
        }

        SDL_Joystick* joystick = SDL_JoystickOpen(index);
        if (joystick != nullptr)
        {
            joysticks_.push_back(joystick);
            const char* name = SDL_JoystickName(joystick);
            platform::Logger::instance().info(
                "Opened SDL joystick #" + std::to_string(index) + ": " + (name != nullptr ? std::string(name) : std::string("unknown"))
            );
            continue;
        }

        platform::Logger::instance().error(
            "Failed to open SDL joystick #" + std::to_string(index) + ": " + SDL_GetError()
        );
    }
}


void Application::close_input_devices()
{
    for (SDL_GameController* controller : game_controllers_)
    {
        if (controller != nullptr)
        {
            SDL_GameControllerClose(controller);
        }
    }
    game_controllers_.clear();

    for (SDL_Joystick* joystick : joysticks_)
    {
        if (joystick != nullptr)
        {
            SDL_JoystickClose(joystick);
        }
    }
    joysticks_.clear();
}
}
