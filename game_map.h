#ifndef GAME_MAP_H
#define GAME_MAP_H

// #include "glad/glad.h"
#include "entity.h"
#include "game_types.h"
#include "glm/glm.hpp"
#include "shader.h"
#include <cstdio>
#include <fstream>

#include "glm/gtc/matrix_transform.hpp"
#include "glm/gtc/type_ptr.hpp"

using namespace std;

typedef enum state_types { INVALID, VALID, WON } state_types_t;

class GameMap {

public:
GameMap() = default;
    void init_map(const char *scene_file);

  ~GameMap();

  void draw(Shader shaderProgram, camera_t &cam, float delta_time);
  void set_cube_map_texture(vector<string> faces_fnames);
  GLuint get_cube_map_texture();

  float *get_model_data();
  int get_total_vertices();

  glm::vec3 get_start_pos();
  glm::vec3 get_goal_pos();
  state_types_t process_move(glm::vec3 new_pos);

  void debug(int shaderProgram, camera_t &cam, float delta_time);


  GLuint load_floor_model() {
     float planeVertices[] = {

         5.0f, -0.5f,  5.0f,  2.0f, 0.0f,
        -5.0f, -0.5f,  5.0f,  0.0f, 0.0f,
        -5.0f, -0.5f, -5.0f,  0.0f, 2.0f,

         5.0f, -0.5f,  5.0f,  2.0f, 0.0f,
        -5.0f, -0.5f, -5.0f,  0.0f, 2.0f,
         5.0f, -0.5f, -5.0f,  2.0f, 2.0f								
    };
    
    GLuint planeVAO, planeVBO;
    glGenVertexArrays(1, &planeVAO);
    glGenBuffers(1, &planeVBO);
    glBindVertexArray(planeVAO);
    glBindBuffer(GL_ARRAY_BUFFER, planeVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(planeVertices), &planeVertices, GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
    glBindVertexArray(0);
    return planeVAO;
  }

  void draw_floor(Shader shader, GLuint floorVao) {
    GLuint floorTex = load_texture("textures/metal.png");
    shader.setUniformMat("model", glm::mat4(1.0f));
    glBindVertexArray(floorVao);
    glBindTexture(GL_TEXTURE_2D, floorTex);
    shader.setUniformMat("model", glm::mat4(1.0f));
    glDrawArrays(GL_TRIANGLES, 0, 6);
    glBindVertexArray(0);
  }
  // void pick_up_key(glm::vec3 pos);

private:
  Entity floor, key_held;
    GLuint floorVao_, floorTex_;
  GLuint cubeMapTexID_;

  model_list_t *models_;
  glm::vec3 start_pos_;
  glm::vec3 goal_pos_;

  Entity *entities;
  int w, h;
  


  

  // utility funciton to load a texture from a file
  // both load_texture() and load_cubemap() were adapted from:
  // https://github.com/JoeyDeVries/LearnOpenGL/blob/master/src/4.advanced_opengl/6.1.cubemaps_skybox/cubemaps_skybox.cpp
  GLuint load_texture(const char *fname) {
    GLuint texID;
    glGenTextures(1, &texID);

    int width, height, nrComponents;
    unsigned char *data = stbi_load(fname, &width, &height, &nrComponents, 0);
    if (data) {
      GLenum format;
      if (nrComponents == 1)
        format = GL_RED;
      else if (nrComponents == 3)
        format = GL_RGB;
      else if (nrComponents == 4)
        format = GL_RGBA;

      glBindTexture(GL_TEXTURE_2D, texID);
      glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format,
                   GL_UNSIGNED_BYTE, data);
      glGenerateMipmap(GL_TEXTURE_2D);

      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER,
                      GL_LINEAR_MIPMAP_LINEAR);
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

      stbi_image_free(data);
      return texID;
    }

    printf("Texture failed to load at path: %s\n", fname);
    stbi_image_free(data);
    return texID;
  }

  // loads a cubemap texture from 6 individual texture faces
  // order:
  // +X (right)
  // -X (left)
  // +Y (top)
  // -Y (bottom)
  // +Z (front)
  // -Z (back)
  GLuint load_cubemap(vector<string> faces_fnames) {
    printf("Loading cubemap textures...\n");
    GLuint texID;
    glGenTextures(1, &texID);
    glBindTexture(GL_TEXTURE_CUBE_MAP, texID);

    int width, height, nrChannels;
    for (GLuint i = 0; i < faces_fnames.size(); i++) {
      unsigned char *data =
          stbi_load(faces_fnames[i].c_str(), &width, &height, &nrChannels, 0);
      if (data) {
        glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGB, width,
                     height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
        stbi_image_free(data);
      } else {
        printf("Cubemap texture failed to load at path: %s\n",
               faces_fnames[i].c_str());
        stbi_image_free(data);
      }
    }

    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
    return texID;
  }

  void get_coord(glm::vec3 pos, int &x, int &z) {
    x = static_cast<int>(floorf(pos.x + w / 2.f));
    z = static_cast<int>(h + floorf(pos.z));
  }

  model_list_t *init_model_list() {
    model_list_t *res = (model_list_t *)malloc(sizeof(model_list_t));
    res->len = 0;
    res->total_vertices = 0;
    res->root = nullptr;

    return res;
  }

  void clear_models(model_list_t *model_list) {
    model_t *current = model_list->root;
    while (current != nullptr) {
      model_t *next = current->next_model;
      delete[] current->data;
      free(current);
      current = next;
    }
    model_list->root = nullptr;
    model_list->len = 0;
    model_list->total_vertices = 0;
    free(model_list);
  }

  // / input model_files : [models / model1.txt, models / model2.txt, ...]
  // // purpose: for every model we populate model_t and return a vector of all
  // model_t
  // // return 0 on succes, 1 if error occured
  int add_model(const char *fname, model_list_t *model_list) {
    if (model_list == nullptr) {
      printf("model list is null");
      return 1;
    }

    ifstream modelFile;
    modelFile.open(fname);
    if (!modelFile.is_open()) {
      // problem opening
      printf("can't open file\n");
      return 1;
    }

    model_t *new_model = (model_t *)malloc(sizeof(model_t));
    if (new_model == NULL) {
      printf("can't allocate memory ");
      modelFile.close();
      return 1;
    }

    int num_lines = 0;
    modelFile >> num_lines;
    new_model->data = new float[num_lines];
    for (int i = 0; i < num_lines; i++) {
      modelFile >> new_model->data[i];
    }

    new_model->name = fname;
    new_model->num_vertices = num_lines / 8;
    new_model->start = model_list->total_vertices;
    new_model->next_model = nullptr;
    model_list->total_vertices += new_model->num_vertices;
    model_list->len++;

    if (model_list->root == nullptr) {
      // empty list new model will be head of that list
      model_list->root = new_model;
    } else {
      // search for the next empty place in the list and have new model point to
      // it
      model_t *prev = model_list->root;
      while (prev->next_model != nullptr) {
        prev = prev->next_model;
      }
      prev->next_model = new_model;
      new_model->next_model = nullptr;
    }

    modelFile.close();
    return 0;
  }

  // combines all model data into one buffer and returns that buffer
  float *combine_models(model_list_t *models) {
    int modelDataSize = models->total_vertices * 8;
    float *modelData = new float[modelDataSize];

    model_t *curr_model = models->root;
    int offset = 0;
    while (curr_model != nullptr) {

      // copy over data
      float *curr_size = curr_model->data + curr_model->num_vertices * 8;
      copy(curr_model->data, curr_size, modelData + offset);
      offset += curr_model->num_vertices * 8;
      curr_model = curr_model->next_model;
    }

    return modelData;
  }

  void draw_all_models(int shaderProgram, model_list_t *models,
                       float timePast) {
    GLint uniColor = glGetUniformLocation(shaderProgram, "inColor");
    GLint uniTexID = glGetUniformLocation(shaderProgram, "texID");
    GLint uniModel = glGetUniformLocation(shaderProgram, "model");

    // just draw all models in white at origin

    model_t *curr_model = models->root;

    printf("drawing model %s\n", curr_model->name);
    // glm::vec3 colVec(colR, colG, colB);

    // glUniform3fv(uniColor, 1, glm::value_ptr(colVec));

    glm::mat4 model = glm::mat4(1);
    model = glm::rotate(model, timePast * glm::radians(90.0f),
                        glm::vec3(0.0f, 1.0f, 1.0f));
    model = glm::rotate(model, timePast * glm::radians(45.0f),
                        glm::vec3(1.0f, 0.0f, 0.0f));

    glUniformMatrix4fv(uniModel, 1, GL_FALSE,
                       glm::value_ptr(model)); // pass model matrix to shader

    // Set which texture to use (-1 = no texture)
    glUniform1i(uniTexID, -1);

    // Draw an instance of the model (at the position & orientation specified by
    // the model matrix above)
    glDrawArrays(
        GL_TRIANGLES, curr_model->start,
        curr_model
            ->num_vertices); //(Primitive Type, Start Vertex, Num Vertices)

    ////// next model drawing code //////
    curr_model = curr_model->next_model;
    printf("drawing model %s\n", curr_model->name);

    model = glm::mat4(1);
    model = glm::translate(model, glm::vec3(-2, -1, -.4));
    glUniformMatrix4fv(uniModel, 1, GL_FALSE, glm::value_ptr(model));

    // Set which texture to use (0 = wood texture ... bound to GL_TEXTURE0)
    glUniform1i(uniTexID, 0);

    // Draw an instance of the model (at the position & orientation specified by
    // the model matrix above)
    glDrawArrays(
        GL_TRIANGLES, curr_model->start,
        curr_model
            ->num_vertices); //(Primitive Type, Start Vertex, Num Vertices)

    //// 3 rd model ////
    curr_model = curr_model->next_model;
    printf("drawing model %s\n", curr_model->name);
    model = glm::mat4(1);
    model = glm::translate(model, glm::vec3(2, 1, .4));

    glUniform1i(uniTexID, 0); // wood texture
    glUniformMatrix4fv(uniModel, 1, GL_FALSE, glm::value_ptr(model));
    glDrawArrays(GL_TRIANGLES, curr_model->start, curr_model->num_vertices);
  }
};

#endif // GAME_MAP_H