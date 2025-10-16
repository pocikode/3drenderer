#include <SDL3/SDL.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

bool is_running = false;

SDL_Window *window = NULL;
SDL_Renderer *renderer = NULL;

uint32_t *color_buffer = NULL;
SDL_Texture *color_buffer_texture = NULL;

int window_width = 800;
int window_height = 600;

bool initialize_window(void)
{
  if (!SDL_Init(SDL_INIT_VIDEO))
  {
    fprintf(stderr, "Error: SDL_Init(): %s.\n", SDL_GetError());
    return false;
  }

  // query screen resolution
  const SDL_DisplayMode *display_mode = SDL_GetCurrentDisplayMode(1);
  if (!display_mode)
  {
    fprintf(stderr, "Error: SDL_GetCurrentDisplayMode(): %s\n", SDL_GetError());
  }
  window_width = display_mode->w;
  window_height = display_mode->h;

  // create SDL window
  window = SDL_CreateWindow(NULL, window_width, window_height, SDL_WINDOW_BORDERLESS);
  if (!window)
  {
    fprintf(stderr, "Error: SDL_CreateWindow(): %s.\n", SDL_GetError());
    return false;
  }

  // create SDL renderer
  renderer = SDL_CreateRenderer(window, NULL);
  SDL_SetRenderVSync(renderer, 1);
  if (!renderer)
  {
    fprintf(stderr, "Error: SDL_CreateRenderer(): %s.\n", SDL_GetError());
    return false;
  }

  SDL_SetWindowPosition(window, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED);
  SDL_SetWindowFullscreen(window, true);
  SDL_ShowWindow(window);

  return true;
}

void setup(void)
{
  color_buffer = (uint32_t *)malloc(sizeof(uint32_t) * window_width * window_height);

  color_buffer_texture = SDL_CreateTexture(
    renderer,
    SDL_PIXELFORMAT_ARGB8888,
    SDL_TEXTUREACCESS_STREAMING,
    window_width,
    window_height);
};

void process_input(void)
{
  SDL_Event event;
  SDL_PollEvent(&event);

  switch (event.type)
  {
  case SDL_EVENT_QUIT:
    is_running = false;
    break;
  case SDL_EVENT_KEY_DOWN:
    if (event.key.key == SDLK_ESCAPE)
      is_running = false;
    break;
  }
};

void update(void) {
  // TODO:
};

void draw_grid(void)
{
  for (int y = 0; y < window_height; y += 10)
  {
    for (int x = 0; x < window_width; x += 10)
    {
      color_buffer[(window_width * y) + x] = 0xFF333333;
    }
  }
}

void draw_rect(int x, int y, int width, int height, uint32_t color)
{
  for (int row = y; row < height; row++)
  {
    for (int col = x; col < width; col++)
    {
      color_buffer[(window_width * row) + col] = color;
    }
  }
}

void render_color_buffer(void)
{
  SDL_UpdateTexture(color_buffer_texture, NULL, color_buffer, (window_width * sizeof(uint32_t)));
  SDL_RenderTexture(renderer, color_buffer_texture, NULL, NULL);
}

void clear_color_buffer(uint32_t color)
{
  for (int y = 0; y < window_height; y++)
  {
    for (int x = 0; x < window_width; x++)
    {
      color_buffer[(window_width * y) + x] = color;
    }
  }
}

void render(void)
{
  SDL_SetRenderDrawColor(renderer, 0, 0, 0, SDL_ALPHA_OPAQUE);
  SDL_RenderClear(renderer);

  draw_grid();
  draw_rect(40, 40, 500, 300, 0xFFFF00FF);
  render_color_buffer();
  clear_color_buffer(0xFF000000);

  SDL_RenderPresent(renderer);
};

void destroy_window(void)
{
  free(color_buffer);
  SDL_DestroyRenderer(renderer);
  SDL_DestroyWindow(window);
  SDL_Quit();
}

int main(void)
{
  is_running = initialize_window();

  setup();

  while (is_running)
  {
    process_input();
    update();
    render();
  }

  destroy_window();

  return 0;
}
