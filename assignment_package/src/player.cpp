#include "player.h"
#include <iostream>

Player::Player(Camera* c)
    : cam(c), pos(glm::vec3(3200, 0, 3200)), vel(glm::vec3(0)), w_pressed(false), a_pressed(false),
      s_pressed(false), d_pressed(false), space_pressed(false),
      right_mouse(false), left_mouse(false), flying(true), swimming(false), inLava(false), followMe(false) {}

void Player::updateKeyEvent(QKeyEvent *e, int state) {

    float amount = 5;

    switch (e->key()) {
    case Qt::Key_W:
        //cam->TranslateAlongLook(amount);
        if (state == 1) { w_pressed = true; }
        if (state == 0) { w_pressed = false; }
        break;
    case Qt::Key_A:
        //cam->TranslateAlongRight(-amount);
        if (state == 1) { a_pressed = true; }
        if (state == 0) { a_pressed = false; }
        break;
    case Qt::Key_S:
        //cam->TranslateAlongLook(-amount);
        if (state == 1) { s_pressed = true; }
        if (state == 0) { s_pressed = false; }
        break;
    case Qt::Key_D:
        //cam->TranslateAlongRight(amount);
        if (state == 1) { d_pressed = true; }
        if (state == 0) { d_pressed = false; }
        break;
    case Qt::Key_Space:
        if (state == 1) { space_pressed = true; }
        if (state == 0) { space_pressed = false; }
        break;
    case Qt::Key_F:
        if (state == 1) {
            flying = !flying;
        }
        break;
    case Qt::Key_Q:
        cam->TranslateAlongUp(-amount/3);
        break;
    case Qt::Key_E:
        cam->TranslateAlongUp(amount/3);
        break;
    case Qt::Key_Right:
        cam->RotateAboutUp(-amount/2);
        break;
    case Qt::Key_Left:
        cam->RotateAboutUp(amount/2);
        break;
    case Qt::Key_Up:
        cam->RotateAboutRight(-amount);
        break;
    case Qt::Key_Down:
        cam->RotateAboutRight(amount);
        break;
    case Qt::Key_1:
        cam->fovy += amount;
        break;
    case Qt::Key_2:
        cam->fovy -= amount;
    case Qt::Key_P:
        if(state == 1) { followMe = !followMe; }
        break;
    default:
        break;
    }
    cam->RecomputeAttributes();
}

void Player::updateMouseEvent(QMouseEvent *e, int state, int w, int h) {
    if(e->button() == Qt::LeftButton) {
        if(state == 1) {
            left_mouse = true;
        }

        if(state == 0) {
            left_mouse = false;
        }
    }

    else if(e->button() == Qt::RightButton) {
        if(state == 1) { right_mouse = true; }
        if(state == 0) { right_mouse = false; }
    }

    // Chris removed this temporarily so can debug and not have mouse glued to center
    else if(state == 2) {
        finalX = e->windowPos().x();
        finalY = e->windowPos().y();

        this->updateCamera(w,h);
    }
}

//update camera
void Player::updateCamera(int w, int h) {

    float deltaX = finalX - w/2;// - pos().x();
    float deltaY = finalY - h/2;// - pos().y();

    deltaX *= 0.03;
    deltaY *= -0.03;

    cam->RotateAboutRight(deltaY);
    cam->RotateAboutUp(deltaX);

}

//Update the velocity based on which keys are being held
void Player::updateVelocity() {

    float speed = 0.005;

    if(flying) { speed = 0.01; }

    if(swimming || inLava) {
        speed = speed * 2/3;
    }

    //move forward
    vel[0] = 0;
    vel[1] = 0;

    if(w_pressed) { vel[1] = 1*speed; }

    if(a_pressed) { vel[0] = 1*speed; }

    if(s_pressed) { vel[1]  = -1*speed; }

    if(d_pressed) { vel[0] = -1*speed; }

    if(space_pressed) { vel[2] = 0.005; }

    if(flying) { vel[2] = 0; }

}
