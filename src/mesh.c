#include "mesh.h"
#include "array.h"
#include "triangle.h"
#include <stdio.h>
#include <string.h>

mesh_t mesh = {
  .vertices = NULL,
  .faces = NULL,
  .rotation = {0, 0, 0},
  .scale = {1.0, 1.0, 1.0},
  .translation = {0, 0, 0},
};

vec3_t cube_vertices[N_CUBE_VERTICES] = {
  {-1, -1, -1},
  {-1, 1, -1},
  {1, 1, -1},
  {1, -1, -1},
  {1, 1, 1},
  {1, -1, 1},
  {-1, 1, 1},
  {-1, -1, 1},
};

face_t cube_faces[N_CUBE_FACES] = {
  // front
  {.a = 1, .b = 2, .c = 3, .a_uv = {0, 1}, .b_uv = {0, 0}, .c_uv = {1, 0}, .color = 0xFFFFFFFF},
  {.a = 1, .b = 3, .c = 4, .a_uv = {0, 1}, .b_uv = {1, 0}, .c_uv = {1, 1}, .color = 0xFFFFFFFF},
  // right
  {.a = 4, .b = 3, .c = 5, .a_uv = {0, 1}, .b_uv = {0, 0}, .c_uv = {1, 0}, .color = 0xFFFFFFFF},
  {.a = 4, .b = 5, .c = 6, .a_uv = {0, 1}, .b_uv = {1, 0}, .c_uv = {1, 1}, .color = 0xFFFFFFFF},
  // back
  {.a = 6, .b = 5, .c = 7, .a_uv = {0, 1}, .b_uv = {0, 0}, .c_uv = {1, 0}, .color = 0xFFFFFFFF},
  {.a = 6, .b = 7, .c = 8, .a_uv = {0, 1}, .b_uv = {1, 0}, .c_uv = {1, 1}, .color = 0xFFFFFFFF},
  // left
  {.a = 8, .b = 7, .c = 2, .a_uv = {0, 1}, .b_uv = {0, 0}, .c_uv = {1, 0}, .color = 0xFFFFFFFF},
  {.a = 8, .b = 2, .c = 1, .a_uv = {0, 1}, .b_uv = {1, 0}, .c_uv = {1, 1}, .color = 0xFFFFFFFF},
  // top
  {.a = 2, .b = 7, .c = 5, .a_uv = {0, 1}, .b_uv = {0, 0}, .c_uv = {1, 0}, .color = 0xFFFFFFFF},
  {.a = 2, .b = 5, .c = 3, .a_uv = {0, 1}, .b_uv = {1, 0}, .c_uv = {1, 1}, .color = 0xFFFFFFFF},
  // bottom
  {.a = 6, .b = 8, .c = 1, .a_uv = {0, 1}, .b_uv = {0, 0}, .c_uv = {1, 0}, .color = 0xFFFFFFFF},
  {.a = 6, .b = 1, .c = 4, .a_uv = {0, 1}, .b_uv = {1, 0}, .c_uv = {1, 1}, .color = 0xFFFFFFFF},
};

void load_cube_mesh_data(void)
{
  for (int i = 0; i < N_CUBE_VERTICES; i++)
  {
    vec3_t cube_vertex = cube_vertices[i];
    array_push(mesh.vertices, cube_vertex);
  }

  for (int i = 0; i < N_CUBE_FACES; i++)
  {
    face_t cube_face = cube_faces[i];
    array_push(mesh.faces, cube_face);
  }
}

void load_obj_file_data(char *filename)
{
  FILE *fp = fopen(filename, "r");
  if (fp == NULL)
  {
    perror("Error opening obj file");
    return;
  }

  char buff[512];
  while (fgets(buff, 512, fp))
  {
    // vertex information
    if (buff[0] == 'v' && buff[1] == ' ')
    {
      vec3_t vertex;
      sscanf(buff, "v %f %f %f", &vertex.x, &vertex.y, &vertex.z);
      array_push(mesh.vertices, vertex);
      continue;
    }

    // face information
    if (buff[0] == 'f' && buff[1] == ' ')
    {
      int vertex_indices[3];
      int texture_indices[3];
      int normal_indices[3];
      sscanf(
        buff,
        "f %d/%d/%d %d/%d/%d %d/%d/%d",
        &vertex_indices[0], &texture_indices[0], &normal_indices[0],
        &vertex_indices[1], &texture_indices[1], &normal_indices[1],
        &vertex_indices[2], &texture_indices[2], &normal_indices[2]
      );

      face_t face = {
        vertex_indices[0],
        vertex_indices[1],
        vertex_indices[2],
        .color = 0xFFFFFFFF,
      };
      array_push(mesh.faces, face);
      continue;
    }
  }

  fclose(fp);
}
