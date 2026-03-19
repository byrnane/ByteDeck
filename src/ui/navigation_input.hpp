#pragma once

#include <SDL.h>

#include <cstdlib>
#include <string_view>

namespace bytedeck::ui
{
enum class NavigationInput
{
    none,
    up,
    down,
    left,
    right,
    accept,
    back,
    quit
};

namespace detail
{
constexpr Uint8 kTrimuiButtonB = 0;
constexpr Uint8 kTrimuiButtonA = 1;
constexpr Uint8 kTrimuiButtonMenu = 8;
constexpr Sint16 kJoystickAxisThreshold = 16000;

enum class InputBackend
{
    auto_detect,
    controller_only,
    joystick_only
};

inline InputBackend input_backend()
{
    const char* raw = std::getenv("BYTEDECK_INPUT_BACKEND");
    if (raw == nullptr || *raw == '\0')
    {
        return InputBackend::auto_detect;
    }

    const std::string_view value(raw);
    if (value == "joystick")
    {
        return InputBackend::joystick_only;
    }
    if (value == "controller")
    {
        return InputBackend::controller_only;
    }
    return InputBackend::auto_detect;
}
}

inline NavigationInput navigation_input_from_event(const SDL_Event& event)
{
    const detail::InputBackend backend = detail::input_backend();

    switch (event.type)
    {
    case SDL_KEYDOWN:
        if (event.key.repeat != 0)
        {
            return NavigationInput::none;
        }
        switch (event.key.keysym.sym)
        {
        case SDLK_UP:
            return NavigationInput::up;
        case SDLK_DOWN:
            return NavigationInput::down;
        case SDLK_LEFT:
            return NavigationInput::left;
        case SDLK_RIGHT:
            return NavigationInput::right;
        case SDLK_RETURN:
        case SDLK_KP_ENTER:
            return NavigationInput::accept;
        case SDLK_ESCAPE:
            return NavigationInput::back;
        case SDLK_q:
            return NavigationInput::quit;
        default:
            return NavigationInput::none;
        }

    case SDL_CONTROLLERBUTTONDOWN:
        if (backend == detail::InputBackend::joystick_only)
        {
            return NavigationInput::none;
        }
        switch (event.cbutton.button)
        {
        case SDL_CONTROLLER_BUTTON_DPAD_UP:
            return NavigationInput::up;
        case SDL_CONTROLLER_BUTTON_DPAD_DOWN:
            return NavigationInput::down;
        case SDL_CONTROLLER_BUTTON_DPAD_LEFT:
            return NavigationInput::left;
        case SDL_CONTROLLER_BUTTON_DPAD_RIGHT:
            return NavigationInput::right;
        case SDL_CONTROLLER_BUTTON_A:
            return NavigationInput::accept;
        case SDL_CONTROLLER_BUTTON_B:
            return NavigationInput::back;
        default:
            return NavigationInput::none;
        }

    case SDL_JOYBUTTONDOWN:
        if (backend == detail::InputBackend::controller_only)
        {
            return NavigationInput::none;
        }
        switch (event.jbutton.button)
        {
        case detail::kTrimuiButtonA:
            return NavigationInput::accept;
        case detail::kTrimuiButtonB:
            return NavigationInput::back;
        case detail::kTrimuiButtonMenu:
            return NavigationInput::quit;
        default:
            return NavigationInput::none;
        }

    case SDL_JOYHATMOTION:
        if (backend == detail::InputBackend::controller_only)
        {
            return NavigationInput::none;
        }
        if ((event.jhat.value & SDL_HAT_UP) != 0)
        {
            return NavigationInput::up;
        }
        if ((event.jhat.value & SDL_HAT_DOWN) != 0)
        {
            return NavigationInput::down;
        }
        if ((event.jhat.value & SDL_HAT_LEFT) != 0)
        {
            return NavigationInput::left;
        }
        if ((event.jhat.value & SDL_HAT_RIGHT) != 0)
        {
            return NavigationInput::right;
        }
        return NavigationInput::none;

    case SDL_JOYAXISMOTION:
        if (backend == detail::InputBackend::controller_only)
        {
            return NavigationInput::none;
        }
        if (event.jaxis.axis == 0)
        {
            if (event.jaxis.value <= -detail::kJoystickAxisThreshold)
            {
                return NavigationInput::left;
            }
            if (event.jaxis.value >= detail::kJoystickAxisThreshold)
            {
                return NavigationInput::right;
            }
        }
        else if (event.jaxis.axis == 1)
        {
            if (event.jaxis.value <= -detail::kJoystickAxisThreshold)
            {
                return NavigationInput::up;
            }
            if (event.jaxis.value >= detail::kJoystickAxisThreshold)
            {
                return NavigationInput::down;
            }
        }
        return NavigationInput::none;

    default:
        return NavigationInput::none;
    }
}


inline NavigationInput navigation_input_release_from_event(const SDL_Event& event)
{
    const detail::InputBackend backend = detail::input_backend();

    switch (event.type)
    {
    case SDL_KEYUP:
        switch (event.key.keysym.sym)
        {
        case SDLK_UP:
            return NavigationInput::up;
        case SDLK_DOWN:
            return NavigationInput::down;
        case SDLK_LEFT:
            return NavigationInput::left;
        case SDLK_RIGHT:
            return NavigationInput::right;
        default:
            return NavigationInput::none;
        }

    case SDL_CONTROLLERBUTTONUP:
        if (backend == detail::InputBackend::joystick_only)
        {
            return NavigationInput::none;
        }
        switch (event.cbutton.button)
        {
        case SDL_CONTROLLER_BUTTON_DPAD_UP:
            return NavigationInput::up;
        case SDL_CONTROLLER_BUTTON_DPAD_DOWN:
            return NavigationInput::down;
        case SDL_CONTROLLER_BUTTON_DPAD_LEFT:
            return NavigationInput::left;
        case SDL_CONTROLLER_BUTTON_DPAD_RIGHT:
            return NavigationInput::right;
        default:
            return NavigationInput::none;
        }

    default:
        return NavigationInput::none;
    }
}


inline bool is_repeatable_navigation_input(NavigationInput input)
{
    switch (input)
    {
    case NavigationInput::up:
    case NavigationInput::down:
    case NavigationInput::left:
    case NavigationInput::right:
        return true;
    case NavigationInput::none:
    case NavigationInput::accept:
    case NavigationInput::back:
    case NavigationInput::quit:
    default:
        return false;
    }
}
}
