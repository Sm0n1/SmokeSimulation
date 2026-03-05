#include <SDL3/SDL.h>

int main(int argc, char *argv[])
{
    if (SDL_Init(SDL_INIT_VIDEO) < 0)
    {
        SDL_Log("SDL init failed: %s", SDL_GetError());
        return 1;
    }

    SDL_Window *window = SDL_CreateWindow(
        "SDL3 Window",
        800,
        600,
        0
    );

    SDL_Delay(3000);

    SDL_DestroyWindow(window);
    SDL_Quit();
}