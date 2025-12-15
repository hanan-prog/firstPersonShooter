#include "game_map.h"


#include <cstdio>
#include <fstream>
using namespace std;

void GameMap::init_map(const char* fname) {
    std::ifstream mapFile;
    mapFile.open(fname);
    if (!mapFile.is_open()) {
        printf("Error: could not open map file %s\n", fname);
        exit(1);
    }

    mapFile >> w >> h;

    // I should probably use a more memory safe way but because i know we have data in models this is fine
    // cube((walls/doors))->knot(keys)->teapot(goal)->sphere(start)

    // load models
    models_ = init_model_list();

    // should probably error check but cant be bothered
    add_model("models/cube.txt", models_); // walls and doors 
    add_model("models/knot.txt", models_); // key
    add_model("models/teapot.txt", models_); // goal
    add_model("models/sphere.txt", models_); // start 

    printf("loaded %d models with total %d vertices\n", models_->len, models_->total_vertices);

    model_t* cube = models_->root;
    model_t* knot = cube->next_model;
    model_t* teapot = knot->next_model;
    model_t* sphere = teapot->next_model;

    entities = new Entity[w * h]; // max possible entities
    
    transform_t floor_transform{};
    floor_transform.translation = glm::vec3(0, -0.5f, -h / 2.f);
    floor_transform.rotation = glm::vec3(0.0f, 1.0f, 0.0f);
    floor_transform.scale = glm::vec3(w, 1, h);
    floor.init_ground(floor_transform, cube);
    char ch;
    for (int i = 0; i < h; i++) {
        for (int j = 0; j < w; j++) {
            mapFile >> ch;
            int idx = i * w + j;
            glm::vec3 start_pos = glm::vec3(j - w / 2.0f + 0.5f, 0.5f, i + 0.5f -h);
            if (ch == 'W') {
               transform_t wall_transform{};
                wall_transform.translation = start_pos;
                wall_transform.scale = glm::vec3(1.0f, 1.0f, 1.0f);
                wall_transform.rotation = glm::vec3(0.0f, 1.0f, 0.0f);
                wall_transform.angle = 0.0f;
                entities[idx].init_wall(wall_transform, cube);
            } else if (ch == 'S') {
                start_pos_ = start_pos;
                start_pos_.y = 0.65f;
            } else if (ch == 'G') {
                transform_t goal_transform{};
                goal_transform.translation = start_pos;
                goal_pos_ = start_pos;
                goal_transform.scale = glm::vec3(1.0f, 1.0f, 1.0f);
                goal_transform.rotation = glm::vec3(0.0f, 1.0f, 0.0f);
                goal_transform.angle = 0.0f;
                entities[idx].init_goal(goal_transform, sphere);
            } 
            
            // else if(ch >= 97 && ch <= 101) { 
            //     // key 
            //     transform_t key_transform{};
            //     key_transform.translation = start_pos;
            //     key_transform.scale = glm::vec3(0.2f, 0.2f, 0.2f);
            //     key_transform.rotation = glm::vec3(0.0f, 1.0f, 0.0f);
            //     key_transform.angle = 0.0f;
            //     entities[idx].init_key(key_transform, knot, ch);
            // } 
            
            // else if (ch >= 65 && ch <= 69) { 
            //     // door 
            //     transform_t door_transform{};
            //     door_transform.translation = start_pos;
            //     door_transform.scale = glm::vec3(1.0f, 1.0f, 1.0f);
            //     door_transform.rotation = glm::vec3(0.0f, 1.0f, 0.0f);
            //     door_transform.angle = 0.0f;
            //     entities[idx].init_door(door_transform, cube, tolower(ch));
            // } 
        }
    }
    key_held = Entity(); // initialize to none
    mapFile.close();
}

void GameMap::set_cube_map_texture(vector<string> faces_fnames) {
    cubeMapTexID_ = load_cubemap(faces_fnames);
}

GLuint GameMap::get_cube_map_texture() {
    return cubeMapTexID_;
}


float* GameMap::get_model_data() {
    // combine models into one buffer and get the result 
    float* modelData = combine_models(models_);
    return modelData;
}

int GameMap::get_total_vertices() {
    return models_->total_vertices;
}

glm::vec3 GameMap::get_start_pos() {
    return start_pos_;
}

glm::vec3 GameMap::get_goal_pos() {
    return goal_pos_;
}

GameMap::~GameMap() {
    clear_models(models_);
    delete[] entities;
}

void GameMap::draw(Shader shaderProgram, camera_t& cam, float delta_time) {
    // draw floor
    // cam.pos = glm::vec3(0, 1, 3);
    // cam.fwd_dir = glm::vec3(0, 0, -1);
    floor.draw(shaderProgram, cam);
    // draw_floor(shaderProgram, floorVao_, floorTex_);
    for (int i = 0; i < h; i++) {
        for (int j = 0; j < w; j++) {
            int idx = i * w + j;
            if (entities[idx].get_type() != GROUND && entities[idx].get_type() != NONE) {
                if (entities[idx].get_type() == GOAL) {
                    // rotate goal
                    // printf("rotating goal\n");
                    entities[idx].set_angle(-delta_time * 3.14f);
                } 
                
                
                // else if (entities[idx].get_type() == KEY) {
                //     // rotate key
                //     // printf("rotating key\n");
                //     entities[idx].set_angle(delta_time * 3.14f / 2);
                // } else if (entities[idx].get_type() == DOOR) {
                //     // maybe animate door opening later
                //     if ((idx - w >= 0 && entities[idx - w].get_type() == WALL) || (idx + w < h * w && entities[idx + w].get_type() == WALL)) {
                //         entities[idx].set_angle((float)M_PI /2.f);
                //         entities[idx].set_rotation(glm::vec3(0.f, 1.f, 0.f));
                //     }
                // }
                entities[idx].draw(shaderProgram, cam);
            }
        }
    }

    // // if key is being held 
    // if (key_held.get_type() == KEY) {
       

        
    //     glm::vec3 held_pos = cam.pos + cam.fwd_dir * glm::vec3(0.4) + glm::vec3(0.f, -0.05f, 0.f);
    //     key_held.set_translation(held_pos);
        
        
    //     key_held.set_scale(glm::vec3(0.3f, 0.3f, 0.3f));
    //     key_held.set_angle(delta_time * 3.14f);
    //     // float ang = -acosf(glm::dot(glm::normalize(cam.fwd_dir),
    //     //     glm::vec3(-1.f, 0.f, 0.f)));

    //     // // account for negative angles
    //     // if (glm::dot(glm::vec3(0, 1, 0), glm::cross(glm::normalize(cam.fwd_dir),glm::vec3(-1.f, 0.f, 0.f))) < 0.f) {
    //     //     ang = -ang;
    //     // }
    //     // const float speed = 1.5;
    //     // float ang = speed * delta_time;
    //     // if ()
    //     // key_held.set_angle(ang);
    //     key_held.set_rotation(glm::vec3(0.f, 1.f, 0.f));
    //     key_held.draw(shaderProgram, cam);
    // }
}

// void GameMap::pick_up_key(glm::vec3 pos) {
//     int x, z;
//     get_coord(pos, x, z);
//     printf("picking up key at coord %d %d\n", x, z);
//     printf("entity type at coord: %d\n", entities[z * w + x].get_type());
//     if (entities[z * w + x].get_type() == KEY) {
//         transform_t key_transform;
//         model_t* knot = models_->root->next_model; // second model is knot, Terrible way to do this but im lazy
//         if( key_held.get_type() == NONE) {
//             key_held.init_key(key_transform, knot, entities[z * w + x].get_key_id());
//             entities[z * w + x].set_type(NONE); // remove key from map
//             printf("Picked up key %c\n", key_held.get_key_id());
//         } else {
//             // update held key
//             char curr_key = key_held.get_key_id();
//             char cell_key = entities[z * w + x].get_key_id();
//             key_held.set_key_id(cell_key);
//             entities[z * w + x].set_key_id(curr_key);
//             printf("Swapped key %c with key %c\n", curr_key, cell_key);
//         }
//     }
// }


state_types_t GameMap::process_move(glm::vec3 new_pos) {
    int x, z;
    get_coord(new_pos, x, z);
    printf("processing move to coord %d %d\n", x, z);

    if (x < 0 || x >= w || z < 0 || z >= h) {
        return INVALID;
    }
    if (entities[z * w + x].get_type() == WALL) {
        return INVALID;
    } 
    
    // else if (entities[z * w + x].get_type() == DOOR) {
    //     // check if we have the key
    //     if (entities[z * w + x].get_key_id() == key_held.get_key_id()) {
    //         // open the door
    //         printf("Opened door with key %c\n", key_held.get_key_id());
    //         key_held.set_type(NONE); // drop the key
    //         entities[z * w + x].set_type(NONE); // remove the door
    //         return VALID;
    //     }
    //     return INVALID;
    // } 
    
    else if (entities[z * w + x].get_type() == GOAL){
        return WON;
    }
    
    return VALID;
}

void GameMap::debug(int shaderProgram, camera_t& cam, float delta_time) {
    GLint uniView = glGetUniformLocation(shaderProgram, "view");
    GLint uniProj = glGetUniformLocation(shaderProgram, "proj");

    glm::mat4 view = glm::lookAt(
        glm::vec3(3.f, 0.f, 0.f),  //Cam Position
        glm::vec3(0.0f, 0.0f, 0.0f),  //Look at point
        glm::vec3(0.0f, 0.0f, 1.0f)); //Up
    glUniformMatrix4fv(uniView, 1, GL_FALSE, glm::value_ptr(view));

    glm::mat4 proj = glm::perspective(glm::radians(45.0f),cam.aspect_ratio, 0.1f, 10.0f); //FOV, aspect, near, far
    glUniformMatrix4fv(uniProj, 1, GL_FALSE, glm::value_ptr(proj));
    draw_all_models(shaderProgram, models_, delta_time);
}