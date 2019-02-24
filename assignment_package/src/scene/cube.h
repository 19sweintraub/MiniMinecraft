#pragma once

#include "drawable.h"
#include <la.h>

#include <QOpenGLContext>
#include <QOpenGLBuffer>
#include <QOpenGLShaderProgram>
#include "terrain.h"


class Cube : public Drawable
{
public:
    Cube(OpenGLContext* context);

    void create();
};
