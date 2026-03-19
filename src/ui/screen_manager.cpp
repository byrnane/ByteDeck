#include "ui/screen_manager.hpp"

namespace bytedeck::ui
{
void ScreenManager::push(std::unique_ptr<Screen> screen)
{
    screens_.push_back(std::move(screen));
}


void ScreenManager::pop()
{
    if (!screens_.empty())
    {
        screens_.pop_back();
    }
}


Screen* ScreenManager::current()
{
    if (screens_.empty())
    {
        return nullptr;
    }

    return screens_.back().get();
}


bool ScreenManager::empty() const
{
    return screens_.empty();
}
}
