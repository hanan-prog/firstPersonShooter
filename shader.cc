#include "shader.h"

Shader::Shader(const char *vShaderFileName, const char *fShaderFileName, const char *gShaderFileName) {
  shaderProgram_ = InitShader(vShaderFileName, fShaderFileName);
}


void Shader::useShader() { glUseProgram(shaderProgram_); }

// 
void Shader::initShaderAttribs8Verts() {
  	//Tell OpenGL how to set fragment shader input 
	GLint posAttrib = glGetAttribLocation(shaderProgram_, "position");
	glVertexAttribPointer(posAttrib, 3, GL_FLOAT, GL_FALSE, 8*sizeof(float), 0);
	  //Attribute, vals/attrib., type, isNormalized, stride, offset
	glEnableVertexAttribArray(posAttrib);
	
	GLint normAttrib = glGetAttribLocation(shaderProgram_, "inNormal");
	glVertexAttribPointer(normAttrib, 3, GL_FLOAT, GL_FALSE, 8*sizeof(float), (void*)(5*sizeof(float)));
	glEnableVertexAttribArray(normAttrib);
	
	GLint texAttrib = glGetAttribLocation(shaderProgram_, "inTexcoord");
	glEnableVertexAttribArray(texAttrib);
	glVertexAttribPointer(texAttrib, 2, GL_FLOAT, GL_FALSE, 8*sizeof(float), (void*)(3*sizeof(float)));
}

void Shader::initShaderAttribs3Verts() {
  GLint skyPosAttrib = glGetAttribLocation(shaderProgram_, "position");
  glVertexAttribPointer(skyPosAttrib, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float),
                        (void *)0);
  glEnableVertexAttribArray(skyPosAttrib);
}

void Shader::initShaderAttribs5Verts() {
  GLint posAttrib = glGetAttribLocation(shaderProgram_, "position");
  glVertexAttribPointer(posAttrib, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float),
                        (void *)0);
  glEnableVertexAttribArray(posAttrib);

  GLint texAttrib = glGetAttribLocation(shaderProgram_, "inTexcoord");
  glEnableVertexAttribArray(texAttrib);
  glVertexAttribPointer(texAttrib, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float),
                        (void *)(3 * sizeof(float)));
}



GLuint Shader::InitShader(const char *vShaderFileName,
                          const char *fShaderFileName, const char *gShaderFileName) {
  GLuint vertex_shader, fragment_shader;
  GLchar *vs_text, *fs_text;
  GLuint program;

  // check GLSL version
  printf("GLSL version: %s\n\n", glGetString(GL_SHADING_LANGUAGE_VERSION));

  // Create shader handlers
  vertex_shader = glCreateShader(GL_VERTEX_SHADER);
  fragment_shader = glCreateShader(GL_FRAGMENT_SHADER);

  // Read source code from shader files
  vs_text = readShaderSource(vShaderFileName);
  fs_text = readShaderSource(fShaderFileName);
  // if geometry shader filename is present, read it too
  
  GLuint geometry_shader;
  if (gShaderFileName != nullptr) {
    geometry_shader = glCreateShader(GL_GEOMETRY_SHADER);
    GLchar *gs_text = readShaderSource(gShaderFileName);

    // error check
    if (gs_text == NULL) {
      printf("Failed to read from geometry shader file %s\n", gShaderFileName);
      exit(1);
    } else if (DEBUG_ON) {
      printf("\nGeometry Shader:\n=====================\n");
      printf("%s\n", gs_text);
      printf("=====================\n\n");
    }

    // compile geometry shader and add to program
    const char *gg = gs_text;
    glShaderSource(geometry_shader, 1, &gg, NULL);
    glCompileShader(geometry_shader);


    GLint compiled;
    glGetShaderiv(geometry_shader, GL_COMPILE_STATUS, &compiled);
    if (!compiled) {
      printf("Geometry shader failed to compile\n");
      if (DEBUG_ON) {
        GLint logMaxSize, logLength;
        glGetShaderiv(geometry_shader, GL_INFO_LOG_LENGTH, &logMaxSize);
        printf("printing error message of %d bytes\n", logMaxSize);
        char *logMsg = new char[logMaxSize];
        glGetShaderInfoLog(geometry_shader, logMaxSize, &logLength, logMsg);
        printf("%d bytes retrieved\n", logLength);
        printf("error message: %s\n", logMsg);
        delete[] logMsg;
      }
      exit(1);
    }
  }



  // error check
  if (vs_text == NULL) {
    printf("Failed to read from vertex shader file %s\n", vShaderFileName);
    exit(1);
  } else if (DEBUG_ON) {
    printf("Vertex Shader:\n=====================\n");
    printf("%s\n", vs_text);
    printf("=====================\n\n");
  }
  if (fs_text == NULL) {
    printf("Failed to read from fragent shader file %s\n", fShaderFileName);
    exit(1);
  } else if (DEBUG_ON) {
    printf("\nFragment Shader:\n=====================\n");
    printf("%s\n", fs_text);
    printf("=====================\n\n");
  }

  // Load Vertex Shader
  const char *vv = vs_text;
  glShaderSource(vertex_shader, 1, &vv, NULL); // Read source
  glCompileShader(vertex_shader);              // Compile shaders

  // Check for errors
  GLint compiled;
  glGetShaderiv(vertex_shader, GL_COMPILE_STATUS, &compiled);
  if (!compiled) {
    printf("Vertex shader failed to compile:\n");
    if (DEBUG_ON) {
      GLint logMaxSize, logLength;
      glGetShaderiv(vertex_shader, GL_INFO_LOG_LENGTH, &logMaxSize);
      printf("printing error message of %d bytes\n", logMaxSize);
      char *logMsg = new char[logMaxSize];
      glGetShaderInfoLog(vertex_shader, logMaxSize, &logLength, logMsg);
      printf("%d bytes retrieved\n", logLength);
      printf("error message: %s\n", logMsg);
      delete[] logMsg;
    }
    exit(1);
  }

  // Load Fragment Shader
  const char *ff = fs_text;
  glShaderSource(fragment_shader, 1, &ff, NULL);
  glCompileShader(fragment_shader);
  glGetShaderiv(fragment_shader, GL_COMPILE_STATUS, &compiled);

  // Check for Errors
  if (!compiled) {
    printf("Fragment shader failed to compile\n");
    if (DEBUG_ON) {
      GLint logMaxSize, logLength;
      glGetShaderiv(fragment_shader, GL_INFO_LOG_LENGTH, &logMaxSize);
      printf("printing error message of %d bytes\n", logMaxSize);
      char *logMsg = new char[logMaxSize];
      glGetShaderInfoLog(fragment_shader, logMaxSize, &logLength, logMsg);
      printf("%d bytes retrieved\n", logLength);
      printf("error message: %s\n", logMsg);
      delete[] logMsg;
    }
    exit(1);
  }

  // Create the program
  program = glCreateProgram();

  // Attach shaders to program
  glAttachShader(program, vertex_shader);
  glAttachShader(program, fragment_shader);

  // if geometry shader is present, attach it too
  if (gShaderFileName != nullptr) {
    glAttachShader(program, geometry_shader);
  }

  // Link and set program to use
  glLinkProgram(program);

  return program;
}

void Shader::cleanUpShader() {
  glDeleteProgram(shaderProgram_);
}