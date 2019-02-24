#pragma once
#include "drawable.h"
#include <la.h>

class Overlay : public Drawable
{
public:
    Overlay(OpenGLContext* context);
    void create() override;
    int state; // 1 is water, 2 is lava
};
