
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

// #include "models.h"
#include "game_map.h"
#include "game_types.h"
#include "shader.h"

#define STB_IMAGE_IMPLEMENTATION // only place once in one .cpp file
#include "stb_image.h"

using namespace std;

int screenWidth = 800;
int screenHeight = 600;

bool save_output = true;
bool DEBUG_ON = false;

bool fullscreen = false;

float current_time = 0.0f;
float last_time = 0.0f;
float delta_time = 0.0f;

void Win2PPM(int width, int height);

camera_t init_camera(glm::vec3 start_pos, glm::vec3 fwd_dir) {
  camera_t cam;
  cam.aspect_ratio = screenWidth / (float)screenHeight;
  cam.fov = glm::radians(45.0f);
  cam.near = 0.01f;
  cam.far = 10.0f;

  cam.pos = start_pos;
  // cam.pos.y += 1.0f; // raise camera height
  cam.fwd_dir = fwd_dir; // forward direction
  cam.up = glm::vec3(0, 1, 0);
  return cam;
}
void turn_camera(camera_t &cam, float angle) {
  glm::mat4 rotation =
      glm::rotate(glm::mat4(1.0f), angle, glm::vec3(0.0f, 1.0f, 0.0f));
  cam.fwd_dir = glm::vec3(rotation * glm::vec4(cam.fwd_dir, 0.0f));
}

glm::vec3 get_camera_pos(camera_t &cam, float dis) {
  glm::vec3 forward = cam.fwd_dir;
  glm::mat4 translation = glm::translate(glm::mat4(1.0f), forward * dis);
  return glm::vec3(translation * glm::vec4(cam.pos, 1.0f));
}
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

void drawGeometry(Shader shader, model_t *model1, model_t *model2,
                  GLuint texID) {
  int shaderProg = shader.getShader();

  // modify model matrix based on some user input
  glm::mat4 model = glm::mat4(1.0f);
  shader.setUniformMat("model", model);

  // draw first model
  glActiveTexture(GL_TEXTURE0);
  glBindTexture(GL_TEXTURE_CUBE_MAP, texID);
  glDrawArrays(GL_TRIANGLES, model1->start, model1->num_vertices);
  // draw second model
  glActiveTexture(GL_TEXTURE0);
  glBindTexture(GL_TEXTURE_CUBE_MAP, texID);
  glDrawArrays(GL_TRIANGLES, model2->start, model2->num_vertices);
}

void drawEnviornmentMap(Shader shader, int skyboxModelStart,
                        int skyboxModelNumVerts, GLuint cubemapTex) {
  int shaderProg = shader.getShader();

  // draw skybox
  glActiveTexture(GL_TEXTURE0);
  glBindTexture(GL_TEXTURE_CUBE_MAP, cubemapTex);
  glDrawArrays(GL_TRIANGLES, skyboxModelStart, skyboxModelNumVerts);
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

  //   /// load shaders
  Shader skyboxShader("shaders/skybox.vs", "shaders/skybox.fs");
  Shader shader("shaders/vertex.vs", "shaders/fragment.fs");

  //   load skybox texture
  vector<string> faces_fnames{
      "textures/skybox/right.jpg", "textures/skybox/left.jpg",
      "textures/skybox/top.jpg",   "textures/skybox/bottom.jpg",
      "textures/skybox/front.jpg", "textures/skybox/back.jpg"};

  //   vector<string> faces_fnames{
  //       "textures/intersteller/right.tga", "textures/intersteller/left.tga",
  //       "textures/intersteller/top.tga",   "textures/intersteller/down.tga",
  //       "textures/intersteller/front.tga", "textures/intersteller/back.tga"};

  GLuint cubemapTexture = load_cubemap(faces_fnames);
  printf("Cubemap texture ID: %d\n", cubemapTexture);

  // load game map
  GameMap *game_map = new GameMap("scenes/map1.txt");
  float *modelData = game_map->get_model_data();
  int total_verts = game_map->get_total_vertices();

  GLuint vao;
  glGenVertexArrays(1, &vao); // Create a VAO
  glBindVertexArray(vao); // Bind the above created VAO to the current context
  GLuint vbo[1];
  glGenBuffers(1, vbo); // Create 1 buffer called vbo
  glBindBuffer(GL_ARRAY_BUFFER,
               vbo[0]); // Set the vbo as the active array buffer (Only one
                        // buffer can be active at a time)
  glBufferData(GL_ARRAY_BUFFER, total_verts * 8 * sizeof(float), modelData,
               GL_STATIC_DRAW); // upload vertices to vbo

  shader.initShaderAttribs8Verts();
  glBindVertexArray(0);

  // load skybox model
  const char *fname = "models/skybox.txt";
  ifstream modelFile;
  modelFile.open(fname);
  if (!modelFile.is_open()) {
    // problem opening
    printf("can't open file\n");
    return 1;
  }

  int num_lines = 0;
  modelFile >> num_lines;
  float *skyData = new float[num_lines];
  for (int i = 0; i < num_lines; i++) {
    modelFile >> skyData[i];
  }

  int skyVerts = num_lines / 3;

  GLuint skyboxVAO, skyboxVBO;
  glGenVertexArrays(1, &skyboxVAO);
  glBindVertexArray(skyboxVAO);
  glGenBuffers(1, &skyboxVBO);
  glBindBuffer(GL_ARRAY_BUFFER, skyboxVBO);
  glBufferData(GL_ARRAY_BUFFER, skyVerts * 3 * sizeof(float), skyData,
               GL_STATIC_DRAW);
  skyboxShader.initShaderAttribs3Verts();
  glBindVertexArray(0);

  // initialize camera
  camera_t global_cam =
      init_camera(game_map->get_start_pos(), glm::vec3(0, 0, -1));
  printf("initial camera position %.2f %.2f %.2f\n", global_cam.pos.x,
         global_cam.pos.y, global_cam.pos.z);

         // get skybox view and projection locations
    GLint skyboxViewLoc =
        glGetUniformLocation(skyboxShader.getShader(), "view");
    GLint skyboxProjLoc =
        glGetUniformLocation(skyboxShader.getShader(), "proj");

  glEnable(GL_DEPTH_TEST);

  bool quit = false;
  bool pick_up = false;

  float move, turn_angle, speed = 0.03f;

  SDL_Event event;
  while (!quit) {

    move = 0.0f;
    turn_angle = 0.0f;

    while (SDL_PollEvent(&event)) {
      // handle user input

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
        }
      }

      if (event.type == SDL_EVENT_KEY_DOWN) {
        if (event.key.key == SDLK_UP) {
          // move forward
          // global_cam.moveZ += cam_move_speed;
          // printf("forward works move\n");

          move = 1.f;
        }

        if (event.key.key == SDLK_DOWN) {
          // backwards

          move = -1.f;
        }

        if (event.key.key == SDLK_LEFT) {
          // left

          turn_angle = speed;
        }

        if (event.key.key == SDLK_RIGHT) {
          // right
          turn_angle = -speed;
        }
      }
    }

    if (turn_angle) {
      turn_camera(global_cam, turn_angle);
    }

    if (move) {
      glm::vec3 new_pos = get_camera_pos(global_cam, move * speed);

      // add offset to avoid collison
      glm::vec3 offset_pos = get_camera_pos(global_cam, move * (speed + 0.06f));
      if (game_map->process_move(offset_pos) == VALID) {

        global_cam.pos = new_pos;
        printf("moving to new position %.2f %.2f %.2f\n", global_cam.pos.x,
               global_cam.pos.y, new_pos.z);
      } else if (game_map->process_move(offset_pos) == WON) {
        printf("You reached the goal! You won!\n");
        quit = true;
      } else {
        printf("move invalid due to collision\n");
      }
    }

    last_time = current_time;
    current_time = SDL_GetTicks() / 1000.0f; // convert to seconds
    delta_time = current_time - last_time;

    glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    shader.useShader();
    shader.setTexNum("skybox", 0);
    glBindTexture(GL_TEXTURE_CUBE_MAP, cubemapTexture);

    glBindVertexArray(vao);
    game_map->draw(shader, global_cam, delta_time);
   



    // glm::mat4 view = camera.get_view_matrix();
    // glm::mat4 proj = camera.get_projection_matrix();
    // shader.setUniformMat("view", view);
    // shader.setUniformMat("proj", proj);

    // glm::mat4 model = glm::mat4(1.0f);
    // shader.setUniformMat("model", model);
    // shader.setTexNum("texID", 0);
    // glActiveTexture(GL_TEXTURE0);
    // glBindTexture(GL_TEXTURE_2D, floorTexture);

    // glBindVertexArray(groundVao);
    // glDrawArrays(GL_TRIANGLES, 0, groundModel->num_vertices);
    // glBindVertexArray(0);

    // cube
    // model = glm::mat4(1.0f);
    // model = glm::translate(model, glm::vec3(2.0f, 0.5f, -1.0f));
    // model = glm::rotate(model, delta_time * glm::radians(90.0f),
    //                     glm::vec3(0.0f, 1.0f, 1.0f));
    // model = glm::rotate(model, delta_time * glm::radians(45.0f),
    //                     glm::vec3(1.0f, 0.0f, 0.0f));

    // shader.setUniformMat("model", model);

    // shader.setTexNum("texID", -1);
    // glm::vec3 cubeColor(0.7f, 0.2f, 0.3f);
    // shader.setUniformColor("inColor", cubeColor);

    // glBindVertexArray(cubeVao);
    // glDrawArrays(GL_TRIANGLES, 0, cubeModel->num_vertices);
    // glBindVertexArray(0);

    // draw sphere
    // glBindVertexArray(sphereVao);
    // glm::mat4 model = glm::mat4(1.0f);
    // model = glm::translate(model, glm::vec3(0.f, 1.5f, 0.0f));
    // shader.setUniformMat("model", model);
    // // glm::vec3 sphereColor(0.2f, 0.3f, 0.8f);
    // // shader.setUniformColor("inColor", sphereColor);
    // // shader.setTexNum("texID", -1);
    // glActiveTexture(GL_TEXTURE0);
    // glBindTexture(GL_TEXTURE_CUBE_MAP, cubemapTexture);
    // glDrawArrays(GL_TRIANGLES, 0, sphereModel->num_vertices);
    glBindVertexArray(0);

    // draw skybox as last
    glDepthFunc(GL_LEQUAL);

    skyboxShader.useShader();
    skyboxShader.setTexNum("skybox", 0);

    glm::mat4 view = glm::lookAt(global_cam.pos, global_cam.pos + global_cam.fwd_dir, global_cam.up);
    glm::mat4 proj = glm::perspective(global_cam.fov, global_cam.aspect_ratio, global_cam.near, global_cam.far);

    // so that if the player moves, the skybox still looks all encompssing
    skyboxShader.setUniformMat("view", glm::mat4(glm::mat3(view)));
    skyboxShader.setUniformMat("proj", proj);

    glBindVertexArray(skyboxVAO);
    drawEnviornmentMap(skyboxShader, 0, skyVerts, cubemapTexture);
    glBindVertexArray(0);
    glDepthFunc(GL_LESS); // set depth function back to default

    if (save_output) {
      Win2PPM(screenWidth, screenHeight);
      // save_output = false;
    }

    SDL_GL_SwapWindow(window); // Double buffering
  }

  // clean up
  shader.cleanUpShader();
  skyboxShader.cleanUpShader();
//   glDeleteVertexArrays(1, &cubeVao);
//   glDeleteVertexArrays(1, &groundVao);
//   glDeleteVertexArrays(1, &skyboxVAO);
  SDL_GL_DestroyContext(context);
  SDL_Quit();

  return 0;
}

void Win2PPM(int width, int height) {
  char outdir[10] = "out/"; // Must be defined!
  int i, j;
  FILE *fptr;
  static int counter = 0;
  char fname[32];
  unsigned char *image;

  /* Allocate our buffer for the image */
  image = (unsigned char *)malloc(3 * width * height * sizeof(char));
  if (image == NULL) {
    fprintf(stderr, "ERROR: Failed to allocate memory for image\n");
  }

  /* Open the file */
  snprintf(fname, sizeof(fname), "%simage_%04d.ppm", outdir, counter);
  if ((fptr = fopen(fname, "w")) == NULL) {
    fprintf(stderr, "ERROR: Failed to open file for window capture\n");
  }

  /* Copy the image into our buffer */
  glReadBuffer(GL_BACK);
  glReadPixels(0, 0, width, height, GL_RGB, GL_UNSIGNED_BYTE, image);

  /* Write the PPM file */
  fprintf(fptr, "P6\n%d %d\n255\n", width, height);
  for (j = height - 1; j >= 0; j--) {
    for (i = 0; i < width; i++) {
      fputc(image[3 * j * width + 3 * i + 0], fptr);
      fputc(image[3 * j * width + 3 * i + 1], fptr);
      fputc(image[3 * j * width + 3 * i + 2], fptr);
    }
  }

  free(image);
  fclose(fptr);
  counter++;
}