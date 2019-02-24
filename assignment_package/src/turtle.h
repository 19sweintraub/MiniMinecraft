#ifndef TURTLE_H
#define TURTLE_H

#include <glm/glm.hpp>

using namespace glm;

class Turtle
{
public:
    Turtle(glm::vec2 position, glm::vec2 orientation, int depth);

    glm::vec2 pos;
    glm::vec2 orient;
    int depth;
};

#endif // TURTLE_H
