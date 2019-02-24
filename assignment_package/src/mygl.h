#ifndef MYGL_H
#define MYGL_H

#include <openglcontext.h>
#include <utils.h>
#include <shaderprogram.h>
#include <scene/cube.h>
#include <scene/worldaxes.h>
#include "camera.h"
#include <scene/terrain.h>
#include <scene/quad.h>
#include <scene/crosshair.h>
#include "texture.h"
#include <QOpenGLVertexArrayObject>
#include <QOpenGLShaderProgram>
#include "player.h"
#include <QDateTime>
#include <QThreadPool>
#include "scene/worker.h"
#include "overlay.h"
#include "npc.h"
#include "scene/quad.h"
#include "postprocessshader.h"
#include <queue>
#include <QSound>

class MyGL : public OpenGLContext
{
    Q_OBJECT
private:
    Chunk* mp_geomCube;// The instance of a unit cube we can use to render any cube. Should NOT be used in final version of your project.
    WorldAxes* mp_worldAxes; // A wireframe representation of the world axes. It is hard-coded to sit centered at (32, 128, 32).
    ShaderProgram* mp_progLambert;// A shader program that uses lambertian reflection
    ShaderProgram* mp_progFlat;// A shader program that uses "flat" reflection (no shadowing at all)
    Crosshair* mp_geomCrosshair; // Instance of crosshair which stays static in middle of screen

    // The screen-space quadrangle used to draw
    // the scene with the post-process shaders.
    Quad m_geomQuad;

    PostProcessShader* m_postProcessNoop;
    PostProcessShader* m_postProcessUnderwater;
    PostProcessShader* m_postProcessUnderlava;

    // A collection of handles to the five frame buffers we've given
    // ourselves to perform render passes. The 0th frame buffer is always
    // written to by the render pass that uses the currently bound surface shader.
    GLuint m_frameBuffer;
    // A collection of handles to the textures used by the frame buffers.
    // m_frameBuffers[i] writes to m_renderedTextures[i].
    GLuint m_renderedTexture;
    // A collection of handles to the depth buffers used by our frame buffers.
    // m_frameBuffers[i] writes to m_depthRenderBuffers[i].
    GLuint m_depthRenderBuffer;

    Overlay* mp_geomOverlay;// Instance of the overlay when in the water or lava

    // For the sky
    ShaderProgram*  mp_progSky; // A screen-space shader for creating the sky background
    Quad* mp_geomQuad;

    GLuint vao; // A handle for our vertex array object. This will store the VBOs created in our geometry classes.
                // Don't worry too much about this. Just know it is necessary in order to render geometry.

    Camera* mp_camera;
    Terrain* mp_terrain;
    Texture* mp_texture;

    Player* main_player;
    std::vector<NPC*> npcs;

    int lastCharUpdate;
    int lastSoundUpdate;

    int m_time;
    int m_blockIdx;

    std::vector<glm::vec2> linear_river_positions; // turtle positions of river
    std::vector<glm::vec2> delta_river_positions;

    /// Timer linked to timerUpdate(). Fires approx. 60 times per second
    QTimer timer;

    int64_t lastCall; //The time that the last update function was called at
    bool firstCalled; //boolean saying the update function has been called the first time
    int64_t delta; //the change in time between timer update calls

    QHash<int, BlockType> setBlockType;  // Used to iterate through blocktypes when setblocks to show off various textures

    void MoveMouseToCenter(); // Forces the mouse position to the screen's center. You should call this
                              // from within a mouse move event after reading the mouse movement so that
                              // your mouse stays within the screen bounds and is always read.

    QThreadPool *threadPool;
    QMutex mutex;

public:
    explicit MyGL(QWidget *parent = 0);
    ~MyGL();

    void initializeGL();
    void resizeGL(int w, int h);
    void paintGL();

    void GLDrawScene();

    // Sets up the arrays of frame buffers
    // used to store render passes. Invoked
    // once in initializeGL().
    void createRenderBuffers();

    // A helper function that iterates through
    // each of the render passes required by the
    // currently bound post-process shader and
    // invokes them.
    void performPostprocessRenderPass();

    QSound footsteps;
    QSound water;
    QSound creepy;
    QSound wind;

protected:
    void mousePressEvent(QMouseEvent *e);
    void keyPressEvent(QKeyEvent *e);
    void keyReleaseEvent(QKeyEvent *e);
    void mouseMoveEvent(QMouseEvent *e);
    void mouseReleaseEvent(QMouseEvent *e);
    void removeBlock();
    void addBlock();
    void bfs(glm::ivec3 start); // used to find path to expand water for fluid simulation
    std::vector<glm::ivec3> getNeighbors(glm::ivec3 node);

private slots:
    /// Slot that gets called ~60 times per second
    void timerUpdate();
};


#endif // MYGL_H
