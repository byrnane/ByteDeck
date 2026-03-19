#pragma once

#include "data/models.hpp"
#include "platform/paths.hpp"
#include "platform/user_settings.hpp"
#include "ui/screen_manager.hpp"

#include <SDL.h>

#include <memory>
#include <string>

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
    void render();
    void refresh_window_title();
    void toggle_fullscreen();

    bool initialized_ = false;
    bool fullscreen_ = false;
    SDL_Window* window_ = nullptr;
    SDL_Renderer* renderer_ = nullptr;
    platform::Paths paths_;
    platform::UserSettings user_settings_;
    data::LibraryData library_;
    ui::ScreenManager screen_manager_;
    std::string window_title_;
};
}
