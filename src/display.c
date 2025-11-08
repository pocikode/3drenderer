#include <display.h>
#include <math.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

static SDL_Window *window = NULL;
static SDL_Renderer *renderer = NULL;

static uint32_t *color_buffer = NULL;
static float *z_buffer = NULL;

static SDL_Texture *color_buffer_texture = NULL;
static int window_width = 800;
static int window_height = 600;

int render_method = 0;
int cull_method = 0;

int get_window_width(void)
{
  return window_width;
}

int get_window_height(void)
{
  return window_height;
}

bool initialize_window(void)
{
  if (!SDL_Init(SDL_INIT_VIDEO))
  {
    fprintf(stderr, "Error: SDL_Init(): %s.\n", SDL_GetError());
    return false;
  }

  // query screen resolution
  int num_displays = 0;
  SDL_DisplayID *displays = SDL_GetDisplays(&num_displays);
  if (num_displays <= 0 || !displays)
  {
    fprintf(stderr, "Error: No displays found: %s\n", SDL_GetError());
    window_width = 800; // fallback resolution
    window_height = 600;
  }
  else
  {
    const SDL_DisplayMode *display_mode = SDL_GetCurrentDisplayMode(displays[0]);
    if (!display_mode)
    {
      fprintf(stderr, "Error: SDL_GetCurrentDisplayMode(): %s\n", SDL_GetError());
      window_width = 800; // fallback resolution
      window_height = 600;
    }
    else
    {
      window_width = display_mode->w;
      window_height = display_mode->h;
    }
    SDL_free(displays);
  }

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

  // allocate memory for color buffer and z-buffer
  color_buffer = (uint32_t *)malloc(sizeof(uint32_t) * window_width * window_height);
  z_buffer = (float *)malloc(sizeof(float) * window_width * window_height);

  //  create SDL texture for color buffer
  color_buffer_texture = SDL_CreateTexture(
    renderer,
    SDL_PIXELFORMAT_RGBA32,
    SDL_TEXTUREACCESS_STREAMING,
    window_width,
    window_height
  );

  return true;
}

void set_render_method(int method)
{
  render_method = method;
}

void set_cull_method(int method)
{
  cull_method = method;
}

bool is_cull_backface(void)
{
  return cull_method == CULL_BACKFACE;
}

bool should_render_filled_triangles(void)
{
  return render_method == RENDER_FILL_TRIANGLE || render_method == RENDER_FILL_TRIANGLE_WIRE;
}

bool should_render_textured_triangle(void)
{
  return render_method == RENDER_TEXTURED || render_method == RENDER_TEXTURED_WIRE;
}

bool should_render_wireframe(void)
{
  return render_method != RENDER_FILL_TRIANGLE && render_method != RENDER_TEXTURED;
}

bool should_render_wire_vertex(void)
{
  return render_method == RENDER_WIRE_VERTEX;
}

void draw_pixel(int x, int y, uint32_t color)
{
  if (x < 0 || x >= window_width || y < 0 || y >= window_height)
  {
    return;
  }
  color_buffer[(window_width * y) + x] = color;
}

void draw_line(int x0, int y0, int x1, int y1, uint32_t color)
{
  int dx = x1 - x0;
  int dy = y1 - y0;

  int steps = abs(dx) > abs(dy) ? abs(dx) : abs(dy);

  float inc_x = dx / (float)steps;
  float inc_y = dy / (float)steps;

  float x = x0;
  float y = y0;
  for (int i = 0; i <= steps; i++)
  {
    draw_pixel(round(x), round(y), color);
    x += inc_x;
    y += inc_y;
  }
}

void draw_triangle(int x0, int y0, int x1, int y1, int x2, int y2, uint32_t color)
{
  draw_line(x0, y0, x1, y1, color);
  draw_line(x1, y1, x2, y2, color);
  draw_line(x2, y2, x0, y0, color);
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
  SDL_RenderPresent(renderer);
}

void clear_color_buffer(uint32_t color)
{
  for (int i = 0; i < window_height * window_width; i++)
  {
    color_buffer[i] = color;
  }
}

void clear_z_buffer(void)
{
  for (int i = 0; i < window_height * window_width; i++)
  {
    z_buffer[i] = 1.0;
  }
}

float get_zbuffer_at(int x, int y)
{
  if (x < 0 || x >= window_width || y < 0 || y >= window_height)
  {
    return 1.0;
  }

  return z_buffer[(window_width * y) + x];
}

void set_zbuffer_at(int x, int y, float value)
{
  if (x < 0 || x >= window_width || y < 0 || y >= window_height)
  {
    return;
  }

  z_buffer[(window_width * y) + x] = value;
}

void destroy_window(void)
{
  free(color_buffer);
  free(z_buffer);

  SDL_DestroyRenderer(renderer);
  SDL_DestroyWindow(window);
  SDL_Quit();
}
