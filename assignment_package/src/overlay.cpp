#include "overlay.h"
#include "scene/terrain.h"

Overlay::Overlay(OpenGLContext* context): Drawable(context), state(0) {}

void Overlay::create() {

    std::vector<GLuint> buf_idx;
    std::vector<Chunk::vboStruct> interleave;   //  pos / nor / col / uv / flag

    buf_idx.push_back(0);
    buf_idx.push_back(1);
    buf_idx.push_back(2);
    buf_idx.push_back(0);
    buf_idx.push_back(2);
    buf_idx.push_back(3);

    Chunk::vboStruct toPushBack;

    //bottom left
    toPushBack.pos = glm::vec4(-1, -1, 0.11, 1);
    toPushBack.nor = glm::vec4(1, 1, 1, 1);

    if(state == 1){
        toPushBack.col = glm::vec4(0, 0, 1, 0.3);
    }

    if(state == 2){
        toPushBack.col = glm::vec4(1,0,1,0.3);
    }
    toPushBack.uv = glm::vec2(0, 0);
    toPushBack.flagCos = glm::vec2(0, 0);

    interleave.push_back(toPushBack);

    //top left
    toPushBack.pos = glm::vec4(-1, 1, 0.11, 1);

    interleave.push_back(toPushBack);

    //top right
    toPushBack.pos = glm::vec4(1, 1, 0.11, 1);

    interleave.push_back(toPushBack);

    //bottom right
    toPushBack.pos = glm::vec4(1, -1, 0.11, 1);

    interleave.push_back(toPushBack);

    countOp = 6; // Is this supposed to be 4, for 4 vertices?

    generateOpIdx();
    context->glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, bufIdxOp);
    context->glBufferData(GL_ELEMENT_ARRAY_BUFFER, buf_idx.size() * sizeof(GLuint), buf_idx.data(), GL_STATIC_DRAW);

    generateOpInter();
    context->glBindBuffer(GL_ARRAY_BUFFER, bufInterOp);
    context->glBufferData(GL_ARRAY_BUFFER, 4 * sizeof(Chunk::vboStruct), interleave.data(), GL_STATIC_DRAW);
}
