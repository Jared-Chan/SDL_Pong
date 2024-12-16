
#define SDL_MAIN_USE_CALLBACKS 1 /* use the callbacks instead of main() */
#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>
#include "sdl_pong.hpp"

constexpr int screenWidth {640};
constexpr int screenHeight {480};

// From SDL3 examples
static const struct {
    const char *key;
    const char *value;
} extended_metadata[] = {
    { SDL_PROP_APP_METADATA_URL_STRING, " " },
    { SDL_PROP_APP_METADATA_CREATOR_STRING, "JC" },
    { SDL_PROP_APP_METADATA_COPYRIGHT_STRING, "Placed in the public domain" },
    { SDL_PROP_APP_METADATA_TYPE_STRING, "game" }
};

SDL_AppResult SDL_AppInit(void **appstate, int argc, char *argv[]) {

    if (!SDL_SetAppMetadata("Pong", "1.0", "com.example.pong-1234")) {
        return SDL_APP_FAILURE;
    }
    int i;
    for (i = 0; i < SDL_arraysize(extended_metadata); i++) {
        if (!SDL_SetAppMetadataProperty(extended_metadata[i].key, extended_metadata[i].value)) {
            return SDL_APP_FAILURE;
        }
    }

    SdlPong::AppState *as = new SdlPong::AppState(screenWidth, screenHeight);
    if (!as) {
        return SDL_APP_FAILURE;
    } else {
        *appstate = as;
    }

    if (!SDL_Init(SDL_INIT_VIDEO)) {
        return SDL_APP_FAILURE;
    }
    if (!SDL_CreateWindowAndRenderer("examples/demo/woodeneye-008", 640, 480, 0, &as->mWindow, &as->mRenderer)) {
        return SDL_APP_FAILURE;
    }
    SDL_SetRenderVSync(as->mRenderer, true);

    return SDL_APP_CONTINUE;
}

SDL_AppResult SDL_AppIterate(void *appstate) 
{
    SdlPong::AppState *as = static_cast<SdlPong::AppState*>(appstate);

    as->UpdatePositions();
    as->CheckCollisions();
    as->ProcessCollisions();
    as->Render();

    return SDL_APP_CONTINUE;
}

SDL_AppResult SDL_AppEvent(void *appstate, SDL_Event *event) {
    SdlPong::AppState *as = static_cast<SdlPong::AppState*>(appstate);
    switch (event->type) {
        case SDL_EVENT_QUIT:
            return SDL_APP_SUCCESS;
            break;
        case SDL_EVENT_KEY_DOWN: {
            SDL_Keycode sym = event->key.key;
            if (sym == SDLK_W) as->moveBar(SdlPong::left, SdlPong::up);
            if (sym == SDLK_S) as->moveBar(SdlPong::left, SdlPong::down);
            if (sym == SDLK_UP) as->moveBar(SdlPong::right, SdlPong::up);
            if (sym == SDLK_DOWN) as->moveBar(SdlPong::right, SdlPong::down);
            break;
        }
        case SDL_EVENT_KEY_UP: {
            SDL_Keycode sym = event->key.key;
            if (sym == SDLK_W) as->moveBar(SdlPong::left, SdlPong::none);
            if (sym == SDLK_S) as->moveBar(SdlPong::left, SdlPong::none);
            if (sym == SDLK_UP) as->moveBar(SdlPong::right, SdlPong::none);
            if (sym == SDLK_DOWN) as->moveBar(SdlPong::right, SdlPong::none);
            break;
        }
    }
    return SDL_APP_CONTINUE;
}

void SDL_AppQuit(void *appstate, SDL_AppResult result) {

    SdlPong::AppState *as = static_cast<SdlPong::AppState*>(appstate);
    delete as;
}
