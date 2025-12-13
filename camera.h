#pragma once
#include <glm/glm.hpp>

const float PLAYER_SPEED = 2.5f;
const float MOUSE_SENS = 0.1f;

class Camera {
public:
    glm::vec3 pos = glm::vec3(1.8f, 1.8f, 1.8f);
    glm::vec3 target = glm::vec3(0.0f, 0.0f, 0.0f);

    glm::vec3 front = glm::normalize(glm::vec3(0, 1, 0.5));

    // NOTE: This means for this project we are using Z-up and not Y-up convention
    glm::vec3 worldUp = glm::vec3(0.0f, 0.0f, 1.0f);
    //glm::vec3 right = glm::normalize(glm::cross(worldUp, getCameraDirection()));
    //glm::vec3 cameraUp = getCameraUp();

    float speed = PLAYER_SPEED;
    float mouseSens = MOUSE_SENS;


    glm::mat4 getViewMatrix() {
        return glm::lookAt(pos, pos + front, getCameraUp());
    }

    // Update the view angle of the camera based on the given x and y components
    // Each component is the relative difference between the new position of the
    // mouse and the prior position of the mouse
    void updateCameraRotation(float xrel, float yrel) {
        yaw -= xrel * mouseSens;
        pitch -= yrel * mouseSens;

        // Limit vertical look angle
        if (pitch > 89.0f) {
            pitch = 89.0f;
        }
        else if (pitch < -89.0f) {
            pitch = -89.0f;
        }

        glm::vec3 direction(
            cos(glm::radians(yaw)) * cos(glm::radians(pitch)), // X
            sin(glm::radians(yaw)) * cos(glm::radians(pitch)), // Y
            sin(glm::radians(pitch))                           // Z
        );

        front = glm::normalize(direction);
    }

    // This is only the X and Y components of the viewing angle, used when we need to reference a 2D plane such as when moving on flat ground
    glm::vec3 getCameraFrontHorizOnly() {
        return (glm::normalize(glm::vec3(front.x, front.y, 0)));
    }

    glm::vec3 getCameraRight() {
        return (glm::normalize(glm::cross(worldUp, front)));
    }

    glm::vec3 getCameraUp() {
        return (glm::cross(front, getCameraRight()));
    }

private:
    float pitch = 0.0f;
    float yaw = 0.0f;
};