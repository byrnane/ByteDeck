#pragma once

#include "data/models.hpp"
#include "platform/translation_catalog.hpp"
#include "ui/screen.hpp"

#include <string>
#include <vector>

namespace bytedeck::ui
{
class MainMenuScreen final : public Screen
{
public:
    MainMenuScreen(const data::LibraryData& library, const platform::TranslationCatalog& translations);

    ScreenAction handle_event(const SDL_Event& event) override;
    std::string screen_id() const override;
    UiBindings build_bindings() const override;
    std::string window_title() const override;

private:
    void move_selection(int delta);

    const data::LibraryData& library_;
    const platform::TranslationCatalog& translations_;
    std::vector<std::string> items_;
    std::size_t selected_index_ = 0;
};
}
