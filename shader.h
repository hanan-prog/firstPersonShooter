#ifndef SHADER_H
#define SHADER_H

#include "glad/glad.h"
#include "glm/glm.hpp"
#include <SDL3/SDL_opengl.h>
#include <cstdio>
#include <fstream>

class Shader {
public:
  Shader() = default;
  Shader(const char *vShaderFileName, const char *fShaderFileName, const char *gShaderFileName = nullptr);
  ~Shader() = default;
  GLuint getShader() const { return shaderProgram_; }
  void useShader();

  // shader attributes for 8 float per vertex (3 pos, 3 normal, 2 texcoord)
  void initShaderAttribs8Verts();
  void initShaderAttribs3Verts();
  void initShaderAttribs5Verts();


  void cleanUpShader();

  void setTexNum(const std::string &name, int value) const {
    glUniform1i(glGetUniformLocation(shaderProgram_, name.c_str()), value);
  }

  void setUniformMat(const std::string &name, const glm::mat4 &mat) const {
    glUniformMatrix4fv(glGetUniformLocation(shaderProgram_, name.c_str()), 1,
                       GL_FALSE, &mat[0][0]);
  }

private:
  GLuint shaderProgram_;
  bool DEBUG_ON = false;
  GLuint InitShader(const char *vShaderFileName, const char *fShaderFileName, const char *gShaderFileName = nullptr);
  // Create a NULL-terminated string by reading the provided file
  static char *readShaderSource(const char *shaderFile) {
    FILE *fp;
    long length;
    char *buffer;

    // open the file containing the text of the shader code
    fp = fopen(shaderFile, "r");

    // check for errors in opening the file
    if (fp == NULL) {
      printf("can't open shader source file %s\n", shaderFile);
      return NULL;
    }

    // determine the file size
    fseek(fp, 0, SEEK_END); // move position indicator to the end of the file;
    length = ftell(fp);     // return the value of the current position

    // allocate a buffer with the indicated number of bytes, plus one
    buffer = new char[length + 1];

    // read the appropriate number of bytes from the file
    fseek(fp, 0, SEEK_SET); // move position indicator to the start of the file
    fread(buffer, 1, length, fp); // read all of the bytes

    // append a NULL character to indicate the end of the string
    buffer[length] = '\0';

    // close the file
    fclose(fp);

    // return the string
    return buffer;
  }
};

#endif // SHADER_H