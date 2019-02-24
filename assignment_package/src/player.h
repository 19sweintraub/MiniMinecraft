#pragma once

#include <la.h>
#include <glm/glm.hpp>

#include "camera.h"
#include "qevent.h"

class Player {

public:

    Player(Camera* c);

    Camera* cam;
    glm::vec3 pos; //position
    glm::vec3 vel; //velocity - <right, forward, up, 1>

    bool w_pressed;
    bool a_pressed;
    bool s_pressed;
    bool d_pressed;
    bool space_pressed;
    bool followMe;

    bool flying;
    bool swimming;
    bool inLava;

    bool right_mouse;
    bool left_mouse;

    //These numbers are used for tracking the mouse movement
    float finalX;
    float finalY;

    void updateKeyEvent(QKeyEvent *e, int state);
    void updateMouseEvent(QMouseEvent *e, int state, int w, int h);
    void updateCamera(int w, int h);
    void updateVelocity();

};
