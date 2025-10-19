#include "array.h"
#include "display.h"
#include "mesh.h"
#include "triangle.h"
#include <stdint.h>
#include <stdlib.h>

triangle_t *triangles_to_render = NULL;

vec3_t camera_position = {0, 0, -5};

float fov_factor = 640;

bool is_running = false;
uint32_t previous_frame_time = 0;

void setup(void)
{
  color_buffer = (uint32_t *)malloc(sizeof(uint32_t) * window_width * window_height);

  color_buffer_texture = SDL_CreateTexture(
    renderer,
    SDL_PIXELFORMAT_ARGB8888,
    SDL_TEXTUREACCESS_STREAMING,
    window_width,
    window_height);

  load_cube_mesh_data();
}

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

// Perspective Projection
vec2_t project(vec3_t point)
{
  vec2_t projected_point = {
    .x = (fov_factor * point.x) / point.z,
    .y = (fov_factor * point.y) / point.z,
  };
  return projected_point;
}

void update(void)
{
  uint32_t time_to_wait = FRAME_TARGET_TIME - (SDL_GetTicks() - previous_frame_time);
  if (time_to_wait > 0 && time_to_wait <= FRAME_TARGET_TIME)
  {
    SDL_Delay(time_to_wait);
  }

  previous_frame_time = SDL_GetTicks();

  triangles_to_render = NULL;

  mesh.rotation.x += 0.01;
  mesh.rotation.y += 0.01;
  mesh.rotation.z += 0.01;

  int num_faces = array_length(mesh.faces);
  for (int i = 0; i < num_faces; i++)
  {
    face_t mesh_face = mesh.faces[i];

    vec3_t face_vertices[3];
    face_vertices[0] = mesh.vertices[mesh_face.a - 1];
    face_vertices[1] = mesh.vertices[mesh_face.b - 1];
    face_vertices[2] = mesh.vertices[mesh_face.c - 1];

    triangle_t projected_triangle;

    for (int j = 0; j < 3; j++)
    {
      vec3_t transformed_vertex = face_vertices[j];

      transformed_vertex = vec3_rotate_x(transformed_vertex, mesh.rotation.x);
      transformed_vertex = vec3_rotate_y(transformed_vertex, mesh.rotation.y);
      transformed_vertex = vec3_rotate_z(transformed_vertex, mesh.rotation.z);

      transformed_vertex.z -= camera_position.z;

      vec2_t projected_point = project(transformed_vertex);
      projected_point.x += (int)(window_width / 2);
      projected_point.y += (int)(window_height / 2);
      projected_triangle.points[j] = projected_point;
    }

    array_push(triangles_to_render, projected_triangle);
  }
};

void free_resources(void)
{
  free(color_buffer);
  array_free(mesh.vertices);
  array_free(mesh.faces);
}

void render(void)
{
  clear_color_buffer(0xFF000000);
  draw_grid();

  for (int i = 0; i < N_CUBE_FACES; i++)
  {
    triangle_t triangle = triangles_to_render[i];
    draw_triangle(
      triangle.points[0].x,
      triangle.points[0].y,
      triangle.points[1].x,
      triangle.points[1].y,
      triangle.points[2].x,
      triangle.points[2].y,
      0xFFFFFF00);
  }

  array_free(triangles_to_render);

  render_color_buffer();
  SDL_RenderPresent(renderer);
};

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
  free_resources();

  return 0;
}
