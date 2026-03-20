#pragma once

#include "platform/translation_catalog.hpp"
#include "ui/screen.hpp"

#include <string>

namespace bytedeck::ui
{
class PlaceholderScreen final : public Screen
{
public:
    PlaceholderScreen(std::string title, const platform::TranslationCatalog& translations);

    ScreenAction handle_event(const SDL_Event& event) override;
    std::string screen_id() const override;
    UiBindings build_bindings() const override;
    std::string window_title() const override;

private:
    const platform::TranslationCatalog& translations_;
    std::string title_;
};
}
