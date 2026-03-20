#pragma once

#include "data/models.hpp"
#include "platform/translation_catalog.hpp"
#include "ui/screen.hpp"

#include <vector>

namespace bytedeck::ui
{
class GamesScreen final : public Screen
{
public:
    GamesScreen(const data::LibraryData& library, const platform::TranslationCatalog& translations);

    ScreenAction handle_event(const SDL_Event& event) override;
    std::string screen_id() const override;
    UiBindings build_bindings() const override;
    std::string window_title() const override;

private:
    void move_selection(int delta);

    const platform::TranslationCatalog& translations_;
    std::vector<data::SystemEntry> entries_;
    std::size_t selected_index_ = 0;
};
}
