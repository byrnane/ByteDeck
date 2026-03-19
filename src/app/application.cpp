#include "app/application.hpp"

#include "core/library_scanner.hpp"
#include "platform/logger.hpp"
#include "ui/screens/apps_screen.hpp"
#include "ui/screens/game_browser_screen.hpp"
#include "ui/screens/games_screen.hpp"
#include "ui/screens/main_menu_screen.hpp"
#include "ui/screens/placeholder_screen.hpp"

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

        if (screen_manager_.empty())
        {
            running = false;
            break;
        }

        render();
        SDL_Delay(16);
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
    platform::Logger::instance().info("Starting ByteDeck desktop skeleton");

    user_settings_ = platform::UserSettings::load(paths_.user_settings_path());

    core::LibraryScanner scanner(paths_);
    library_ = scanner.scan();
    scanner.save_cache(library_);

    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_GAMECONTROLLER) != 0)
    {
        throw std::runtime_error(std::string("SDL_Init failed: ") + SDL_GetError());
    }

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

    initialized_ = true;
}


void Application::shutdown()
{
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
    switch (event.type)
    {
    case SDL_QUIT:
        running = false;
        return;
    case SDL_KEYDOWN:
        if (event.key.keysym.sym == SDLK_q)
        {
            running = false;
            return;
        }
        if (event.key.keysym.sym == SDLK_F11)
        {
            toggle_fullscreen();
            return;
        }
        break;
    default:
        break;
    }

    auto* current = screen_manager_.current();
    if (current == nullptr)
    {
        running = false;
        return;
    }

    const ui::ScreenAction action = current->handle_event(event);
    switch (action.type)
    {
    case ui::ScreenActionType::none:
        break;
    case ui::ScreenActionType::pop:
        screen_manager_.pop();
        break;
    case ui::ScreenActionType::quit:
        running = false;
        break;
    case ui::ScreenActionType::open_games:
        screen_manager_.push(std::make_unique<ui::GamesScreen>(library_));
        break;
    case ui::ScreenActionType::open_apps:
        screen_manager_.push(std::make_unique<ui::AppsScreen>(library_, paths_.root()));
        break;
    case ui::ScreenActionType::open_placeholder:
        screen_manager_.push(std::make_unique<ui::PlaceholderScreen>(action.value.empty() ? "Placeholder" : action.value));
        break;
    case ui::ScreenActionType::open_game_browser:
        screen_manager_.push(std::make_unique<ui::GameBrowserScreen>(library_, paths_.root(), action.value));
        break;
    }

    refresh_window_title();
}


void Application::render()
{
    SDL_SetRenderDrawBlendMode(renderer_, SDL_BLENDMODE_BLEND);
    SDL_SetRenderDrawColor(renderer_, 15, 18, 24, 255);
    SDL_RenderClear(renderer_);

    auto* current = screen_manager_.current();
    if (current != nullptr)
    {
        current->render(*renderer_);
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
}
