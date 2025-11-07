#include "array.h"
#include "camera.h"
#include "clipping.h"
#include "display.h"
#include "light.h"
#include "matrix.h"
#include "mesh.h"
#include "texture.h"
#include "triangle.h"
#include "vector.h"
#include <math.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

// Global variable for runtime status and game loop
bool is_running = false;
uint32_t previous_frame_time = 0;
float delta_time = 0;

// Array to store triangles that should be rendered each frame
#define MAX_TRIANGLES 10000
triangle_t triangles_to_render[MAX_TRIANGLES];
int num_triangles_to_render = 0;

// Declaration of global transformation vertices
mat4_t world_matrix;
mat4_t proj_matrix;
mat4_t view_matrix;

void setup(void)
{
  render_method = RENDER_TEXTURED;
  cull_method = CULL_BACKFACE;

  color_buffer = (uint32_t *)malloc(sizeof(uint32_t) * window_width * window_height);
  z_buffer = (float *)malloc(sizeof(float) * window_width * window_height);

  color_buffer_texture = SDL_CreateTexture(
    renderer,
    SDL_PIXELFORMAT_RGBA32,
    SDL_TEXTUREACCESS_STREAMING,
    window_width,
    window_height
  );

  // initialize perspective projection matrix
  float aspect_x = window_width / (float)window_height;
  float aspect_y = window_height / (float)window_width;
  float fov_y = M_PI / 3.0;
  float fov_x = atan(tan(fov_y / 2.0) * aspect_x) * 2.0;
  float z_near = 0.1;
  float z_far = 100.0;
  proj_matrix = mat4_make_perspective(fov_y, aspect_y, z_near, z_far);

  // initialize frustum planes
  init_frustum_planes(fov_x, fov_y, z_near, z_far);

  // load vertex and face values from OBJ file for mesh data structure
  load_obj_file_data("../assets/efa.obj");

  // load texture information from PNG file
  load_png_texture_data("../assets/efa.png");
}

void process_input(void)
{
  SDL_Event event;
  while (SDL_PollEvent(&event))
  {
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
      case SDLK_X:
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
      case SDLK_5:
        render_method = RENDER_TEXTURED;
        break;
      case SDLK_6:
        render_method = RENDER_TEXTURED_WIRE;
        break;
      case SDLK_UP:
        camera.position.y += 3.0 * delta_time;
        break;
      case SDLK_DOWN:
        camera.position.y -= 3.0 * delta_time;
        break;
      case SDLK_W:
        camera.forward_velocity = vec3_mul(camera.direction, 5.0 * delta_time);
        camera.position = vec3_add(camera.position, camera.forward_velocity);
        break;
      case SDLK_S:
        camera.forward_velocity = vec3_mul(camera.direction, 5.0 * delta_time);
        camera.position = vec3_sub(camera.position, camera.forward_velocity);
        break;
      case SDLK_A:
        camera.yaw_angle += 1.0 * delta_time;
        break;
      case SDLK_D:
        camera.yaw_angle -= 1.0 * delta_time;
        break;
      }
      break; // break swich key down event
    }
  }
};

void update(void)
{
  uint32_t time_to_wait = FRAME_TARGET_TIME - (SDL_GetTicks() - previous_frame_time);
  if (time_to_wait > 0 && time_to_wait <= FRAME_TARGET_TIME)
  {
    SDL_Delay(time_to_wait);
  }

  delta_time = (SDL_GetTicks() - previous_frame_time) / 1000.0;

  previous_frame_time = SDL_GetTicks();

  num_triangles_to_render = 0;

  // change mesh rotation / scale / translation per frame
  // mesh.rotation.x += 0.5 * delta_time;
  // mesh.rotation.y += 0.5 * delta_time;
  // mesh.rotation.z += 0.5 * delta_time;
  // mesh.scale.x += 0.002;
  // mesh.translation.x += 0.01;
  mesh.translation.z = 5; // move away object from camera

  //////////////////////
  // view matrix
  //////////////////////
  // initialize the target looking at the positive z-axis
  vec3_t camera_target = {0, 0, 1};
  mat4_t camera_yaw_rotation = mat4_make_rotation_y(camera.yaw_angle);
  camera.direction = vec3_from_vec4(mat4_mul_vec4(camera_yaw_rotation, vec4_from_vec3(camera_target)));

  // offset the camera position in the direction where the camera is pointing at
  camera_target = vec3_add(camera.position, camera.direction);
  vec3_t camera_up_direction = {0, 1, 0}; // default up direction

  view_matrix = mat4_look_at(camera.position, camera_target, camera_up_direction);

  ///////////////////////////
  // transformation matrix
  ///////////////////////////
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
    face_vertices[0] = mesh.vertices[mesh_face.a];
    face_vertices[1] = mesh.vertices[mesh_face.b];
    face_vertices[2] = mesh.vertices[mesh_face.c];

    // perform transformations
    vec4_t transformed_vertices[3];
    for (int j = 0; j < 3; j++)
    {
      vec4_t transformed_vertex = vec4_from_vec3(face_vertices[j]);

      // world matrix (scale * rotation * translation matrices)
      // order matters: scale - rotation - translation
      world_matrix = mat4_identity();
      world_matrix = mat4_mul_mat4(scale_matrix, world_matrix);
      world_matrix = mat4_mul_mat4(rotation_matrix_z, world_matrix);
      world_matrix = mat4_mul_mat4(rotation_matrix_y, world_matrix);
      world_matrix = mat4_mul_mat4(rotation_matrix_x, world_matrix);
      world_matrix = mat4_mul_mat4(translation_matrix, world_matrix);

      transformed_vertex = mat4_mul_vec4(world_matrix, transformed_vertex);

      // Multiply the view matrix by the vector to transform the scene to camera space
      transformed_vertex = mat4_mul_vec4(view_matrix, transformed_vertex);

      transformed_vertices[j] = transformed_vertex;
    }

    // calculate vectors from triangle vertices
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

    // perform back-face culling
    if (cull_method == CULL_BACKFACE)
    {
      vec3_t origin = {0, 0, 0};
      vec3_t camera_ray = vec3_sub(origin, vector_a);
      float dot_normal_camera = vec3_dot(vector_normal, camera_ray);
      if (dot_normal_camera < 0)
      {
        continue;
      }
    }

    // create polygon from triangle vertices to perform clipping
    polygon_t polygon = polygon_from_triangle(
      vec3_from_vec4(transformed_vertices[0]),
      vec3_from_vec4(transformed_vertices[1]),
      vec3_from_vec4(transformed_vertices[2]),
      mesh_face.a_uv,
      mesh_face.b_uv,
      mesh_face.c_uv
    );

    // clip the polygon against the frustum planes
    clip_polygon(&polygon);

    // break the polygon into triangles after clipping
    triangle_t triangles_after_clipping[MAX_NUM_POLY_TRIANGLES];
    int num_triangles_after_clipping = 0;
    triangles_from_polygon(&polygon, triangles_after_clipping, &num_triangles_after_clipping);

    // loop through each triangle after clipping
    for (int t = 0; t < num_triangles_after_clipping; t++)
    {
      triangle_t triangle_after_clipping = triangles_after_clipping[t];

      // perform projection
      vec4_t projected_points[3];
      for (int j = 0; j < 3; j++)
      {
        projected_points[j] = mat4_mul_vec4_project(proj_matrix, triangle_after_clipping.points[j]);

        // scale into the view
        projected_points[j].x *= (window_width / 2.0);
        projected_points[j].y *= -(window_height / 2.0);

        // translate the projected points into the middle of the screen
        projected_points[j].x += (window_width / 2.0);
        projected_points[j].y += (window_height / 2.0);
      }

      // perform lighting calculation
      float light_intensity = -vec3_dot(vector_normal, light.direction);
      uint32_t triangle_color = light_apply_intensity(mesh_face.color, light_intensity);

      triangle_t triangle_to_render = {
        .points = {
          {projected_points[0].x, projected_points[0].y, projected_points[0].z, projected_points[0].w},
          {projected_points[1].x, projected_points[1].y, projected_points[1].z, projected_points[1].w},
          {projected_points[2].x, projected_points[2].y, projected_points[2].z, projected_points[2].w},
        },
        .texcoords = {
          triangle_after_clipping.texcoords[0],
          triangle_after_clipping.texcoords[1],
          triangle_after_clipping.texcoords[2],
        },
        .color = triangle_color,
      };

      if (num_triangles_to_render < MAX_TRIANGLES)
      {
        triangles_to_render[num_triangles_to_render++] = triangle_to_render;
      }
    }
  }
};

void free_resources(void)
{
  free(color_buffer);
  free(z_buffer);
  upng_free(png_texture);
  array_free(mesh.vertices);
  array_free(mesh.faces);
}

void render(void)
{
  SDL_RenderClear(renderer);
  draw_grid();

  for (int i = 0; i < num_triangles_to_render; i++)
  {
    triangle_t triangle = triangles_to_render[i];

    // draw filled triangle
    if (render_method == RENDER_FILL_TRIANGLE || render_method == RENDER_FILL_TRIANGLE_WIRE)
    {
      draw_filled_triangle(
        triangle.points[0].x, triangle.points[0].y, triangle.points[0].z, triangle.points[0].w, // vertex A
        triangle.points[1].x, triangle.points[1].y, triangle.points[1].z, triangle.points[1].w, // vertex B
        triangle.points[2].x, triangle.points[2].y, triangle.points[2].z, triangle.points[2].w, // vertex C
        triangle.color
      );
    }

    // draw textured triangle
    if (render_method == RENDER_TEXTURED || render_method == RENDER_TEXTURED_WIRE)
    {
      draw_textured_triangle(
        triangle.points[0].x, triangle.points[0].y, triangle.points[0].z, triangle.points[0].w, triangle.texcoords[0].u, triangle.texcoords[0].v, // vertex A
        triangle.points[1].x, triangle.points[1].y, triangle.points[1].z, triangle.points[1].w, triangle.texcoords[1].u, triangle.texcoords[1].v, // vertex B
        triangle.points[2].x, triangle.points[2].y, triangle.points[2].z, triangle.points[2].w, triangle.texcoords[2].u, triangle.texcoords[2].v, // vertex C
        mesh_texture
      );
    }

    // draw wireframe
    if (render_method != RENDER_FILL_TRIANGLE && render_method != RENDER_TEXTURED)
    {
      draw_triangle(
        triangle.points[0].x,
        triangle.points[0].y,
        triangle.points[1].x,
        triangle.points[1].y,
        triangle.points[2].x,
        triangle.points[2].y,
        0xFFFFFFFF
      );
    }

    // draw the vertex
    if (render_method == RENDER_WIRE_VERTEX)
    {
      draw_rect(triangle.points[0].x - 3, triangle.points[0].y - 3, 6, 6, 0xFFFF0000);
      draw_rect(triangle.points[1].x - 3, triangle.points[1].y - 3, 6, 6, 0xFFFF0000);
      draw_rect(triangle.points[2].x - 3, triangle.points[2].y - 3, 6, 6, 0xFFFF0000);
    }
  }

  render_color_buffer();

  clear_color_buffer(0xFF000000);
  clear_z_buffer();

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
