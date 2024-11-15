#include <stdio.h>
#include "display.h"

bool is_running = false;

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

void render(void) {
    SDL_SetRenderDrawColor(renderer, 255, 255, 0, 255);
    SDL_RenderClear(renderer);

    draw_grid(10, 0xFF333333);
    draw_rect(300, 200, 300, 150, 0xFFFF00FF);
    draw_pixel(30, 30, 0xFFFFFF00);

    render_color_buffer();
    clear_color_buffer(0xFF000000);

    SDL_RenderPresent(renderer);
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
