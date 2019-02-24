#include "crosshair.h"
#include "terrain.h"

Crosshair::Crosshair(OpenGLContext* context): Drawable(context) {
    // I don't think anything needs to get instantiated.
}

void Crosshair::setScreenWidthAndHeight(int w, int h) {
    width = w;
    height = h;
}

void Crosshair::create() {
    // Should draw 2 lines

    // Coordinates are in screen space, between [-1, 1] in x and y directions.

    // Might be around 0.1f amount for cross hairs

    // Need to take into account screen dimensions / aspect ratio.

    std::vector<GLuint> buf_idx;
    std::vector<Chunk::vboStruct> interleave;   //  pos / nor / col / uv / flag

    buf_idx.push_back(0);
    buf_idx.push_back(1); // 0 and 1 are for horizontal line
    buf_idx.push_back(2);
    buf_idx.push_back(3); // 2 and 3 are for vertical line

    // TODO: First focus on the horizontal line. After that works, then try the vertical line.

    float crosshair_offset = 0.05f;
    float width_height_ratio = float(width) / float(height);
//    float crosshair_offset = 5.0f;

    Chunk::vboStruct toPushBack;

    toPushBack.pos = glm::vec4(-1.0 * crosshair_offset, 0, 0, 1);
    toPushBack.nor = glm::vec4(1, 1, 1, 1);
    toPushBack.col = glm::vec4(1, 1, 1, 1);
    toPushBack.uv = glm::vec2(0, 0);
    toPushBack.flagCos = glm::vec2(0, 0);

    interleave.push_back(toPushBack);

    toPushBack.pos = glm::vec4(crosshair_offset, 0, 0, 1);

    interleave.push_back(toPushBack);

    toPushBack.pos = glm::vec4(0, -1.0 * crosshair_offset * width_height_ratio, 0, 1);

    interleave.push_back(toPushBack);

    toPushBack.pos = glm::vec4(0, crosshair_offset * width_height_ratio, 0, 1);

    interleave.push_back(toPushBack);

    countOp = 4; // Is this supposed to be 4, for 4 vertices?

    generateOpIdx();
    context->glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, bufIdxOp);
    context->glBufferData(GL_ELEMENT_ARRAY_BUFFER, buf_idx.size() * sizeof(GLuint), buf_idx.data(), GL_STATIC_DRAW);

    generateOpInter();
    context->glBindBuffer(GL_ARRAY_BUFFER, bufInterOp);
    context->glBufferData(GL_ARRAY_BUFFER, 4 * sizeof(Chunk::vboStruct), interleave.data(), GL_STATIC_DRAW);
}

GLenum Crosshair::drawMode()
{
    return GL_LINES;
}
