#ifndef GAME_TYPES_H
#define GAME_TYPES_H

#include "glad/glad.h"
#include "glm/glm.hpp"
#include <vector>

typedef enum  {
    FORWARD,
    BACKWARD,
    LEFT,
    RIGHT
}movement_t;
// node
typedef struct model_t {
    const char* name;
    int start;
    int num_vertices;
    float* data;
    model_t* next_model;
} model_t;

// linked list of models
typedef struct model_list_t {
    int len;
    int total_vertices;
    model_t* root;
} model_list_t;

typedef struct transform_t {
    glm::vec3 translation ;
    glm::vec3 rotation;
    glm::vec3 scale;
    float angle;
} transform_t;


typedef struct camera_t {
    // left to right and up and down angles
    float pitch, yaw;
    glm::vec3 pos, fwd_dir, up, right; // camera position, direction, right vectors
    glm::mat4 transformation; 
    float fov, aspect_ratio, near, far;
} camera_t;

typedef enum entity_types { DOOR, WALL, KEY, START, GOAL, GROUND, NONE } entity_types_t;
// typedef enum state_types { INVALID, VALID, WON } state_types_t;

typedef struct entity_t {
    entity_types_t type;
    transform_t transform; // different transformations
    glm::mat4 model; //  model matrix  
    glm::vec3 material; // color
    GLuint textID = -1;  // no texture by default
    model_t *geometry;
    char key_id; // for doors and keys
} entity_t;

typedef struct game_map_t {
    // all entities 
    // state_types_t state;
    camera_t global_cam;
   
    entity_t held_key_entity;
    entity_t floor_entity;
    int map_width, map_height;
    glm::vec3 start_pos;
    std::vector<entity_t> entities;
} game_map_t; 



#endif // GAME_TYPES_H