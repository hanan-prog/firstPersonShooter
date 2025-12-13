#include "glad/glad.h" //Include order can matter here
#if defined(__APPLE__) || defined(__linux__)
#include <SDL3/SDL.h>
#include <SDL3/SDL_opengl.h>
#else
#include <SDL.h>
#include <SDL_opengl.h>
#endif

#define GLM_FORCE_RADIANS
#define GLM_ENABLE_EXPERIMENTAL
#include <cstdio>
#include <fstream>

#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "glm/gtc/type_ptr.hpp"

#include "models.h"
#include "shader.h"

#define STB_IMAGE_IMPLEMENTATION // only place once in one .cpp file
#include "camera.h"
#include "stb_image.h"

using namespace std;

int screenWidth = 800;
int screenHeight = 600;

bool DEBUG_ON = false;

bool fullscreen = false;

float delta_time = 0.0f, last = 0.0f, current = 0.0f;
float speed = 0.1f;

glm::vec3 pos = glm::vec3(0.0f, 0.f, 4.f);
glm::vec3 fwd_dir = glm::vec3(0.f, 0.f, -1.f);
glm::vec3 up = glm::vec3(0.f, 1.f, 0.f);
// Camera camera;

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
      glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGB, width, height,
                   0, GL_RGB, GL_UNSIGNED_BYTE, data);
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

void drawGeometry(Shader shader, model_t *model1, model_t *model2, GLuint texID,
                  float delta_time) {
  int shaderProg = shader.getShader();

  // modify model matrix based on some user input
  glm::mat4 model = glm::mat4(1.0f);
  model = glm::translate(model, glm::vec3(-1.0f, 1.0f, .0f));
  shader.setUniformMat("model", model);

  // draw first model
  glActiveTexture(GL_TEXTURE0);
  glBindTexture(GL_TEXTURE_CUBE_MAP, texID);
  glDrawArrays(GL_TRIANGLES, model1->start, model1->num_vertices);

  model = glm::mat4(1.0f);
  model = glm::translate(model, glm::vec3(-1.0f, -2.0f, .0f));
   shader.setUniformMat("model", model);
  // draw second model
  glActiveTexture(GL_TEXTURE0);
  glBindTexture(GL_TEXTURE_CUBE_MAP, texID);
  glDrawArrays(GL_TRIANGLES, model2->start, model2->num_vertices);
}

void drawEnviornmentMap(Shader shader, model_t *skyboxModel,
                        GLuint cubemapTex) {
  int shaderProg = shader.getShader();

  // draw skybox
  glActiveTexture(GL_TEXTURE0);
  glBindTexture(GL_TEXTURE_CUBE_MAP, cubemapTex);
  glDrawArrays(GL_TRIANGLES, skyboxModel->start, skyboxModel->num_vertices);
}

int main(int argc, char *argv[]) {
  // sdl initiailzation
  SDL_Init(SDL_INIT_VIDEO);
  // Print the version of SDL we are using (should be 3.x or higher)
  const int sdl_linked = SDL_GetVersion();
  printf("\nCompiled against SDL version %d.%d.%d ...\n",
         SDL_VERSIONNUM_MAJOR(SDL_VERSION), SDL_VERSIONNUM_MINOR(SDL_VERSION),
         SDL_VERSIONNUM_MICRO(SDL_VERSION));
  printf("Linking against SDL version %d.%d.%d.\n",
         SDL_VERSIONNUM_MAJOR(sdl_linked), SDL_VERSIONNUM_MINOR(sdl_linked),
         SDL_VERSIONNUM_MICRO(sdl_linked));

  // Ask SDL to get a recent version of OpenGL (3.2 or greater)
  SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
  SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
  SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 2);

  // Create a window (title, width, height, flags)
  SDL_Window *window = SDL_CreateWindow("My OpenGL Program", screenWidth,
                                        screenHeight, SDL_WINDOW_OPENGL);
  if (!window) {
    printf("SDL_CreateWindow Error: %s\n", SDL_GetError());
    SDL_Quit();
    return 1;
  }

  // Create a context to draw in
  SDL_GLContext context = SDL_GL_CreateContext(window);

  // Load OpenGL extentions with GLAD
  if (!gladLoadGLLoader((GLADloadproc)SDL_GL_GetProcAddress)) {
    printf("ERROR: Failed to initialize OpenGL context.\n");
    return -1;
  }

  // information about OpenGL
  printf("\nOpenGL loaded\n");
  printf("Vendor:   %s\n", glGetString(GL_VENDOR));
  printf("Renderer: %s\n", glGetString(GL_RENDERER));
  printf("Version:  %s\n\n", glGetString(GL_VERSION));

  Models *modelManager = new Models();
  modelManager->load_model("models/skybox.txt", 3);
  modelManager->load_model("models/cube.txt", 8);
  modelManager->load_model("models/plane.txt", 5);
  modelManager->load_model("models/sphere.txt", 8);
  model_list_t *models = modelManager->get_models();

  model_t *skyboxModel = models->root;
  model_t *cubeModel = skyboxModel->next_model;
  model_t *groundModel = cubeModel->next_model;
  model_t *sphereModel = groundModel->next_model;

  /// load shaders
  Shader skyboxShader("shaders/skybox.vs", "shaders/skybox.fs");
  Shader shader("shaders/vertex.vs", "shaders/fragment.fs");
  Shader hudShader("shaders/screen.vs", "shaders/screen.fs");

  // load textures
    GLuint cubeTexture = load_texture("textures/wood.bmp");
    GLuint floorTexture = load_texture("textures/grass.png");

    printf("Cube texture ID: %d\n", cubeTexture);
    printf("Ground texture ID: %d\n", floorTexture);

  // load skybox texture
  vector<string> faces_fnames{
      "textures/skybox/right.jpg", "textures/skybox/left.jpg",
      "textures/skybox/top.jpg",   "textures/skybox/bottom.jpg",
      "textures/skybox/front.jpg", "textures/skybox/back.jpg"};

    // vector<string> faces_fnames{
    // "textures/intersteller/right.tga", "textures/intersteller/left.tga",
    // "textures/intersteller/top.tga",   "textures/intersteller/down.tga",
    // "textures/intersteller/front.tga", "textures/intersteller/back.tga"};

  GLuint cubemapTexture = load_cubemap(faces_fnames);
  printf("Cubemap texture ID: %d\n", cubemapTexture);

  // covers screen in NDC
  float screenVerts[] = {-1.0f, 1.0f, 0.0f, 1.0f,  -1.0f, -1.0f,
                         0.0f,  0.0f, 1.0f, -1.0f, 1.0f,  0.0f,

                         -1.0f, 1.0f, 0.0f, 1.0f,  1.0f,  -1.0f,
                         1.0f,  0.0f, 1.0f, 1.0f,  1.0f,  1.0f};

  GLuint screenVAO, screenVBO;
  glGenVertexArrays(1, &screenVAO);
  glBindVertexArray(screenVAO);
  glGenBuffers(1, &screenVBO);
  glBindBuffer(GL_ARRAY_BUFFER, screenVBO);
  glBufferData(GL_ARRAY_BUFFER, sizeof(screenVerts), &screenVerts,
               GL_STATIC_DRAW);
  // position attribute
  glEnableVertexAttribArray(0);
  glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void *)0);
  // texcoord attribute
  glEnableVertexAttribArray(1);
  glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float),
                        (void *)(2 * sizeof(float)));
  glBindVertexArray(0);

  // cube and floor share shader and attributes
  float *groundData = groundModel->data;
  int groundVerts = groundModel->num_vertices;

  GLuint vao, vbo;
  glGenVertexArrays(1, &vao);
  glBindVertexArray(vao);
  glGenBuffers(1, &vbo);
  glBindBuffer(GL_ARRAY_BUFFER, vbo);
  glBufferData(GL_ARRAY_BUFFER, groundVerts * 5 * sizeof(float), groundData,
               GL_STATIC_DRAW);
  shader.initShaderAttribs5Verts();
  glBindVertexArray(0);

  // sphere and cube
  float *modelData =
      modelManager->combined_model_data(cubeModel, sphereModel, 8);
  int modelVerts = cubeModel->num_vertices + sphereModel->num_vertices;

  GLuint modelVAO, modelVBO;
  glGenVertexArrays(1, &modelVAO);
  glBindVertexArray(modelVAO);
  glGenBuffers(1, &modelVBO);
  glBindBuffer(GL_ARRAY_BUFFER, modelVBO);
  glBufferData(GL_ARRAY_BUFFER, modelVerts * 8 * sizeof(float), modelData,
               GL_STATIC_DRAW);
  shader.initShaderAttribs8Verts();
  glBindVertexArray(0);

  float *skyData = skyboxModel->data;
  int skyVerts = skyboxModel->num_vertices;

  GLuint skyboxVAO, skyboxVBO;
  glGenVertexArrays(1, &skyboxVAO);
  glBindVertexArray(skyboxVAO);
  glGenBuffers(1, &skyboxVBO);
  glBindBuffer(GL_ARRAY_BUFFER, skyboxVBO);
  glBufferData(GL_ARRAY_BUFFER, skyVerts * 3 * sizeof(float), skyData,
               GL_STATIC_DRAW);
  skyboxShader.initShaderAttribs3Verts();
  glBindVertexArray(0);

  // initialize shader
  shader.useShader();
  shader.setTexNum("tex0", 0);

  skyboxShader.useShader();
  skyboxShader.setTexNum("skybox", 0);

  /// frame buffer config
  hudShader.useShader();
  hudShader.setTexNum("hudTexure", 0);

//   GLuint framebuffer;
//   glGenFramebuffers(1, &framebuffer);
//   glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);

//   GLuint colorBuffer;
//   glGenTextures(1, &colorBuffer);
//   glBindTexture(GL_TEXTURE_2D, colorBuffer);
//   glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, screenWidth, screenHeight, 0, GL_RGB,
//                GL_UNSIGNED_BYTE, NULL);
//   glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
//   glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
//   glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D,
//                          colorBuffer, 0);

//   GLuint rbo;
//   glGenRenderbuffers(1, &rbo);
//   glBindRenderbuffer(GL_RENDERBUFFER, rbo);
//   glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, screenWidth,
//                         screenHeight);
//   glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT,
//                             GL_RENDERBUFFER, rbo);

//   if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
//     printf("ERROR::FRAMEBUFFER:: Framebuffer is not complete!\n");
//   glBindFramebuffer(GL_FRAMEBUFFER, 0);

  // Load the new shader

  glEnable(GL_DEPTH_TEST);

  bool quit = false;
  SDL_Event event;
  while (!quit) {

    while (SDL_PollEvent(&event)) {
      // handle user input
      delta_time = SDL_GetTicks() / 1000.0f;
      if (event.type == SDL_EVENT_QUIT) {
        quit = true;
      }

      if (event.type == SDL_EVENT_KEY_UP) {
        if (event.key.key == SDLK_ESCAPE) {
          quit = true;
        }

        if (event.key.key == SDLK_F) {
          fullscreen = !fullscreen;
          SDL_SetWindowFullscreen(window,
                                  fullscreen ? SDL_WINDOW_FULLSCREEN : 0);
          SDL_GetWindowSize(window, &screenWidth, &screenHeight);

          glViewport(0, 0, screenWidth, screenHeight);
        }
      }

      if (event.type == SDL_EVENT_KEY_DOWN) {
        float velocity = PLAYER_SPEED * delta_time;
        if (event.key.key == SDLK_W) {
          pos += fwd_dir * velocity;
        }

        if (event.key.key == SDLK_S) {
          pos -= fwd_dir * velocity;
        }

        if (event.key.key == SDLK_A) {

          pos -= glm::normalize(glm::cross(fwd_dir, up)) * velocity;
        }

        if (event.key.key == SDLK_D) {
          pos += glm::normalize(glm::cross(fwd_dir, up)) * velocity;
        }
      }

      //   if (event.type == SDL_EVENT_KEY_DOWN) {
      //     float velocity = PLAYER_SPEED * delta_time;
      //     if (event.key.key == SDLK_W) {
      //         camera.pos += camera.front * velocity;
      //     }

      //     if (event.key.key == SDLK_S) {
      //         camera.pos -= camera.front * velocity;
      //     }

      //     if (event.key.key == SDLK_A) {
      //         camera.pos -= camera.getCameraRight() * velocity;
      //     }

      //     if (event.key.key == SDLK_D) {
      //         camera.pos += camera.getCameraRight() * velocity;
      //     }

      //   }

      //   if (event.type == SDL_EVENT_MOUSE_MOTION) {
      //         // Grab the specific motionEvent from the general SDL_Event
      //         structure SDL_MouseMotionEvent motionEvent = event.motion;
      //         camera.updateCameraRotation(motionEvent.xrel,
      //         motionEvent.yrel);
      //     }
    }

    // render
    // glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);
    // glEnable(GL_DEPTH_TEST); // disabled for screen quad

    glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    shader.useShader();



    glm::mat4 view = glm::lookAt(pos, pos + fwd_dir, up);
    glm::mat4 proj = glm::perspective(glm::radians(45.0f),
                                      (float)screenWidth /
                                      (float)screenHeight, 0.1f, 100.0f);

        shader.setUniformMat("view", view);
    shader.setUniformMat("proj", proj);
    // glm::vec3 overheadPos = pos + glm::vec3(0.0f, 20.0f, 0.0f);
    // glm::mat4 orthoView =
    //     glm::lookAt(overheadPos, pos, glm::vec3(0.0f, 0.0f, -1.0f));
    // shader.setUniformMat("view", orthoView);

     glm::mat4 model = glm::mat4(1.0f);
     shader.setUniformMat("model", model);

    glBindVertexArray(vao);
     glActiveTexture(GL_TEXTURE0);
  glBindTexture(GL_TEXTURE_2D, floorTexture);
  glDrawArrays(GL_TRIANGLES, groundModel->start, groundModel->num_vertices);
    // drawGeometry(shader, cubeModel, groundModel, cubemapTexture, delta_time);
    glBindVertexArray(0);

    // render other models in the scene
    glBindVertexArray(modelVAO);
    drawGeometry(shader, cubeModel, sphereModel, cubemapTexture, delta_time);
    glBindVertexArray(0);

    // draw main screen
    // glBindFramebuffer(GL_FRAMEBUFFER, 0);        
    // glViewport(0, 0, screenWidth, screenHeight);
    // glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
    // glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // shader.useShader();

    // glm::mat4 proj = glm::perspective(glm::radians(45.0f),
    //                                   (float)screenWidth / (float)screenHeight,
    //                                   0.1f, 100.0f);
    // shader.setUniformMat("proj", proj);
    // glm::mat4 view = glm::lookAt(pos, pos + fwd_dir, up);
    // shader.setUniformMat("view", view);

    // glBindVertexArray(vao);
    // drawGeometry(shader, cubeModel, groundModel, cubemapTexture, delta_time);
    // glBindVertexArray(0);

    // glBindVertexArray(modelVAO);
    // drawGeometry(shader, cubeModel, sphereModel, cubemapTexture, delta_time);
    // glBindVertexArray(0);

    glDepthFunc(GL_LEQUAL);
    skyboxShader.useShader();
    skyboxShader.setUniformMat("view", glm::mat4(glm::mat3(view)));
    skyboxShader.setUniformMat("proj", proj);
    glBindVertexArray(skyboxVAO);
    drawEnviornmentMap(skyboxShader, skyboxModel, cubemapTexture);
    glBindVertexArray(0);
    glDepthFunc(GL_LESS);

    // glDisable(GL_DEPTH_TEST);
    // hudShader.useShader();

    // glBindVertexArray(screenVAO);
    // glActiveTexture(GL_TEXTURE0);
    // glBindTexture(GL_TEXTURE_2D, colorBuffer);
    // glDrawArrays(GL_TRIANGLES, 0, 6);
    // glBindVertexArray(0);

    // glEnable(GL_DEPTH_TEST);

    SDL_GL_SwapWindow(window);

    // // draw to screen
    // glBindFramebuffer(GL_FRAMEBUFFER, 0);
    // glDisable(GL_DEPTH_TEST); // disabled for screen quad
    // glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
    // glClear(GL_COLOR_BUFFER_BIT);
    // hudShader.useShader();
    // glBindVertexArray(screenVAO);
    // glBindTexture(GL_TEXTURE_2D, colorBuffer);
    // glDrawArrays(GL_TRIANGLES, 0, 6);
    // glBindVertexArray(0);

    // draw skybox as last
    // glDepthFunc(GL_LEQUAL); // change depth function so depth test passes
    // // when values are equal to depth buffer's content
    // skyboxShader.useShader();
    // // remove translation from the view matrix

    // // so that if the player moves, the skybox still looks all encompssing
    // skyboxShader.setUniformMat("view", glm::mat4(glm::mat3(view)));
    // skyboxShader.setUniformMat("proj", proj);

    // glBindVertexArray(skyboxVAO);
    // drawEnviornmentMap(skyboxShader, skyboxModel, cubemapTexture);
    // glBindVertexArray(0);
    // glDepthFunc(GL_LESS); // set depth function back to default

    // SDL_GL_SwapWindow(window); // Double buffering
  }

  // clean up
  shader.cleanUpShader();
  skyboxShader.cleanUpShader();

  glDeleteVertexArrays(1, &vao);
  SDL_GL_DestroyContext(context);
  SDL_Quit();

  return 0;
}
