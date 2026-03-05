#include <SDL3/SDL.h>
#include <stdio.h>

#define N 512
#define SIZE ((N + 2) * (N + 2))
#define IX(i, j) ((i) + (j) * (N + 2))
#define DT (1.0 / 60.0)

static float u[SIZE];
static float v[SIZE];
static float u_prev[SIZE];
static float v_prev[SIZE];
static float density[SIZE];
static float density_prev[SIZE];

float mouse_x = 0.0f;
float mouse_y = 0.0f;
int is_mouse_pressed = false;

void update_state()
{
    if (is_mouse_pressed)
    {
        int i_lower_bound = (mouse_x >= 17)     ? ((int)mouse_x - 16) : (1);
        int i_upper_bound = (mouse_x <= N - 15) ? ((int)mouse_x + 16) : (N + 1);
        int j_lower_bound = (mouse_y >= 17)     ? ((int)mouse_y - 16) : (1);
        int j_upper_bound = (mouse_y <= N - 15) ? ((int)mouse_y + 16) : (N + 1);

        for (int i = i_lower_bound; i < i_upper_bound; i += 1)
        {
            for (int j = j_lower_bound; j < j_upper_bound; j += 1)
            {
                density[IX(i, j)] = 255;
            }
        }
    }
}

void render_state(SDL_Renderer *renderer, SDL_Texture *texture)
{
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
    SDL_RenderClear(renderer);

    SDL_Color* pixels;
    int pitch;
    SDL_LockTexture(texture, NULL, (void**)&pixels, &pitch);
    int stride = pitch / sizeof(SDL_Color);
    for (int i = 0; i < N + 2; i += 1)
    {
        for (int j = 0; j < N + 2; j += 1)
        {
            pixels[i + j * stride] = (SDL_Color){ .r = (Uint8)density[IX(i, j)], .g = 0, .b = 0, .a = 255 };
        }
    }
    SDL_UnlockTexture(texture);
    SDL_FRect srcrect = { .x = 0, .y = 0, .w = N + 2, .h = N + 2 };
    SDL_FRect dstrect = { .x = -1, .y = -1, .w = N + 2, .h = N + 2 };
    SDL_RenderTexture(renderer, texture, &srcrect, &dstrect);
    (void)texture;

    const SDL_FRect rect = { .x = mouse_x - 8, .y = mouse_y - 8, .w = 16, .h = 16 };
    SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
    SDL_RenderFillRect(renderer, &rect);

    SDL_RenderPresent(renderer);
}

int main(int argc, char *argv[])
{
    (void)argc;
    (void)argv;

    if (SDL_Init(SDL_INIT_VIDEO) < 0)
    {
        SDL_Log("SDL init failed: %s", SDL_GetError());
        return 1;
    }

    SDL_Window *window = SDL_CreateWindow("Smoke", N, N, 0);
    if (window == NULL)
    {
        SDL_Log("SDL create window failed: %s", SDL_GetError());
        return 1;
    }

    SDL_Renderer* renderer = SDL_CreateRenderer(window, NULL);
    if (renderer == NULL)
    {
        SDL_Log("SDL create renderer failed: %s", SDL_GetError());
        return 1;
    }

    SDL_Texture* texture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGBA32, SDL_TEXTUREACCESS_STREAMING, N + 2, N + 2);
    if (texture == NULL)
    {
        SDL_Log("SDL create texture failed: %s", SDL_GetError());
        return 1;
    }

    Uint64 time = SDL_GetTicksNS();
    Uint64 time_accumulator = 0;

    bool is_running = true;
    while (is_running)
    {
        // Update time
        Uint64 frame_time = SDL_GetTicksNS() - time;
        time += frame_time;
        time_accumulator += frame_time;

        // This part handles all the SDL events
        SDL_Event event;
        while (SDL_PollEvent(&event))
        {
            switch (event.type)
            {
            case SDL_EVENT_QUIT:
            case SDL_EVENT_WINDOW_CLOSE_REQUESTED:
                is_running = false;
                break;
            case SDL_EVENT_MOUSE_MOTION:
                mouse_x = event.motion.x;
                mouse_y = event.motion.y;
                break;
            case SDL_EVENT_MOUSE_BUTTON_DOWN:
                is_mouse_pressed = true;
                break;
            case SDL_EVENT_MOUSE_BUTTON_UP:
                is_mouse_pressed = false;
                break;
            }
        }

        // If there is a lag spike, then we may need to simulate multiple time steps
        while (time_accumulator >= SDL_NS_PER_SECOND / 60)
        {
            update_state();
            time_accumulator -= SDL_NS_PER_SECOND / 60;
        }
        
        render_state(renderer, texture);

        SDL_Log("Frametime: %f (%f)", (double)frame_time / SDL_NS_PER_SECOND, ((double)frame_time / SDL_NS_PER_SECOND) / (SDL_NS_PER_SECOND / 60.0));
    }

    SDL_DestroyWindow(window);
    SDL_Quit();

    return 0;
}