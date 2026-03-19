#pragma once

#include "ui/screen.hpp"

#include <memory>
#include <vector>

namespace bytedeck::ui
{
class ScreenManager
{
public:
    void push(std::unique_ptr<Screen> screen);
    void pop();
    Screen* current();
    bool empty() const;

private:
    std::vector<std::unique_ptr<Screen>> screens_;
};
}
