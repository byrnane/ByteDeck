#pragma once

#include "ui/screen.hpp"

namespace bytedeck::ui
{
class SettingsStubScreen final : public Screen
{
public:
    ScreenAction handle_event(const SDL_Event& event) override;
    std::string screen_id() const override;
    UiBindings build_bindings() const override;
    std::string window_title() const override;
};
}
