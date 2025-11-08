#include "mesh.h"
#include "array.h"
#include "texture.h"
#include "triangle.h"
#include "upng.h"
#include <stdio.h>
#include <string.h>

#define MAX_NUM_MESHES 10
static mesh_t meshes[MAX_NUM_MESHES];
static int mesh_count = 0;

void load_mesh(char *obj_filename, char *png_filename, vec3_t scale, vec3_t translation, vec3_t rotation)
{
  load_mesh_obj_data(&meshes[mesh_count], obj_filename);
  load_mesh_png_data(&meshes[mesh_count], png_filename);

  meshes[mesh_count].scale = scale;
  meshes[mesh_count].translation = translation;
  meshes[mesh_count].rotation = rotation;

  mesh_count++;
}

void load_mesh_obj_data(mesh_t *mesh, char *obj_filename)
{
  FILE *fp = fopen(obj_filename, "r");
  if (fp == NULL)
  {
    perror("Error opening obj file");
    return;
  }

  tex2_t *texcoords = NULL;

  char buff[512];
  while (fgets(buff, 512, fp))
  {
    // vertex information
    if (buff[0] == 'v' && buff[1] == ' ')
    {
      vec3_t vertex;
      sscanf(buff, "v %f %f %f", &vertex.x, &vertex.y, &vertex.z);
      array_push(mesh->vertices, vertex);
      continue;
    }

    // texture coordinate information
    if (strncmp(buff, "vt", 2) == 0)
    {
      tex2_t texcoord;
      sscanf(buff, "vt %f %f", &texcoord.u, &texcoord.v);
      array_push(texcoords, texcoord);
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
        .a = vertex_indices[0] - 1,
        .b = vertex_indices[1] - 1,
        .c = vertex_indices[2] - 1,
        .a_uv = texcoords[texture_indices[0] - 1],
        .b_uv = texcoords[texture_indices[1] - 1],
        .c_uv = texcoords[texture_indices[2] - 1],
        .color = 0xFFFFFFFF,
      };
      array_push(mesh->faces, face);
      continue;
    }
  }

  array_free(texcoords);
  fclose(fp);
}

void load_mesh_png_data(mesh_t *mesh, char *png_filename)
{
  upng_t *png_image = upng_new_from_file(png_filename);
  if (png_image != NULL)
  {
    upng_decode(png_image);
    if (upng_get_error(png_image) == UPNG_EOK)
    {
      mesh->texture = png_image;
    }
  }
}

int get_num_meshes(void)
{
  return mesh_count;
}

mesh_t *get_mesh(int index)
{
  return &meshes[index];
}

void free_meshes(void)
{
  for (int i = 0; i < mesh_count; i++)
  {
    upng_free(meshes[i].texture);
    array_free(meshes[i].faces);
    array_free(meshes[i].vertices);
  }
}
