#ifndef TRIANGLE_H
#define TRIANGLE_H

#include "texture.h"
#include "vector.h"
#include <stdint.h>

typedef struct
{
  int a;
  int b;
  int c;
  tex2_t a_uv;
  tex2_t b_uv;
  tex2_t c_uv;
  uint32_t color;
} face_t;

typedef struct
{
  vec4_t points[3];
  tex2_t texcoords[3];
  uint32_t color;
  float avg_depth;
} triangle_t;

vec3_t barycentric_weights(vec2_t a, vec2_t b, vec2_t c, vec2_t p);

void draw_triangle_pixel(
  int x, int y, uint32_t color,
  vec4_t point_a, vec4_t point_b, vec4_t point_c
);

void draw_filled_triangle(
  int x0, int y0, float z0, float w0, // Vertex A
  int x1, int y1, float z1, float w1, // Vertex B
  int x2, int y2, float z2, float w2, // Vertex C
  uint32_t color
);

void draw_texel(
  int x, int y, uint32_t *texture,
  vec4_t point_a, vec4_t point_b, vec4_t point_c,
  tex2_t a_uv, tex2_t b_uv, tex2_t c_uv
);

void draw_textured_triangle(
  int x0, int y0, float z0, float w0, float u0, float v0, // vertex A
  int x1, int y1, float z1, float w1, float u1, float v1, // vertex A
  int x2, int y2, float z2, float w2, float u2, float v2, // vertex A
  uint32_t *texture
);

#endif // !TRIANGLE_H
