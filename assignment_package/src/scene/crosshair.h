#ifndef CROSSHAIR_H
#define CROSSHAIR_H

#include "drawable.h"
#include <la.h>


class Crosshair : public Drawable
{
public:
    int width;
    int height;

    Crosshair(OpenGLContext* context);
    void create() override;
    void setScreenWidthAndHeight(int w, int h);
    GLenum drawMode() override;
};

#endif // CROSSHAIR_H
