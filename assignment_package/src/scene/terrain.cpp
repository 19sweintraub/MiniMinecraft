#include <scene/terrain.h>
#include <scene/cube.h>
#include <glm/gtx/noise.hpp>
#include <iostream>

Terrain::Terrain(OpenGLContext *context) :
    context(context), m_chunks(new QHash<int64_t, Chunk*>), linearlsystem(new LinearLSystem()),
    deltalsystem(new DeltaLSystem()),
    linear_turtle(glm::vec2(3200, 3200), glm::vec2(0, 1), 1), linear_turtle_history(),
    delta_turtle(glm::vec2(3200, 3200), glm::vec2(0, -1), 1), delta_turtle_history()
{}

BlockType Terrain::getBlockAt(int x, int y, int z)  // maybe make constant later const
{
    // get key
    int64_t key = locToKey(x, z);
    glm::ivec2 relCoord = getRelCoord(x, z);
    if (m_chunks->contains(key)) {
        return (*m_chunks)[key]->getBlockFromChunk(relCoord[0], y, relCoord[1]);
    } else {
        (*m_chunks)[key] = new Chunk(context, m_chunks);
        return (*m_chunks)[key]->getBlockFromChunk(relCoord[0], y, relCoord[1]);
    }
}

void Terrain::removeDirtAndGrass(int x, int z) {
    int y_to_remove = getPerlinY(x, z);
    // remove underneath layers of dirt
//    for (int y = 128; y < y_to_remove; y++) {
//        setBlockAt(x, y_to_remove, z, EMPTY);
//    }
    // remove top layer of dirt
    setBlockAt(x, y_to_remove, z, EMPTY);
    // remove top layer of grass
    setBlockAt(x, y_to_remove + 1, z, EMPTY);
}

// Helper function which carves out the stone blocks beneath the river so that
// the player can go underneath the water
void Terrain::carveBlocksBelowRiver(int x, int z) {
    // We assume water layer is at height of y = 128
    // Stone blocks are between y = 120 and y = 127
    // So we remove the stone blocks from 124 to 127
    setBlockAt(x, 124, z, EMPTY);
    setBlockAt(x, 125, z, EMPTY);
    setBlockAt(x, 126, z, EMPTY);
    setBlockAt(x, 127, z, EMPTY);
}

// This function renders the river water blocks, carves out terrain.
// This will be the separate function which takes the river positions
// and draws them. That way random positions are only generated once, then
// stored. Then we just re-draw the saved river positions.
void Terrain::renderLinearRiver(std::vector<glm::vec2> river_positions) {
    int RIVER_HEIGHT = 128;
    int TERRAIN_SLOPE_DOWN_OFFSET = 10;

    for (glm::vec2 curr_pos : river_positions) {
        int x = (int)curr_pos[0];
        int z = (int)curr_pos[1];

        // Remove the block for perlin terrain
        removeDirtAndGrass(x, z);
        // Set the block for river at a height of 128
        setBlockAt(x, RIVER_HEIGHT, z, WATER);
        // Remove the stone blocks below water
        carveBlocksBelowRiver(x, z);

        // Linear river has a width of 5 blocks

        // Also need to remove terrain above
        removeDirtAndGrass(x-2, z);
        removeDirtAndGrass(x-1, z);
        removeDirtAndGrass(x+1, z);
        removeDirtAndGrass(x+2, z);

        setBlockAt(x-2, RIVER_HEIGHT, z, WATER);
        setBlockAt(x-1, RIVER_HEIGHT, z, WATER);
        setBlockAt(x+1, RIVER_HEIGHT, z, WATER);
        setBlockAt(x+2, RIVER_HEIGHT, z, WATER);
        // Remove the stone blocks below water
        carveBlocksBelowRiver(x-2, z);
        carveBlocksBelowRiver(x-1, z);
        carveBlocksBelowRiver(x+1, z);
        carveBlocksBelowRiver(x+2, z);

        // Terrain to the left of x-2 and right of x+2 needs to slope down
        for (int i = 0; i < TERRAIN_SLOPE_DOWN_OFFSET; i++) {
            // Remove TERRAIN_SLOPE_DOWN_OFFSET number of blocks with z less than
            int x_left = x-2-i;
            removeDirtAndGrass(x_left, z);
            if (getBlockAt(x_left, RIVER_HEIGHT, z) != WATER) {
                setBlockAt(x_left, RIVER_HEIGHT + i/3 - 1, z, DIRT);
                setBlockAt(x_left, RIVER_HEIGHT + i/3, z, GRASS);
            }

            // Remove TERRAIN_SLOPE_DOWN_OFFSET number of blocks with z greater than
            int x_right = x+2+i;
            removeDirtAndGrass(x_right, z);
            if (getBlockAt(x_right, RIVER_HEIGHT, z) != WATER) {
                setBlockAt(x_right, RIVER_HEIGHT + i/3 - 1, z, DIRT);
                setBlockAt(x_right, RIVER_HEIGHT + i/3, z, GRASS);
            }
        }
    }
}

void Terrain::renderDeltaRiver(std::vector<glm::vec2> river_positions) {
    int RIVER_HEIGHT = 128;
    int TERRAIN_SLOPE_DOWN_OFFSET = 5;

    for (glm::vec2 curr_pos : river_positions) {
        int x = (int)curr_pos[0];
        int z = (int)curr_pos[1];

        // Remove the block for perlin terrain
        removeDirtAndGrass(x, z);
        // Set the block for river at a height of 128
        setBlockAt(x, RIVER_HEIGHT, z, WATER);
        // Remove the stone blocks below water
        carveBlocksBelowRiver(x, z);

        // Delta river has a smaller width of 3 blocks, since it branches more than a linear river

        // Also need to remove terrain above
        removeDirtAndGrass(x-1, z);
        removeDirtAndGrass(x+1, z);

        // If orientation is (0, 1), due north or south, then render with x-1, x+1
        setBlockAt(x-1, RIVER_HEIGHT, z, WATER);
        setBlockAt(x+1, RIVER_HEIGHT, z, WATER);
        // Remove the stone blocks below water
        carveBlocksBelowRiver(x-1, z);
        carveBlocksBelowRiver(x+1, z);

        // Terrain to the left of x-2 and right of x+2 needs to slope down
        for (int i = 0; i < TERRAIN_SLOPE_DOWN_OFFSET; i++) {
            // Remove TERRAIN_SLOPE_DOWN_OFFSET number of blocks with z less than
            int x_left = x-1-i;
            removeDirtAndGrass(x_left, z);
            if (getBlockAt(x_left, RIVER_HEIGHT, z) != WATER) {
                setBlockAt(x_left, RIVER_HEIGHT + i/3 - 1, z, DIRT);
                setBlockAt(x_left, RIVER_HEIGHT + i/3, z, GRASS);
            }

            // Remove TERRAIN_SLOPE_DOWN_OFFSET number of blocks with z greater than
            int x_right = x+1+i;
            removeDirtAndGrass(x_right, z);
            if (getBlockAt(x_right, RIVER_HEIGHT, z) != WATER) {
                setBlockAt(x_right, RIVER_HEIGHT + i/3 - 1, z, DIRT);
                setBlockAt(x_right, RIVER_HEIGHT + i/3, z, GRASS);
            }
        }
    }
}

std::vector<glm::vec2> Terrain::getLinearRiverPositions()
{
    std::vector<glm::vec2> river_positions;

    // 1. Set initial axiom.
    QString axiom = QString("X");

    // 2. Expand the grammar for some number of iterations. For now, just one iteration.
    int NUM_ITERATIONS = 64;
    QString expanded_grammar = axiom;
    for (int i = 0; i < NUM_ITERATIONS; i++) {
        expanded_grammar = linearlsystem->expandGrammar(expanded_grammar);
    }

    // 3. For each character in the grammar, call the drawing operation.
    for (QChar c : expanded_grammar) {
        if (c == 'F') {
            glm::vec2 curr_pos = linear_turtle.pos;
            river_positions.push_back(curr_pos);
            // Lastly, advance the turtle position forward in that orientation
            float len = 1.0f;
            linear_turtle.pos += linear_turtle.orient * len;
        } else if (c == '[') {
            // Save turtle state
            linear_turtle.depth = linear_turtle.depth + 1;
            linear_turtle_history.push(linear_turtle);
        } else if (c == ']') {
            // Pop turtle state
            linear_turtle = linear_turtle_history.top();
            linear_turtle_history.pop();
        } else if (c == 'P') {
            // Perturb orientation
            float max_angle = 30.0;
            float orientation = ((float)rand()) / RAND_MAX * max_angle - max_angle / 2;
            float cos = glm::cos(glm::radians(orientation));
            float sin = glm::sin(glm::radians(orientation));
            glm::vec2 o = linear_turtle.orient;
            linear_turtle.orient.x = cos * o.x - sin * o.y;
            linear_turtle.orient.y = sin * o.x + cos * o.y;
        }
    }

    return river_positions;
}

std::vector<glm::vec2> Terrain::getDeltaRiverPositions()
{
    std::vector<glm::vec2> river_positions;

    // 1. Set initial axiom.
    QString axiom = QString("X");

    // 2. Expand the grammar for some number of iterations. For now, just one iteration.
    int NUM_ITERATIONS = 50;
    QString expanded_grammar = axiom;
    for (int i = 0; i < NUM_ITERATIONS; i++) {
        expanded_grammar = deltalsystem->expandGrammar(expanded_grammar);
    }

    // 3. For each character in the grammar, call the drawing operation.
    for (QChar c : expanded_grammar) {
        if (c == 'F') {
            glm::vec2 curr_pos = delta_turtle.pos;
            river_positions.push_back(curr_pos);
            // Lastly, advance the turtle position forward in that orientation
            float len = 1.0f;
            delta_turtle.pos += delta_turtle.orient * len;
        } else if (c == '[') {
            // Save turtle state
            delta_turtle.depth = delta_turtle.depth + 1;
            delta_turtle_history.push(delta_turtle);
        } else if (c == ']') {
            // Pop turtle state
            delta_turtle = delta_turtle_history.top();
            delta_turtle_history.pop();
        } else if (c == 'P') {
            // Perturb orientation
            float max_angle = 50.0;
            float orientation = ((float)rand()) / RAND_MAX * max_angle - max_angle / 2;
            float cos = glm::cos(glm::radians(orientation));
            float sin = glm::sin(glm::radians(orientation));
            glm::vec2 o = delta_turtle.orient;
            delta_turtle.orient.x = cos * o.x - sin * o.y;
            delta_turtle.orient.y = sin * o.x + cos * o.y;
        }
    }
    return river_positions;
}


void Terrain::setBlockAt(int x, int y, int z, BlockType t)
{
    // get key
    int64_t key = locToKey(x, z);
    glm::ivec2 relCoord = getRelCoord(x, z);
    if (m_chunks->contains(key)) {
        (*m_chunks)[key]->m_chunkBlocks[relCoord[0] + 16*y + 16*256*relCoord[1]] = t;
    } else {
        (*m_chunks)[key] = new Chunk(context, m_chunks);
        (*m_chunks)[key]->m_chunkBlocks[relCoord[0] + 16*y + 16*256*relCoord[1]] = t;
    }
    (*m_chunks)[key]->hasChanged = true;
}

float Terrain::rand_vec2(vec2 n) {
    return (fract(sin(dot(n, vec2(12.9898, 4.1414))) * 43758.5453));
}

float Terrain::fbm(float x, float z) {
    float total = 0;
    float persistence = 0.25f;
    int octaves = 6;

    for (int i = 0; i < octaves; i++) {
        float freq = std::pow(2.f, i);
        float amp = std::pow(persistence, i);
        total += interpNoise2D(x * freq, z * freq) * amp;
    }

    return total;
}

float Terrain::interpNoise2D(float x, float z) {
    float intX = floor(x);
    float fractX = fract(x);
    float intZ = floor(z);
    float fractZ = fract(z);

    float v1 = rand_vec2(vec2(intX, intZ));
    float v2 = rand_vec2(vec2(intX + 1, intZ));
    float v3 = rand_vec2(vec2(intX, intZ + 1));
    float v4 = rand_vec2(vec2(intX + 1, intZ + 1));

    float i1 = mix(v1, v2, fractX);
    float i2 = mix(v3, v4, fractX);
    return mix(i1, i2, fractZ);
}

int Terrain::getPerlinY(int x, int z) {
    float HEIGHT_SCALING_FACTOR = 5.0;
    float x_input = float(x) / 10.0;
    float z_input = float(z) / 10.0;
    float noise = fbm(x_input, z_input); // Noise is between [-1, 1]
    int y = int(noise * HEIGHT_SCALING_FACTOR) + 128;
    return y;
}

// Creates a terrain chunk (initial chunks or newly added chunks) within
// the bounding box represented by min_z, max_z, min_z, and max_z
// IMPORTANT: Only renders blocks within this bounding box. Not entire terrain.
void Terrain::createScene(int min_x_in, int max_x_in, int min_z_in, int max_z_in)
{
    // Create the basic terrain floor
    for(int x = min_x_in; x < max_x_in; ++x)
    {
        for(int z = min_z_in; z < max_z_in; ++z)
        {
            int y_random = getPerlinY(x, z);
            // Also need to set DIRT between 128 to y_random-1
//            for (int y = 128; y < y_random; y++) {
//                setBlockAt(x, y, z, DIRT);
//            }
            setBlockAt(x, y_random, z, DIRT);
            setBlockAt(x, y_random+1, z, GRASS);
            // Should create stones between 120 and 127
            for (int y = 120; y < 127; y++) {
                setBlockAt(x, y, z, BEDROCK);
            }
        }
    }

    // Set the terrain's min_x, max_x, etc. variables given these inputs
    if (min_x_in < this->min_x) {
        this->min_x = min_x_in;
    }
    if (max_x_in > this->max_x) {
        this->max_x = max_x_in;
    }
    if (min_z_in < this->min_z) {
        this->min_z = min_z_in;
    }
    if (max_z_in > this->max_z) {
        this->max_z = max_z_in;
    }
}

int64_t Terrain::locToKey(float x, float z) {
    // x and z are in world coordinates

    int xCorner = floor(x/16.0);
    int zCorner = floor(z/16.0);

    return ((int64_t)xCorner << 32) | zCorner;

}

glm::ivec2 Terrain::getRelCoord(int x, int z) {

    float xDivide = floor(x/16.0);
    float zDivide = floor(z/16.0);

    return glm::ivec2(floor(x - xDivide*16.0), floor(z - zDivide*16.0));
}

Chunk::Chunk(OpenGLContext* context): Drawable(context), hasChanged(true) {

    BlockType t = EMPTY;
    for (int i = 0; i < 16*16*256; i++) {
        m_chunkBlocks[i] = t;
    }

    // Stone, dirt, grass, wood, lead, bedrock, lava, water, ice
    // lava, water are animated
    // grass has different uvs for each face

}

Chunk::Chunk(OpenGLContext* context, QHash<int64_t, Chunk *>* globChunks): Drawable(context), m_globalChunks(globChunks), hasChanged(true) {

    BlockType t = EMPTY;
    for (int i = 0; i < 16*16*256; i++) {
        m_chunkBlocks[i] = t;
    }

}

BlockType Chunk::getBlockFromChunk(int x, int y, int z) const {

    return m_chunkBlocks[x + 16*y + 256*16*z];

}

void Chunk::create() {

    std::vector<vboStruct> interleaveOpaque;   //  pos / nor / col / uv / flag
    std::vector<vboStruct> interleaveTrans;   //  pos / nor / col / uv / flag
    std::vector<vboStruct>* VBO = nullptr; // initialize

    std::vector<GLuint> chunk_idx_opaque;
    std::vector<GLuint> chunk_idx_trans;
    std::vector<GLuint>* indices = nullptr; // initialize

    int idxOpaque = 0;
    int idxTrans = 0;
    int* index = nullptr;

    for(int x = 0; x < 16; x++) {
        for(int y = 0; y < 256; y++) {
            for(int z = 0; z < 16; z++) {

                // check surrounding blocks
                BlockType block = m_chunkBlocks[x + 16*y + 256*16*z];

                // set which VBO to populate
                switch (block) {
                case WATER:
                    VBO = &interleaveTrans;
                    index = &idxTrans;
                    indices = &chunk_idx_trans;
                    break;
                default:
                    VBO = &interleaveOpaque;
                    index = &idxOpaque;
                    indices = &chunk_idx_opaque;
                    break;
                }

                // for each block build a convex hull of the faces... if the block is transparent (water) then do not add faces between them
                if (block != EMPTY) {
                    for (int off = -1; off <= 1; off += 2) {
                        BlockType adj;
                        if ( ((x+off) >= 0) && ((x + off) < 16)) {
                            // surrounding chunks
                            adj = m_chunkBlocks[(x + off) + 16*y + 256*16*z];
                            if ((adj == EMPTY) || ((adj == WATER) && (block != WATER))) {
                                glm::vec4 dir = float(off)*glm::vec4(1, 0, 0, 0);
                                addFaceToVbo(block, x, y, z, dir, VBO, indices, index);
                            }
                        }
                        else if (!m_globalChunks->contains(locToKey((x+off), z))) {
                            glm::vec4 dir = float(off)*glm::vec4(1, 0, 0, 0);
                            if (block != WATER) {
                                addFaceToVbo(block, x, y, z, dir, VBO, indices, index);
                            }
                        }
                        else {
                            adj = (*m_globalChunks)[locToKey((x+off), z)]->getBlockFromChunk((x+off), y, z);
                            if ((adj == EMPTY) || ((adj == WATER) && (block != WATER))) {
                                glm::vec4 dir = float(off)*glm::vec4(1, 0, 0, 0);
                                addFaceToVbo(block, x, y, z, dir, VBO, indices, index);
                            }
                        }
                    }


                    for (int off = -1; off <= 1; off += 2) {
                        if ( ((y+off) >= 0) && ((y + off) < 256)) {
                            // surrounding chunks
                            BlockType adj = m_chunkBlocks[x + 16*(y+off) + 256*16*z];
                            if ((adj == EMPTY) || ((adj == WATER) && (block != WATER))) {
                                glm::vec4 dir = float(off)*glm::vec4(0, 1, 0, 0);
                                if (!((block == WATER) && (off == -1))) {
                                    addFaceToVbo(block, x, y, z, dir, VBO, indices, index);
                                }
                            }
                        }
                        else {
                            glm::vec4 dir = float(off)*glm::vec4(0, 1, 0, 0);
                            addFaceToVbo(block, x, y, z, dir, VBO, indices, index);
                        }
                    }

                    for (int off = -1; off <= 1; off += 2) {
                        BlockType adj;
                        if ( ((z+off) >= 0) && ((z + off) < 16)) {
                            // surrounding chunks
                            adj = m_chunkBlocks[x + 16*y + 256*16*(z+off)];
                            if ((adj == EMPTY) || ((adj == WATER) && (block != WATER))) {
                                glm::vec4 dir = float(off)*glm::vec4(0, 0, 1, 0);
                                addFaceToVbo(block, x, y, z, dir, VBO, indices, index);
                            }
                        }
                        else if (!m_globalChunks->contains(locToKey(x, (z+off)))) {
                            glm::vec4 dir = float(off)*glm::vec4(0, 0, 1, 0);
                            if (block != WATER) {
                                addFaceToVbo(block, x, y, z, dir, VBO, indices, index);
                            }
                        }
                        else {
                            adj = (*m_globalChunks)[locToKey(x, (z+off))]->getBlockFromChunk(x, y, (z+off));
                            if ((adj == EMPTY) || ((adj == WATER) && (block != WATER))) {
                                glm::vec4 dir = float(off)*glm::vec4(0, 0, 1, 0);
                                addFaceToVbo(block, x, y, z, dir, VBO, indices, index);
                            }
                        }
                    }
                }
            }
        }
    }


    // Buffer opaque data
    if (chunk_idx_opaque.size() > 0) {
        countOp = chunk_idx_opaque.size();

        // Create a VBO on our GPU and store its handle in bufIdx
        generateOpIdx();
        // Tell OpenGL that we want to perform subsequent operations on the VBO referred to by bufIdx
        // and that it will be treated as an element array buffer (since it will contain triangle indices)
        context->glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, bufIdxOp);
        // Pass the data stored in cyl_idx into the bound buffer, reading a number of bytes equal to
        // SPH_IDX_COUNT multiplied by the size of a GLuint. This data is sent to the GPU to be read by shader programs.
        context->glBufferData(GL_ELEMENT_ARRAY_BUFFER, countOp * sizeof(GLuint), chunk_idx_opaque.data(), GL_STATIC_DRAW);

        // The next few sets of function calls are basically the same as above, except bufPos and bufNor are
        // array buffers rather than element array buffers, as they store vertex attributes like position.

        generateOpInter();
        context->glBindBuffer(GL_ARRAY_BUFFER, bufInterOp);
        context->glBufferData(GL_ARRAY_BUFFER, interleaveOpaque.size() * sizeof(vboStruct), interleaveOpaque.data(), GL_STATIC_DRAW);
    }

    // Buffer transparent data
    if (chunk_idx_trans.size() > 0) {
        countTr = chunk_idx_trans.size();

        // Create a VBO on our GPU and store its handle in bufIdx
        generateTrIdx();
        // Tell OpenGL that we want to perform subsequent operations on the VBO referred to by bufIdx
        // and that it will be treated as an element array buffer (since it will contain triangle indices)
        context->glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, bufIdxTr);
        // Pass the data stored in cyl_idx into the bound buffer, reading a number of bytes equal to
        // SPH_IDX_COUNT multiplied by the size of a GLuint. This data is sent to the GPU to be read by shader programs.
        context->glBufferData(GL_ELEMENT_ARRAY_BUFFER, countTr * sizeof(GLuint), chunk_idx_trans.data(), GL_STATIC_DRAW);

        // The next few sets of function calls are basically the same as above, except bufPos and bufNor are
        // array buffers rather than element array buffers, as they store vertex attributes like position.

        generateTrInter();
        context->glBindBuffer(GL_ARRAY_BUFFER, bufInterTr);
        context->glBufferData(GL_ARRAY_BUFFER, interleaveTrans.size() * sizeof(vboStruct), interleaveTrans.data(), GL_STATIC_DRAW);
    }

    // Stop mygl from repopulating the VBO until some data has changed for efficiency
    hasChanged = false;
}

void Chunk::addFaceToVbo(BlockType t, int x, int y, int z, glm::vec4 dir, std::vector<vboStruct> *interleaveVBO, std::vector<GLuint> *chunk_idx, int *idx) {

    glm::vec4 color;
    glm::vec2 startUV; // bottom left corner of texture square
    glm::vec2 animCos; // a vec2 that stores the animation flag (1 for animated) and the cosine power for a given block

    // populate color, uv, animation, and cosine power for a given block
    switch(t)
    {
    case DIRT:
        color = glm::vec4(121.f, 85.f, 58.f, 255.f) / 255.f;
        startUV = glm::vec2(2/16.f, 15/16.f);
        animCos[0] = 0;  // Animate if 1
        animCos[1] = 10;  // Cosine Power
        break;
    case GRASS:
        color = glm::vec4(95.f, 159.f, 53.f, 255.f) / 255.f;

        // grass has a different texture for each face so use the direction of the normal to determine
        // which texture block to sample
        // top face
        if (dir[1] == 1) {
            startUV = glm::vec2(8/16.f, 13/16.f);
            animCos[1] = 10;
        }
        // bottom face
        else if (dir[1] == -1) {
            startUV = glm::vec2(2/16.f, 15/16.f);
            animCos[1] = 10;
        }
        // side face
        else {
            startUV = glm::vec2(3/16.f, 15/16.f);
            animCos[1] = 10;
        }
        animCos[0] = 0;  // Animate if 1
        break;
    case STONE:
        color = glm::vec4(0.5f);
        startUV = glm::vec2(1/16.f, 15/16.f);
        animCos[0] = 0;  // Animate if 1
        animCos[1] = 5;  // Cosine Power
        break;
    case WOOD:
        color = glm::vec4(0.5f);
        startUV = glm::vec2(4/16.f, 14/16.f); // THIS NEEDS TO CHANGE BASED ON FACE
        animCos[0] = 0;  // Animate if 1
        animCos[1] = 10;  // Cosine Power
        break;
    case LEAF:
        color = glm::vec4(0.5f);
        startUV = glm::vec2(5/16.f, 12/16.f);
        animCos[0] = 0;  // Animate if 1
        animCos[1] = 15;  // Cosine Power
        break;
    case BEDROCK:
        color = glm::vec4(0.5f);
        startUV = glm::vec2(1/16.f, 14/16.f);
        animCos[0] = 0;  // Animate if 1
        animCos[1] = 5;  // Cosine Power
        break;
    case LAVA:
        color = glm::vec4(10);
        startUV = glm::vec2(14/16.f, 0.5/16.f);
        animCos[0] = 1;  // Animate if 1
        animCos[1] = 4;  // Cosine Power
        break;
    case WATER:
        color = glm::vec4(0.5f);
        startUV = glm::vec2(14/16.f, 2.5/16.f);
        animCos[0] = 1;  // Animate if 1
        animCos[1] = 100;  // Cosine Power
        break;
    case ICE:
        color = glm::vec4(0.5f);
        startUV = glm::vec2(3/16.f, 11/16.f);
        animCos[0] = 0;  // Animate if 1
        animCos[1] = 2;  // Cosine Power
        break;
    case NPChar:
        color = glm::vec4(95.f, 159.f, 53.f, 255.f) / 255.f;

        // grass has a different texture for each face so use the direction of the normal to determine
        // which texture block to sample
        // top face
        if (dir[1] == 1) {
            startUV = glm::vec2(6/16.f, 9/16.f);
            animCos[1] = 10;
        }
        // bottom face
        else if (dir[1] == -1) {
            startUV = glm::vec2(6/16.f, 8/16.f);
            animCos[1] = 10;
        }
        // side face
        else{
            float random = rand() % 100;
            if(random < 10) {
                startUV = glm::vec2(7/16.f, 8/16.f);
                animCos[1] = 10;
            }
            if(random >= 10) { //lighted face w/ 50% probability
            startUV = glm::vec2(8/16.f, 8/16.f);
            animCos[1] = 10;
            }
        }
        animCos[0] = 0;  // Animate if 1
    }

    // build VBO
    glm::vec4 nor = dir;
    nor[3] = 1;     // the direction passed in has z coordinate of 0... should be 1 for mygl to work properly
    glm::vec4 startPos = glm::vec4(x, y, z, 0.0);
    vboStruct data;

    // determine direction to add face and populate interleaved vbo with position, normal, color, uv, animation flag, and cosine power
    // along x direction
    if (glm::abs(dir[0]) == 1){

        data = setVboData(glm::vec4(0.0, -0.5, -0.5, 1.0) + 0.5f*dir + startPos, nor, color, startUV + glm::vec2(1/16.f, 0), animCos);
        interleaveVBO->push_back(data);

        data = setVboData(glm::vec4(0.0, 0.5, -0.5, 1.0) + 0.5f*dir + startPos, nor, color, startUV + glm::vec2(1/16.f, 1/16.f), animCos);
        interleaveVBO->push_back(data);

        data = setVboData(glm::vec4(0.0, 0.5, 0.5, 1.0) + 0.5f*dir + startPos, nor, color, startUV + glm::vec2(0, 1/16.f), animCos);
        interleaveVBO->push_back(data);

        data = setVboData(glm::vec4(0.0, -0.5, 0.5, 1.0) + 0.5f*dir + startPos, nor, color, startUV, animCos);
        interleaveVBO->push_back(data);

    }

    // along y direction
    if (glm::abs(dir[1]) == 1){

        data = setVboData(glm::vec4(-0.5, 0.0, -0.5, 1.0) + 0.5f*dir + startPos, nor, color, startUV + glm::vec2(0, 1/16.f), animCos);
        interleaveVBO->push_back(data);

        data = setVboData(glm::vec4(0.5, 0.0, -0.5, 1.0) + 0.5f*dir + startPos, nor, color, startUV, animCos);
        interleaveVBO->push_back(data);

        data = setVboData(glm::vec4(0.5, 0.0, 0.5, 1.0) + 0.5f*dir + startPos, nor, color, startUV + glm::vec2(1/16.f, 0), animCos);
        interleaveVBO->push_back(data);

        data = setVboData(glm::vec4(-0.5, 0.0, 0.5, 1.0) + 0.5f*dir + startPos, nor, color, startUV + glm::vec2(1/16.f, 1/16.f), animCos);
        interleaveVBO->push_back(data);

    }

    // along z direction
    if (glm::abs(dir[2]) == 1) {

        data = setVboData(glm::vec4(-0.5, -0.5, 0.0, 1.0) + 0.5f*dir + startPos, nor, color, startUV, animCos);
        interleaveVBO->push_back(data);

        data = setVboData(glm::vec4(0.5, -0.5, 0.0, 1.0) + 0.5f*dir + startPos, nor, color, startUV + glm::vec2(1/16.f, 0), animCos);
        interleaveVBO->push_back(data);

        data = setVboData(glm::vec4(0.5, 0.5, 0.0, 1.0) + 0.5f*dir + startPos, nor, color, startUV + glm::vec2(1/16.f, 1/16.f), animCos);
        interleaveVBO->push_back(data);

        data = setVboData(glm::vec4(-0.5, 0.5, 0.0, 1.0) + 0.5f*dir + startPos, nor, color, startUV + glm::vec2(0, 1/16.f), animCos);
        interleaveVBO->push_back(data);

    }

    // Define indices
    for (int i = 0; i < 2; i++) {
        chunk_idx->push_back(*idx);
        chunk_idx->push_back(*idx + i + 1);
        chunk_idx->push_back(*idx + i + 2);
    }
    *idx = *idx + 4;

}

// Helper function used to set the VBO data into a custom struck that is used to build the interleaved VBO
Chunk::vboStruct Chunk::setVboData(glm::vec4 pos, glm::vec4 nor, glm::vec4 col, glm::vec2 uv, glm::vec2 animCos) {

    vboStruct output;
    output.pos = pos;
    output.nor = nor;
    output.col = col;
    output.uv = uv;
    output.flagCos = animCos;
    return output;

}

int64_t Chunk::locToKey(int x, int z) {
    // x and z are in world coordinates

    int xCorner = floor(x/16.0);
    int zCorner = floor(z/16.0);

    return ((int64_t)xCorner << 32) | zCorner;
}
