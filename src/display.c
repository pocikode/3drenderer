#include <display.h>
#include <stdint.h>
#include <stdlib.h>

SDL_Window *window = NULL;
SDL_Renderer *renderer = NULL;
SDL_Texture *color_buffer_texture = NULL;
uint32_t *color_buffer = NULL;
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

void draw_pixel(int x, int y, uint32_t color)
{
  if (x >= 0 && x < window_width && y >= 0 && y < window_height)
  {
    color_buffer[(window_width * y) + x] = color;
  }
}

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
  for (int row = y; row < y + height; row++)
  {
    for (int col = x; col < x + width; col++)
    {
      draw_pixel(col, row, color);
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

void destroy_window(void)
{
  free(color_buffer);
  SDL_DestroyRenderer(renderer);
  SDL_DestroyWindow(window);
  SDL_Quit();
}
