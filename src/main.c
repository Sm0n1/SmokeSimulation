#include <SDL3/SDL.h>
#include <stdio.h>
#include "simulation.h"

#define N 128
#define SIZE ((N + 2) * (N + 2))
#define DT (1.0 / 60.0)

#define WINDOW_SIZE ((N + 2) * 4)
#define SMOKE_SPAWN_RADIUS 8

static float u[SIZE];
static float v[SIZE];
static float u_prev[SIZE];
static float v_prev[SIZE];
static float density[SIZE];
static float density_prev[SIZE];
static bool boundary[SIZE];

enum interaction_mode {
    DENSITY = 1 << 0,
    VELOCITY = 1 << 1,
};

static float mouse_x_pos = 0.0f;
static float mouse_y_pos = 0.0f;
static float mouse_x_vel = 0.0f;
static float mouse_y_vel = 0.0f;
static bool is_mouse_pressed = false;
static enum interaction_mode mode = DENSITY | VELOCITY;
static bool should_state_reset = false;
static bool smoke_pillar = false;

int win_to_tex(float c)
{
    return (int)(c * ((1.0 / WINDOW_SIZE) * (N + 2)));
}

float tex_to_win(int c)
{
    return (float)(c * ((1.0 / (N + 2)) * WINDOW_SIZE));
}

void init_state()
{
    for (int i = (N / 2) - (N / 8); i < (N / 2) + (N / 8); i += 1)
    {
        for (int j = (N / 2) - (N / 8); j < (N / 2) + (N / 8); j += 1)
        {
            const int di = i - N / 2;
            const int dj = j - N / 2;

            if (di * di + dj * dj < N * N / 64)
            {
                boundary[IX(i, j)] = true;
            }
        }
    }
}

void update_state()
{
    if (smoke_pillar)
    {
        const int center = N / 2 + 1;
        const int half_width = 16;
        for (int i = center - half_width; i <= center + half_width; i += 1)
        {
            density[IX(i, N)]     = 200.0f;
            density[IX(i, N - 1)] = 200.0f;
            density[IX(i, N - 2)] = 200.0f;
            density[IX(i, N - 3)] = 200.0f;

            for (int j = 1; j <= N + 1; j += 1)
            {
                v[IX(i, j)] = -0.5f;
            }
        }
    }

    if (is_mouse_pressed && (mode & DENSITY))
    {
        const int x = win_to_tex(mouse_x_pos);
        const int y = win_to_tex(mouse_y_pos);
        const int i_lower_bound = (x - SMOKE_SPAWN_RADIUS >= 0)     ? (x - SMOKE_SPAWN_RADIUS) : (0);
        const int i_upper_bound = (x + SMOKE_SPAWN_RADIUS <= N + 2) ? (x + SMOKE_SPAWN_RADIUS) : (N + 2);
        const int j_lower_bound = (y - SMOKE_SPAWN_RADIUS >= 0)     ? (y - SMOKE_SPAWN_RADIUS) : (0);
        const int j_upper_bound = (y + SMOKE_SPAWN_RADIUS <= N + 2) ? (y + SMOKE_SPAWN_RADIUS) : (N + 2);

        for (int i = i_lower_bound; i < i_upper_bound; i += 1)
        {
            for (int j = j_lower_bound; j < j_upper_bound; j += 1)
            {
                if (!boundary[IX(i, j)])
                {
                    density[IX(i, j)] = 200.0f;
                }
            }
        }
    }

    if (is_mouse_pressed && (mode & VELOCITY))
    {
        const int x = win_to_tex(mouse_x_pos);
        const int y = win_to_tex(mouse_y_pos);
        const int i_lower_bound = (x - SMOKE_SPAWN_RADIUS >= 0) ? (x - SMOKE_SPAWN_RADIUS) : (0);
        const int i_upper_bound = (x + SMOKE_SPAWN_RADIUS <= N + 2) ? (x + SMOKE_SPAWN_RADIUS) : (N + 2);
        const int j_lower_bound = (y - SMOKE_SPAWN_RADIUS >= 0) ? (y - SMOKE_SPAWN_RADIUS) : (0);
        const int j_upper_bound = (y + SMOKE_SPAWN_RADIUS <= N + 2) ? (y + SMOKE_SPAWN_RADIUS) : (N + 2);

        for (int i = i_lower_bound; i < i_upper_bound; i += 1)
        {
            for (int j = j_lower_bound; j < j_upper_bound; j += 1)
            {
                if (!boundary[IX(i, j)])
                {
                    u[IX(i, j)] += mouse_x_vel;
                    v[IX(i, j)] += mouse_y_vel;
                }
            }
        }
    }

    mouse_x_vel *= 0.9f;
    mouse_y_vel *= 0.9f;

    if (should_state_reset)
    {
        for (int i = 0; i < SIZE; i += 1)
        {
            u[i] = 0;
            v[i] = 0;
            u_prev[i] = 0;
            v_prev[i] = 0;
            density[i] = 0;
            density_prev[i] = 0;
        }

        should_state_reset = false;
    }

    vel_step(N, u, v, u_prev, v_prev, 0.0001f, (float)DT, boundary);
    dens_step(N, density, density_prev, u, v, 0.0001f, (float)DT, boundary);
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
            pixels[i + j * stride] = (SDL_Color){ .r = 255, .g = 255, .b = 255, .a = (Uint8)density[IX(i, j)] };
            
            if (boundary[IX(i, j)])
            {
                pixels[i + j * stride] = (SDL_Color){ .r = 200, .g = 255, .b = 155, .a = 255 };
            }
        }
    }
    SDL_UnlockTexture(texture);
    SDL_FRect srcrect = { .x = 0, .y = 0, .w = N + 2, .h = N + 2 };
    SDL_FRect dstrect = { .x = 0, .y = 0, .w = WINDOW_SIZE, .h = WINDOW_SIZE };
    SDL_RenderTexture(renderer, texture, &srcrect, &dstrect);

    const SDL_FRect rect = {
        .x = tex_to_win(win_to_tex(mouse_x_pos) - SMOKE_SPAWN_RADIUS),
        .y = tex_to_win(win_to_tex(mouse_y_pos) - SMOKE_SPAWN_RADIUS),
        .w = tex_to_win(2 * SMOKE_SPAWN_RADIUS),
        .h = tex_to_win(2 * SMOKE_SPAWN_RADIUS),
    };
    SDL_SetRenderDrawColor(renderer, 255, 255, 255, 64);
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

    SDL_Window *window = SDL_CreateWindow("Smoke", WINDOW_SIZE, WINDOW_SIZE, 0);
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
    SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);

    SDL_Texture* texture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGBA32, SDL_TEXTUREACCESS_STREAMING, N + 2, N + 2);
    if (texture == NULL)
    {
        SDL_Log("SDL create texture failed: %s", SDL_GetError());
        return 1;
    }

    init_state();

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
                mouse_x_pos = event.motion.x;
                mouse_y_pos = event.motion.y;
                mouse_x_vel = event.motion.xrel * 0.2f;
                mouse_y_vel = event.motion.yrel * 0.2f;
                break;
            case SDL_EVENT_MOUSE_BUTTON_DOWN:
                is_mouse_pressed = true;
                break;
            case SDL_EVENT_MOUSE_BUTTON_UP:
                is_mouse_pressed = false;
                break;
            case SDL_EVENT_KEY_DOWN:
                if (event.key.scancode == SDL_SCANCODE_D) mode ^= DENSITY;
                if (event.key.scancode == SDL_SCANCODE_V) mode ^= VELOCITY;
                if (event.key.scancode == SDL_SCANCODE_R) should_state_reset = true;
                if (event.key.scancode == SDL_SCANCODE_P) smoke_pillar ^= true;

                break;

            // When moving the window, this ensures the simulation is paused.
            case SDL_EVENT_WINDOW_EXPOSED:
                time += SDL_GetTicksNS() - time;
            }
        }

        // If there is a lag spike, then we may need to simulate multiple time steps
        while (time_accumulator >= SDL_NS_PER_SECOND / 60)
        {
            update_state();
            time_accumulator -= SDL_NS_PER_SECOND / 60;
        }
        
        render_state(renderer, texture);

        //SDL_Log("Frametime: %f (%f)", (double)frame_time / SDL_NS_PER_SECOND, ((double)frame_time / SDL_NS_PER_SECOND) / (1.0 / 60.0));
    }

    SDL_DestroyWindow(window);
    SDL_Quit();

    return 0;
}