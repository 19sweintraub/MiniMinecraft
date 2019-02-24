#include "quad.h"
#include "terrain.h"

Quad::Quad(OpenGLContext *context) : Drawable(context)
{}

void Quad::create()
{

    std::vector<GLuint> buf_idx;
    std::vector<Chunk::vboStruct> interleave;   //  pos / nor / col / uv / flag

    buf_idx.push_back(0);
    buf_idx.push_back(1); // 0 and 1 are for horizontal line
    buf_idx.push_back(2);
    buf_idx.push_back(0);
    buf_idx.push_back(2);
    buf_idx.push_back(3); // 2 and 3 are for vertical line

    Chunk::vboStruct toPushBack;

    toPushBack.pos = glm::vec4(-1.f, -1.f, 0.999999f, 1.f);
    toPushBack.nor = glm::vec4(1, 1, 1, 1);
    toPushBack.col = glm::vec4(0, 1, 0, 0.5);  // never used
    toPushBack.uv = glm::vec2(0.f, 0.f);
    toPushBack.flagCos = glm::vec2(0, 0);

    interleave.push_back(toPushBack);

    toPushBack.pos = glm::vec4(1.f, -1.f, 0.999999f, 1.f);
    toPushBack.uv = glm::vec2(1.f, 0.f);

    interleave.push_back(toPushBack);

    toPushBack.pos = glm::vec4(1.f, 1.f, 0.999999f, 1.f);
    toPushBack.uv = glm::vec2(1.f, 1.f);

    interleave.push_back(toPushBack);

    toPushBack.pos = glm::vec4(-1.f, 1.f, 0.999999f, 1.f);
    toPushBack.uv = glm::vec2(0.f, 1.f);

    interleave.push_back(toPushBack);

    countOp = 6; // Is this supposed to be 4, for 4 vertices?

    generateOpIdx();
    context->glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, bufIdxOp);
    context->glBufferData(GL_ELEMENT_ARRAY_BUFFER, buf_idx.size() * sizeof(GLuint), buf_idx.data(), GL_STATIC_DRAW);

    generateOpInter();
    context->glBindBuffer(GL_ARRAY_BUFFER, bufInterOp);
    context->glBufferData(GL_ARRAY_BUFFER, 4 * sizeof(Chunk::vboStruct), interleave.data(), GL_STATIC_DRAW);
}
