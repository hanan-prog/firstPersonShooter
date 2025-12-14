#include "entity.h"

#include "glm/gtc/matrix_transform.hpp"
#include "glm/gtc/type_ptr.hpp"

Entity::Entity() {
    // default constructor
    printf("constructing entity\n");
    type_ = NONE;
    transform_.translation = glm::vec3(0.0f);
    transform_.rotation = glm::vec3(0.0f, 1.0f, 0.0f); // default rotation axis
    transform_.scale = glm::vec3(1.0f);             // no scaling
    transform_.angle = 0.0f;
}
Entity::~Entity() {
    // destructor
}

void Entity::init_ground(transform_t transform, model_t* geometry) {
    printf("initializing ground entity\n");
    // initialize ground entity
    transform_ = transform;
    // material_ = color_picker('');
    geometry_ = geometry;
    textID_ = 0; // default texture for ground
    type_ = GROUND;
}
void Entity::init_wall(transform_t transform, model_t* geometry) {
    printf("initializing wall entity\n");
    // initialize wall entity
    transform_ = transform;
    // material_ = color;
    textID_ = 1; // default texture for wall
    geometry_ = geometry;
    type_ = WALL;   
}

void Entity::init_goal(transform_t transform, model_t* geometry) {
    printf("initializing goal entity\n");
    // initialize goal entity
    transform_ = transform;
    material_ = color_picker('g');
    // textID_ = -1; // no texture for goal
    geometry_ = geometry;
    type_ = GOAL;
}

void Entity::draw(Shader shaderProgram, camera_t& cam) {
    // up
    if (geometry_ == nullptr) {
        printf("No geometry to draw for this entity\n");
        return;
    }

    // printf("drawing geometry %s\n", geometry_->name);
    // glm::mat4 model = glm::mat4(1.0f);


    // GLint uniModel = glGetUniformLocation(shaderProgram, "model");
    // GLint uniTexID = glGetUniformLocation(shaderProgram, "texID");
    // GLint uniView = glGetUniformLocation(shaderProgram, "view");
    // GLint uniProj = glGetUniformLocation(shaderProgram, "proj");
    // GLint64 uniColor = glGetUniformLocation(shaderProgram, "inColor");
    // glUniform3fv(uniColor, 1, glm::value_ptr(material_)); //pass color to shader

    glm::mat4 model = get_model_matrix();
    glm::mat4 view = get_view_matrix(cam);
    glm::mat4 proj = get_proj_matrix(cam);
    shaderProgram.setUniformMat("model", model);
    shaderProgram.setUniformMat("view", view);
    shaderProgram.setUniformMat("proj", proj);
    // glUniformMatrix4fv(uniModel, 1, GL_FALSE, glm::value_ptr(model)); //pass model matrix to shader
    // glUniformMatrix4fv(uniView, 1, GL_FALSE, glm::value_ptr(view)); //pass view matrix to shader
    // glUniformMatrix4fv(uniProj, 1, GL_FALSE, glm::value_ptr(proj)); //pass projection matrix to shader
    // glUniform1i(uniTexID, textID_);
    glDrawArrays(GL_TRIANGLES, geometry_->start, geometry_->num_vertices);
}

void Entity::set_angle(float angle) {
    transform_.angle = angle;
}

void Entity::set_translation(glm::vec3 translation) {
    transform_.translation = translation;
}
void Entity::set_scale(glm::vec3 scale) {
    transform_.scale = scale;
}
void Entity::set_rotation(glm::vec3 rotation) {
    transform_.rotation = rotation;
}

entity_types_t Entity::get_type() {
    return type_;
}

// char Entity::get_key_id() {
//     return key_id_;
// }
// void Entity::set_key_id(char key_id) {
//     key_id_ = key_id;
// }

void Entity::set_type(entity_types_t type) {
    type_ = type;
}

glm::mat4 Entity::get_model_matrix() {
    glm::mat4 transmat = glm::translate(glm::mat4(1.0f), transform_.translation);
    glm::mat4 rotmat = glm::rotate(glm::mat4(1.0f), transform_.angle, transform_.rotation);
    glm::mat4 scalemat = glm::scale(glm::mat4(1.0f), transform_.scale);
    return transmat * rotmat * scalemat;
}

glm::mat4 Entity::get_view_matrix(camera_t& cam) {
    return glm::lookAt(cam.pos, cam.pos + cam.fwd_dir, cam.up);
}

glm::mat4 Entity::get_proj_matrix(camera_t& cam) {
    return glm::perspective(cam.fov, cam.aspect_ratio, cam.near, cam.far);
}


