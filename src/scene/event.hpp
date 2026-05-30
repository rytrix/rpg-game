#pragma once

struct Event {
    enum Type {
        SDL,
    };

    Type m_type;

    SDL_Event m_sdl_event;

    bool m_consumed;
};