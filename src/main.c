#include <SDL2/SDL.h>
#include <SDL2/SDL_render.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdint.h>

bool is_running = false;

SDL_Window *window = NULL;
SDL_Renderer *renderer = NULL;

uint32_t *color_buffer = NULL;
SDL_Texture *color_buffer_texture = NULL;

int window_width = 800;
int window_height = 600;

bool initialize_window(void) {
    if (SDL_Init(SDL_INIT_EVERYTHING) != 0) {
        fprintf(stderr, "Error initializing SDL.\n");
        return false;
    }

    // Retrieve current display resolution and set the window dimensions
    SDL_DisplayMode displayMode;
    SDL_GetCurrentDisplayMode(0, &displayMode);
    window_width = displayMode.w;
    window_height = displayMode.h;

    // Create an SDL window in fullscreen mode
    window = SDL_CreateWindow(
        NULL,
        SDL_WINDOWPOS_CENTERED,
        SDL_WINDOWPOS_CENTERED,
        window_width,
        window_height,
        SDL_WINDOW_BORDERLESS
    );
    if (!window) {
        fprintf(stderr, "Error creating SDL window.\n");
        return false;
    }

    // Create an SDL renderer
    renderer = SDL_CreateRenderer(window, -1, 0);
    if (!renderer) {
        fprintf(stderr, "Error creating SDL renderer.\n");
        return false;
    }
    SDL_SetWindowFullscreen(window, SDL_WINDOW_FULLSCREEN);

    return true;
}

void setup(void) {
    // Allocate memory for the color buffer, which holds pixel data for the screen
    color_buffer = (uint32_t *) malloc(sizeof(uint32_t) * window_width * window_height);
    if (!color_buffer) {
        fprintf(stderr, "Error allocating memory for color buffer.\n");
        is_running = false;
    }

    // Create an SDL texture to display the color buffer
    color_buffer_texture = SDL_CreateTexture(
        renderer,
        SDL_PIXELFORMAT_ARGB8888,
        SDL_TEXTUREACCESS_STREAMING,
        window_width,
        window_height
    );
}

void process_input(void) {
    SDL_Event event;
    SDL_PollEvent(&event);

    switch (event.type) {
        case SDL_QUIT:
            is_running = false;
            break;
        case SDL_KEYDOWN:
            if (event.key.keysym.sym == SDLK_ESCAPE)
                is_running = false;
            break;
    }
}

void update(void) {
    // TODO
}

// Copy the color buffer to the texture and render it
void render_color_buffer(void) {
    SDL_UpdateTexture(
        color_buffer_texture,
        NULL,
        color_buffer,
        (int)(window_width * sizeof(uint32_t))
    );
    SDL_RenderCopy(renderer, color_buffer_texture, NULL, NULL);
}

void draw_grid(int gap, uint32_t color) {
    for (int y = 0; y < window_height; y++) {
        for (int x = 0; x < window_width; x++) {
            if (y % gap == 0 && x % gap == 0) {
                color_buffer[(window_width * y) + x] = color;
            }
        }
    }
}

void draw_rect(int x, int y, int width, int height, uint32_t color) {
    for (int i = 0; i < height; i++) {
        for (int j = 0; j < width; j++) {
            int current_x = x + j;
            int current_y = y + i;
            color_buffer[(window_width * current_y) + current_x] = color;
        }
    }
}

// Clear the color buffer with a specific color
void clear_color_buffer(uint32_t color) {
    for (int y = 0; y < window_height; y++) {
        for (int x = 0; x < window_width; x++) {
            color_buffer[(window_width * y) + x] = color;
        }
    }
}

void render(void) {
    SDL_SetRenderDrawColor(renderer, 255, 255, 0, 255);
    SDL_RenderClear(renderer);

    draw_grid(10, 0xFF333333);
    draw_rect(300, 200, 300, 150, 0xFFFF00FF);

    render_color_buffer();
    clear_color_buffer(0xFF000000);

    SDL_RenderPresent(renderer);
}

void destroy_window(void) {
    free(color_buffer);
    SDL_DestroyTexture(color_buffer_texture);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
}

int main(void) {
    is_running = initialize_window();

    setup();

    while (is_running) {
        process_input();
        update();
        render();
    }

    destroy_window();

    return 0;
}
