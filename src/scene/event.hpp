#pragma once

struct Event {
    enum struct Type {
        SDL,
    };

    Type m_type;

    SDL_Event m_sdl_event;

    bool m_consumed;
};