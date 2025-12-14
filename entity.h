#ifndef ENTITY_H
#define ENTITY_H

#include "game_types.h"
#include "shader.h"
#include "glm/glm.hpp"

class Entity {
    public:
        Entity();
        ~Entity();

        void init_ground(transform_t transform, model_t* geometry);
        void init_wall(transform_t transform, model_t* geometry);
        // void init_door(transform_t transform, model_t* geometry, char key_id);
        // void init_key(transform_t transform, model_t* geometry, char key_id);
        void init_goal(transform_t transform, model_t* geometry);
        void draw(Shader shaderProgram, camera_t& cam);

        entity_types_t get_type();
        void set_type(entity_types_t type);
        glm::mat4 get_model_matrix();
        glm::mat4 get_view_matrix(camera_t& cam);
        glm::mat4 get_proj_matrix(camera_t& cam);
        void set_angle(float angle);
        void set_translation(glm::vec3 translation);
        void set_scale(glm::vec3 scale);
        void set_rotation(glm::vec3 rotation);

        // char get_key_id();
        // void set_key_id(char key_id);

    private:
        transform_t transform_;
        glm::vec3 material_; // color
        GLuint textID_ = -1;  // no texture by default
        model_t *geometry_ = nullptr;
        entity_types_t type_;
        char key_id_; // for doors and keys

        glm::vec3 color_picker(char c) {
            switch (tolower(c)) {
            case 's':
                return glm::vec3(0.5, 0.5, 0.5);
            case 'g':
                return glm::vec3(0.0, 1.0, 0.0);
            case 'w':
                return glm::vec3(1.0, 0.0, 0.0);
            case 'a':
                return glm::vec3(1.0, 1.0, 0.0); // yellow
            case 'b':
                return glm::vec3(0.0, 1.0, 1.0); // cyan
            case 'c':
                return glm::vec3(1.0, 0.0, 1.0); // magenta
            case 'd':
                return glm::vec3(0.0, 0.5, 0.0); // dark green
            case 'e':
                return glm::vec3(0.0, 0.0, 0.5); // dark blue
            default:
                return glm::vec3(1.0, 1.0, 1.0); // white
            }
        }
   
};

#endif // ENTITY_H