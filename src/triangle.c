#include "triangle.h"
#include "array.h"
#include "display.h"
#include "swap.h"
#include "texture.h"
#include "vector.h"
#include <stdint.h>
#include <stdlib.h>

void fill_flat_bottom_triangle(int x0, int y0, int x1, int y1, int x2, int y2, uint32_t color)
{
  float inv_slope_1 = (x1 - x0) / (float)(y1 - y0);
  float inv_slope_2 = (x2 - x0) / (float)(y2 - y0);

  // start from top
  float x_start = x0;
  float x_end = x0;

  for (int y = y0; y <= y2; y++)
  {
    draw_line(x_start, y, x_end, y, color);

    x_start += inv_slope_1;
    x_end += inv_slope_2;
  }
}

void fill_flat_top_triangle(int x0, int y0, int x1, int y1, int x2, int y2, uint32_t color)
{
  float inv_slope_1 = (x2 - x0) / (float)(y2 - y0);
  float inv_slope_2 = (x2 - x1) / (float)(y2 - y1);

  // start from bottom
  float x_start = x2;
  float x_end = x2;

  for (int y = y2; y >= y0; y--)
  {
    draw_line(x_start, y, x_end, y, color);

    x_start -= inv_slope_1;
    x_end -= inv_slope_2;
  }
}

void draw_filled_triangle(int x0, int y0, int x1, int y1, int x2, int y2, uint32_t color)
{
  // sort vertices by y-coordinate (y0 < y1 < y3)
  if (y0 > y1)
  {
    int_swap(&y0, &y1);
    int_swap(&x0, &x1);
  }
  if (y1 > y2)
  {
    int_swap(&y1, &y2);
    int_swap(&x1, &x2);
  }
  if (y0 > y1)
  {
    int_swap(&y0, &y1);
    int_swap(&x0, &x1);
  }

  if (y1 == y2)
  {
    fill_flat_bottom_triangle(x0, y0, x1, y1, x2, y2, color);
  }
  else if (y0 == y1)
  {
    fill_flat_top_triangle(x0, y0, x1, y1, x2, y2, color);
  }
  else
  {
    // create new vertex to split triangle in 2
    int My = y1;
    int Mx = ((float)((x2 - x0) * (y1 - y0)) / (float)(y2 - y0) + x0);

    fill_flat_bottom_triangle(x0, y0, x1, y1, Mx, My, color);
    fill_flat_top_triangle(x1, y1, Mx, My, x2, y2, color);
  }
}

void sort_triangles(triangle_t *triangles)
{
  int num_triangles = array_length(triangles);
  for (int i = 0; i < num_triangles; i++)
  {
    for (int j = num_triangles; j > i; j--)
    {
      if (triangles[i].avg_depth < triangles[j].avg_depth)
      {
        triangle_t tmp = triangles[i];
        triangles[i] = triangles[j];
        triangles[j] = tmp;
      }
    }
  }
}

vec3_t barycentric_weights(vec2_t a, vec2_t b, vec2_t c, vec2_t p)
{
  vec2_t ac = vec2_sub(c, a);
  vec2_t ab = vec2_sub(b, a);
  vec2_t ap = vec2_sub(p, a);
  vec2_t pc = vec2_sub(c, p);
  vec2_t pb = vec2_sub(b, p);

  // calculate area of the full parallelogram/triangle ABC using 2D cross product
  float area_paralellogram_abc = (ac.x * ab.y) - (ac.y * ab.x); // || AC * AB ||

  // alpha is area of parallelogram/triangle PBC divided by area of ABC
  float alpha = ((pc.x * pb.y) - (pc.y * pb.x)) / area_paralellogram_abc;

  // beta is area of parallelogram/triangle APC divided by area of ABC
  float beta = ((ac.x * ap.y) - (ac.y * ap.x)) / area_paralellogram_abc;

  float gamma = 1 - alpha - beta;

  vec3_t weights = {alpha, beta, gamma};
  return weights;
}

void draw_texel(
  int x, int y, uint32_t *texture,
  vec4_t point_a, vec4_t point_b, vec4_t point_c,
  tex2_t a_uv, tex2_t b_uv, tex2_t c_uv
)
{
  vec2_t point_p = {x, y};
  vec3_t weights = barycentric_weights(vec2_from_vec4(point_a), vec2_from_vec4(point_b), vec2_from_vec4(point_c), point_p);

  float alpha = weights.x;
  float beta = weights.y;
  float gamma = weights.z;

  // interpolate values of U, V and also 1/w for the current pixel
  float interpolated_u;
  float interpolated_v;
  float interpolated_reciprocal_w;

  // perform interpolation of all U/w and V/w values using barycentric weights and factor of 1/w
  interpolated_u = (a_uv.u / point_a.w * alpha) + (b_uv.u / point_b.w * beta) + (c_uv.u / point_c.w * gamma);
  interpolated_v = (a_uv.v / point_a.w * alpha) + (b_uv.v / point_b.w * beta) + (c_uv.v / point_c.w * gamma);

  // interpolate value of 1/w for the current pixel
  interpolated_reciprocal_w = (1 / point_a.w * alpha) + (1 / point_b.w * beta) + (1 / point_c.w * gamma);

  // divide back both of interpolated values by 1/w
  interpolated_u /= interpolated_reciprocal_w;
  interpolated_v /= interpolated_reciprocal_w;

  int tex_x = abs((int)(interpolated_u * texture_width)) % texture_width;
  int tex_y = abs((int)(interpolated_v * texture_height)) % texture_height;

  // adjust 1/w so the pixels that are closer to the camera have smaller values
  interpolated_reciprocal_w = 1.0 - interpolated_reciprocal_w;

  // only draw pixel if the depth value is less than the one previously stored in the z-buffer
  if (interpolated_reciprocal_w < z_buffer[(window_width * y) + x])
  {
    draw_pixel(x, y, texture[(texture_width * tex_y) + tex_x]);

    // update z-buffer value with 1/w of this current pixel
    z_buffer[(window_width * y) + x] = interpolated_reciprocal_w;
  }
}

void draw_textured_triangle(
  int x0, int y0, float z0, float w0, float u0, float v0, // vertex A
  int x1, int y1, float z1, float w1, float u1, float v1, // vertex A
  int x2, int y2, float z2, float w2, float u2, float v2, // vertex A
  uint32_t *texture
)
{
  // sort vertices by y-coordinate (y0 < y1 < y3)
  if (y0 > y1)
  {
    int_swap(&y0, &y1);
    int_swap(&x0, &x1);
    float_swap(&z0, &z1);
    float_swap(&w0, &w1);
    float_swap(&u0, &u1);
    float_swap(&v0, &v1);
  }
  if (y1 > y2)
  {
    int_swap(&y1, &y2);
    int_swap(&x1, &x2);
    float_swap(&z1, &z2);
    float_swap(&w1, &w2);
    float_swap(&u1, &u2);
    float_swap(&v1, &v2);
  }
  if (y0 > y1)
  {
    int_swap(&y0, &y1);
    int_swap(&x0, &x1);
    float_swap(&z0, &z1);
    float_swap(&w0, &w1);
    float_swap(&u0, &u1);
    float_swap(&v0, &v1);
  }

  // flip V component for inverted UV-coordinates
  v0 = 1.0 - v0;
  v1 = 1.0 - v1;
  v2 = 1.0 - v2;

  // vector points and texture coordinates
  vec4_t point_a = {x0, y0, z0, w0};
  vec4_t point_b = {x1, y1, z1, w1};
  vec4_t point_c = {x2, y2, z2, w2};
  tex2_t a_uv = {u0, v0};
  tex2_t b_uv = {u1, v1};
  tex2_t c_uv = {u2, v2};

  ///////////////////////////////////////////////////////////////////////
  // draw flat-bottom triangle
  ///////////////////////////////////////////////////////////////////////
  float inv_slope_1 = 0;
  float inv_slope_2 = 0;

  if (y1 - y0 != 0)
    inv_slope_1 = (float)(x1 - x0) / abs(y1 - y0);
  if (y2 - y0 != 0)
    inv_slope_2 = (float)(x2 - x0) / abs(y2 - y0);

  // check if flat-bottom (y1-y0 != 0)
  if (y1 - y0 != 0)
  {
    for (int y = y0; y <= y1; y++)
    {
      int x_left = x1 + (y - y1) * inv_slope_1;
      int x_right = x0 + (y - y0) * inv_slope_2;

      if (x_left > x_right)
        int_swap(&x_left, &x_right);

      for (int x = x_left; x < x_right; x++)
      {
        draw_texel(x, y, texture, point_a, point_b, point_c, a_uv, b_uv, c_uv);
      }
    }
  }

  ///////////////////////////////////////////////////////////////////////
  // draw flat-top triangle
  ///////////////////////////////////////////////////////////////////////
  inv_slope_1 = 0;
  inv_slope_2 = 0;

  if (y2 - y1 != 0)
    inv_slope_1 = (float)(x2 - x1) / abs(y2 - y1);
  if (y2 - y0 != 0)
    inv_slope_2 = (float)(x2 - x0) / abs(y2 - y0);

  if (y2 - y1 != 0)
  {
    for (int y = y1; y <= y2; y++)
    {
      int x_left = x1 + (y - y1) * inv_slope_1;
      int x_right = x0 + (y - y0) * inv_slope_2;

      if (x_left > x_right)
        int_swap(&x_left, &x_right);

      for (int x = x_left; x < x_right; x++)
      {
        draw_texel(x, y, texture, point_a, point_b, point_c, a_uv, b_uv, c_uv);
      }
    }
  }
}
