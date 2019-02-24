#pragma once
#include <QList>
#include <la.h>
#include <QHash>
#include "drawable.h"
#include "turtle.h"
#include "linearlsystem.h"
#include "deltalsystem.h"
#include <stack>
#include <QMutex>
#include <QThreadPool>

using namespace glm;

// C++ 11 allows us to define the size of an enum. This lets us use only one byte
// of memory to store our different block types. By default, the size of a C++ enum
// is that of an int (so, usually four bytes). This *does* limit us to only 256 different
// block types, but in the scope of this project we'll never get anywhere near that many.
enum BlockType : unsigned char
{
    EMPTY, GRASS, DIRT, STONE, WOOD, LEAF, BEDROCK, LAVA, WATER, ICE, NPChar
};

class Chunk;
class LinearLSystem;
class DeltaLSystem;

class Terrain
{
public:
    Terrain(OpenGLContext* context);
    BlockType m_blocks[64][256][64];                    // A 3D list of the blocks in the world.
                                                           // You'll need to replace this with a far more
                                                           // efficient system of storing terrain.
    OpenGLContext* context;
    QHash<int64_t, Chunk*>* m_chunks;

    // These are variables for turtle, L-System River
    LinearLSystem* linearlsystem;
    DeltaLSystem* deltalsystem;
    Turtle linear_turtle;
    std::stack<Turtle> linear_turtle_history;
    Turtle delta_turtle;
    std::stack<Turtle> delta_turtle_history;

    // These are methods for turtle, L-System River
    void removeDirtAndGrass(int x, int z);
    void carveBlocksBelowRiver(int x, int z);
    void renderLinearRiver(std::vector<glm::vec2> river_positions);
    void renderDeltaRiver(std::vector<glm::vec2> river_positions);
    std::vector<glm::vec2> getLinearRiverPositions();
    std::vector<glm::vec2> getDeltaRiverPositions();

    int getPerlinY(int x, int z);
    float rand_vec2(vec2 n);
    float fbm(float x, float y);
    float interpNoise2D(float x, float y);
    void createScene(int min_x_in, int max_x_in, int min_z_in, int max_z_in);

    // These will be set in createScene()
    int min_x = 3200 - 32;
    int max_x = 3200 + 32;
    int min_z = 3200 - 32;
    int max_z = 3200 + 32;
    int min_y = 0;
    int max_y = 256;

    //std::vector<*Chunk> chunks; //list of chunks that are going to be added each time createScene is invoked

    BlockType getBlockAt(int x, int y, int z); // const;   // Given a world-space coordinate (which may have negative
                                                           // values) return the block stored at that point in space.
    void setBlockAt(int x, int y, int z, BlockType t); // Given a world-space coordinate (which may have negative
                                                           // values) set the block at that point in space to the
                                                           // given type.
    int64_t locToKey(float x, float z);
    ivec2 getRelCoord(int x, int z);

};

class Chunk : public Drawable
{
public:
    Chunk(OpenGLContext* context);
    Chunk(OpenGLContext* context, QHash<int64_t, Chunk *>* globChunks);
    BlockType m_chunkBlocks[16*256*16];

    QHash<int64_t, Chunk*>* m_globalChunks;
    bool hasChanged;                                        // determines if the chunks VBO should be created


    struct vboStruct
    {
        glm::vec4 pos;
        glm::vec4 nor;
        glm::vec4 col;
        glm::vec2 uv;
        glm::vec2 flagCos;   // animation flag, cosine power
    };

    vboStruct setVboData(glm::vec4 pos, glm::vec4 nor, glm::vec4 col, glm::vec2 uv, vec2 animCos); // Helper function to format data form VBO

    void create();
    BlockType getBlockFromChunk(int x, int y, int z) const;   // x, y, z relative to minimum corner of chunk
//    BlockType* getBlockFromChunk(int x, int y, int z);        // x, y, z relative to minimum corner of chunk

    void addFaceToVbo(BlockType t, int x, int y, int z, glm::vec4 dir, std::vector<vboStruct> *interleaveVBO, std::vector<GLuint> *chunk_idx, int *idx);
    int64_t locToKey(int x, int z);
};
