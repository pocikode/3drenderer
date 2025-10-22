#include "array.h"
#include "display.h"
#include "matrix.h"
#include "mesh.h"
#include "triangle.h"
#include "vector.h"
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

triangle_t *triangles_to_render = NULL;

vec3_t camera_position = {0, 0, 0};
float fov_factor = 640;

bool is_running = false;
uint32_t previous_frame_time = 0;

void setup(void)
{
  render_method = RENDER_WIRE;
  cull_method = CULL_BACKFACE;

  color_buffer = (uint32_t *)malloc(sizeof(uint32_t) * window_width * window_height);

  color_buffer_texture = SDL_CreateTexture(
    renderer,
    SDL_PIXELFORMAT_ARGB8888,
    SDL_TEXTUREACCESS_STREAMING,
    window_width,
    window_height);

  load_cube_mesh_data();
  // load_obj_file_data("../assets/cube.obj");
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
    switch (event.key.key)
    {
    case SDLK_ESCAPE:
      is_running = false;
      break;
    case SDLK_C:
      cull_method = CULL_BACKFACE;
      break;
    case SDLK_D:
      cull_method = CULL_NONE;
      break;
    case SDLK_1:
      render_method = RENDER_WIRE_VERTEX;
      break;
    case SDLK_2:
      render_method = RENDER_WIRE;
      break;
    case SDLK_3:
      render_method = RENDER_FILL_TRIANGLE;
      break;
    case SDLK_4:
      render_method = RENDER_FILL_TRIANGLE_WIRE;
      break;
    }
    break; // break swich key down event
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

  // change mesh rotation / scale / translation per frame
  mesh.rotation.x += 0.01;
  mesh.rotation.y += 0.01;
  mesh.rotation.z += 0.01;
  // mesh.scale.x += 0.002;
  mesh.translation.x += 0.01;
  mesh.translation.z = 5; // move away object from camera

  // create matrix for perfoming rotation / scale / translation
  mat4_t scale_matrix = mat4_make_scale(mesh.scale.x, mesh.scale.y, mesh.scale.z);
  mat4_t translation_matrix = mat4_make_translation(mesh.translation.x, mesh.translation.y, mesh.translation.z);
  mat4_t rotation_matrix_x = mat4_make_rotation_x(mesh.rotation.x);
  mat4_t rotation_matrix_y = mat4_make_rotation_y(mesh.rotation.y);
  mat4_t rotation_matrix_z = mat4_make_rotation_z(mesh.rotation.z);

  int num_faces = array_length(mesh.faces);
  for (int i = 0; i < num_faces; i++)
  {
    face_t mesh_face = mesh.faces[i];

    vec3_t face_vertices[3];
    face_vertices[0] = mesh.vertices[mesh_face.a - 1];
    face_vertices[1] = mesh.vertices[mesh_face.b - 1];
    face_vertices[2] = mesh.vertices[mesh_face.c - 1];

    // perform transformations
    vec4_t transformed_vertices[3];
    for (int j = 0; j < 3; j++)
    {
      vec4_t transformed_vertex = vec4_from_vec3(face_vertices[j]);

      // scale using matrix
      // transformed_vertex = mat4_mul_vec4(scale_matrix, transformed_vertex);

      // rotation using matrix
      transformed_vertex = mat4_mul_vec4(rotation_matrix_x, transformed_vertex);
      transformed_vertex = mat4_mul_vec4(rotation_matrix_y, transformed_vertex);
      transformed_vertex = mat4_mul_vec4(rotation_matrix_z, transformed_vertex);

      // translation using matrix
      transformed_vertex = mat4_mul_vec4(translation_matrix, transformed_vertex);

      transformed_vertices[j] = transformed_vertex;
    }

    // perform back-face culling
    if (cull_method == CULL_BACKFACE)
    {
      vec3_t vector_a = vec3_from_vec4(transformed_vertices[0]);
      vec3_t vector_b = vec3_from_vec4(transformed_vertices[1]);
      vec3_t vector_c = vec3_from_vec4(transformed_vertices[2]);
      vec3_t vector_ab = vec3_sub(vector_b, vector_a);
      vec3_t vector_ac = vec3_sub(vector_c, vector_a);
      vec3_normalize(&vector_ab);
      vec3_normalize(&vector_ac);

      // get the face normal
      vec3_t vector_normal = vec3_cross(vector_ab, vector_ac);
      vec3_normalize(&vector_normal);

      vec3_t camera_ray = vec3_sub(camera_position, vector_a);
      float dot_normal_camera = vec3_dot(vector_normal, camera_ray);
      if (dot_normal_camera < 0)
      {
        continue;
      }
    }

    // perform projection
    vec2_t projected_points[3];
    for (int j = 0; j < 3; j++)
    {
      projected_points[j] = project(vec3_from_vec4(transformed_vertices[j]));
      projected_points[j].x += (int)(window_width / 2);
      projected_points[j].y += (int)(window_height / 2);
    }

    float avg_depth = (transformed_vertices[0].z + transformed_vertices[1].z + transformed_vertices[2].z) / 3.0;
    triangle_t projected_triangle = {
      .points = {
        {projected_points[0].x, projected_points[0].y},
        {projected_points[1].x, projected_points[1].y},
        {projected_points[2].x, projected_points[2].y},
      },
      .color = mesh_face.color,
      .avg_depth = avg_depth,
    };
    array_push(triangles_to_render, projected_triangle);
  }

  sort_triangles(triangles_to_render);
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

  int num_triangles = array_length(triangles_to_render);
  for (int i = 0; i < num_triangles; i++)
  {
    triangle_t triangle = triangles_to_render[i];

    // draw filled triangle
    if (render_method == RENDER_FILL_TRIANGLE || render_method == RENDER_FILL_TRIANGLE_WIRE)
    {
      draw_filled_triangle(
        triangle.points[0].x,
        triangle.points[0].y,
        triangle.points[1].x,
        triangle.points[1].y,
        triangle.points[2].x,
        triangle.points[2].y,
        triangle.color);
    }

    // draw wireframe
    if (render_method != RENDER_FILL_TRIANGLE)
    {
      draw_triangle(
        triangle.points[0].x,
        triangle.points[0].y,
        triangle.points[1].x,
        triangle.points[1].y,
        triangle.points[2].x,
        triangle.points[2].y,
        0xFFFFFFFF);
    }

    // draw the vertex
    if (render_method == RENDER_WIRE_VERTEX)
    {
      draw_rect(triangle.points[0].x - 3, triangle.points[0].y - 3, 6, 6, 0xFFFF0000);
      draw_rect(triangle.points[1].x - 3, triangle.points[1].y - 3, 6, 6, 0xFFFF0000);
      draw_rect(triangle.points[2].x - 3, triangle.points[2].y - 3, 6, 6, 0xFFFF0000);
    }
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
