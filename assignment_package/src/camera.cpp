#include "camera.h"

#include <la.h>
#include <iostream>

Camera::Camera():
    Camera(400, 400)
{
    look = glm::vec3(0,0,1);
    up = glm::vec3(0,1,0);
    right = glm::vec3(1,0,0);
}

Camera::Camera(unsigned int w, unsigned int h):
    Camera(w, h, glm::vec3(0,0,10), glm::vec3(0,0,0), glm::vec3(0,1,0))
{}

Camera::Camera(unsigned int w, unsigned int h, const glm::vec3 &e, const glm::vec3 &r, const glm::vec3 &worldUp):
    fovy(70),
    width(w),
    height(h),
    near_clip(0.1f),
    far_clip(1000),
    eye(e),
    ref(r),
    world_up(worldUp),
    phi(0),
    theta(0),
    zoom(10),
    walking(false)
{
    RecomputeAttributes();
}

Camera::Camera(const Camera &c):
    fovy(c.fovy),
    width(c.width),
    height(c.height),
    near_clip(c.near_clip),
    far_clip(c.far_clip),
    aspect(c.aspect),
    eye(c.eye),
    ref(c.ref),
    look(c.look),
    up(c.up),
    right(c.right),
    world_up(c.world_up),
    V(c.V),
    H(c.H),
    walking(false)
{}


void Camera::RecomputeAttributes()
{
    look = glm::vec3(0,0,1);
    up = glm::vec3(0,1,0);
    right = glm::vec3(1,0,0);

    glm::mat4 rotUp = glm::rotate(glm::mat4(1.0f), glm::radians(theta), up);
    glm::vec4 modRight = rotUp * glm::vec4(right,0);

    glm::mat4 rotRight = glm::rotate(glm::mat4(1.0f),glm::radians(phi),glm::vec3(modRight));

    look = glm::vec3(rotRight * rotUp * glm::vec4(look,0));
    up = glm::vec3(rotRight * rotUp * glm::vec4(up,0));
    right = glm::vec3(rotRight * rotUp * glm::vec4(right,0));

//    look = glm::normalize(ref - eye);
//    right = glm::normalize(glm::cross(look, world_up));
//    up = glm::cross(right, look);

//    float tan_fovy = tan(glm::radians(fovy/2));
//    float len = glm::length(ref - eye);
//    aspect = width/height;
//    V = up*len*tan_fovy;
//    H = right*len*aspect*tan_fovy;
}

glm::mat4 Camera::getViewProj()
{
    return glm::perspective(glm::radians(fovy), width / (float)height, near_clip, far_clip) * glm::lookAt(eye, eye + look, up);
}

void Camera::RotateAboutUp(float deg)
{
    theta += deg;
//    glm::mat4 rotation = glm::rotate(glm::mat4(1.0f), glm::radians(deg), up);
//    ref = ref - eye;
//    ref = glm::vec3(rotation * glm::vec4(ref, 1));
//    ref = ref + eye;

    if(theta >= 360) { theta = theta - 360; }
    if(theta < 0) {theta = 360 + theta;}
    RecomputeAttributes();
}
void Camera::RotateAboutRight(float deg)
{
    phi += deg;
//    glm::mat4 rotation = glm::rotate(glm::mat4(1.0f), glm::radians(deg), right);
//    ref = ref - eye;
//    ref = glm::vec3(rotation * glm::vec4(ref, 1));
//    ref = ref + eye;

    phi = fmax(fmin(phi,89),-89);
    // phi = glm::clamp(theta,-90.f,90.f)
    RecomputeAttributes();
}

void Camera::TranslateAlongLook(float amt)
{

    glm::vec3 translation = look * amt;

    if(walking) {
        translation[1] = 0; //y coordinate of look vector = 0
       // translation = glm::normalize(translation); //This is to be revisted
    }

    eye += translation;
}

void Camera::TranslateAlongRight(float amt)
{
    glm::vec3 translation = right * amt;
    eye += translation;
}

void Camera::TranslateAlongUp(float amt)
{
    glm::vec3 translation = up * amt;
    eye += translation;
}
