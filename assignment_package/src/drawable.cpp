#include "drawable.h"
#include <la.h>

Drawable::Drawable(OpenGLContext* context)
    : bufIdxOp(), bufIdxTr(), bufPos(), bufNor(), bufCol(), bufUV(), bufAnimCos(),
      idxOpBound(false), idxTrBound(false), posBound(false), norBound(false), colBound(false),
      uvBound(false), interOpBound(false), interTrBound(false), animCosBound(false),
      context(context)
{}

Drawable::~Drawable()
{}


void Drawable::destroy()
{
    context->glDeleteBuffers(1, &bufIdxOp);
    context->glDeleteBuffers(1, &bufIdxTr);
    context->glDeleteBuffers(1, &bufPos);
    context->glDeleteBuffers(1, &bufNor);
    context->glDeleteBuffers(1, &bufCol);
    context->glDeleteBuffers(1, &bufUV);
    context->glDeleteBuffers(1, &bufAnimCos);
    context->glDeleteBuffers(1, &bufInterOp);
    context->glDeleteBuffers(1, &bufInterTr);
}

GLenum Drawable::drawMode()
{
    // Since we want every three indices in bufIdx to be
    // read to draw our Drawable, we tell that the draw mode
    // of this Drawable is GL_TRIANGLES

    // If we wanted to draw a wireframe, we would return GL_LINES

    return GL_TRIANGLES;
}

int Drawable::elemCountOp()
{
    return countOp;
}

int Drawable::elemCountTr()
{
    return countTr;
}

void Drawable::generateOpIdx()
{
    idxOpBound = true;
    // Create a VBO on our GPU and store its handle in bufIdx
    context->glGenBuffers(1, &bufIdxOp);
}

void Drawable::generateTrIdx()
{
    idxTrBound = true;
    // Create a VBO on our GPU and store its handle in bufIdx
    context->glGenBuffers(1, &bufIdxTr);
}

void Drawable::generateOpInter()
{
    interOpBound = true;
    // Create a VBO on our GPU and store its handle in bufPos
    context->glGenBuffers(1, &bufInterOp);
}

void Drawable::generateTrInter()
{
    interTrBound = true;
    // Create a VBO on our GPU and store its handle in bufPos
    context->glGenBuffers(1, &bufInterTr);
}

void Drawable::generatePos()
{
    posBound = true;
    // Create a VBO on our GPU and store its handle in bufPos
    context->glGenBuffers(1, &bufPos);
}

void Drawable::generateNor()
{
    norBound = true;
    // Create a VBO on our GPU and store its handle in bufNor
    context->glGenBuffers(1, &bufNor);
}

void Drawable::generateCol()
{
    colBound = true;
    // Create a VBO on our GPU and store its handle in bufCol
    context->glGenBuffers(1, &bufCol);
}

void Drawable::generateUV()
{
    uvBound = true;
    // Create a VBO on our GPU and store its handle in bufCol
    context->glGenBuffers(1, &bufUV);
}

void Drawable::generateAnimCos()
{
    animCosBound = true;
    // Create a VBO on our GPU and store its handle in bufCol
    context->glGenBuffers(1, &bufAnimCos);
}

bool Drawable::bindOpIdx()
{
    if(idxOpBound) {
        context->glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, bufIdxOp);
    }
    return idxOpBound;
}

bool Drawable::bindTrIdx()
{
    if(idxTrBound) {
        context->glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, bufIdxTr);
    }
    return idxTrBound;
}

bool Drawable::bindOpInter()
{
    if(interOpBound){
        context->glBindBuffer(GL_ARRAY_BUFFER, bufInterOp);
    }
    return interOpBound;
}

bool Drawable::bindTrInter()
{
    if(interTrBound){
        context->glBindBuffer(GL_ARRAY_BUFFER, bufInterTr);
    }
    return interTrBound;
}

bool Drawable::bindPos()
{
    if(posBound){
        context->glBindBuffer(GL_ARRAY_BUFFER, bufPos);
    }
    return posBound;
}

bool Drawable::bindNor()
{
    if(norBound){
        context->glBindBuffer(GL_ARRAY_BUFFER, bufNor);
    }
    return norBound;
}

bool Drawable::bindCol()
{
    if(colBound){
        context->glBindBuffer(GL_ARRAY_BUFFER, bufCol);
    }
    return colBound;
}

bool Drawable::bindUV()
{
    if(uvBound){
        context->glBindBuffer(GL_ARRAY_BUFFER, bufUV);
    }
    return uvBound;
}

bool Drawable::bindAnimCos()
{
    if(animCosBound){
        context->glBindBuffer(GL_ARRAY_BUFFER, bufAnimCos);
    }
    return animCosBound;
}
