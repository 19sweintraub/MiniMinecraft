#include "mygl.h"
#include <la.h>
#include <scene/crosshair.h>
#include <iostream>
#include <QApplication>
#include <QKeyEvent>


MyGL::MyGL(QWidget *parent)
    : OpenGLContext(parent),
      m_geomQuad(this), m_postProcessNoop(new PostProcessShader(this)),
      m_postProcessUnderwater(new PostProcessShader(this)),
      m_postProcessUnderlava(new PostProcessShader(this)),
      mp_geomCube(new Chunk(this)), mp_worldAxes(new WorldAxes(this)),
      mp_geomCrosshair(new Crosshair(this)), mp_geomOverlay(new Overlay(this)),
      mp_progLambert(new ShaderProgram(this)), mp_progFlat(new ShaderProgram(this)),
      mp_progSky(new ShaderProgram(this)), mp_geomQuad(new Quad(this)),
      mp_camera(new Camera()), mp_terrain(new Terrain(this)), firstCalled(false),
      mp_texture(new Texture(this)),
      m_time(0.f),
      m_frameBuffer(-1),
      m_renderedTexture(-1),
      m_depthRenderBuffer(-1),
      m_blockIdx(0), footsteps(":/sounds/footsteps.wav"), water(":/sounds/river.wav"),
      creepy(":/sounds/creepy.wav"), wind(":/sounds/wind.wav")
{
    threadPool = QThreadPool::globalInstance(); //create the QThreadPool global instance
    // Connect the timer to a function so that when the timer ticks the function is executed
    connect(&timer, SIGNAL(timeout()), this, SLOT(timerUpdate()));
    // Tell the timer to redraw 60 times per second
    timer.start(16);
    setFocusPolicy(Qt::ClickFocus);

    // setMouseTracking(true); // MyGL will track the mouse's movements even if a mouse button is not pressed
    setCursor(Qt::BlankCursor); // Make the cursor invisible
    main_player = new Player(mp_camera);

    // Iterate through the keys when setting blocks to change the block type
    setBlockType.insert(0, GRASS);
    setBlockType.insert(1, DIRT);
    setBlockType.insert(2, STONE);
    setBlockType.insert(3, WOOD);
    setBlockType.insert(4, LEAF);
    setBlockType.insert(5, BEDROCK);
    setBlockType.insert(6, LAVA);
    setBlockType.insert(7, WATER);
    setBlockType.insert(8, ICE);

    //Create the NPCs
    NPC* pumpkin1 = new NPC(mp_terrain, glm::vec3(3210.0,140.0,3210.0));
    NPC* pumpkin2 = new NPC(mp_terrain, glm::vec3(3205.0,140.0,3205.0));
    NPC* pumpkin3 = new NPC(mp_terrain, glm::vec3(3190.0,140.0,3205.0));
    NPC* pumpkin4 = new NPC(mp_terrain, glm::vec3(3205.0,140.0,3200.0));

    NPC* pumpkin5 = new NPC(mp_terrain, glm::vec3(3230.0,140.0,3230.0));
    NPC* pumpkin6 = new NPC(mp_terrain, glm::vec3(3225.0,140.0,3225.0));
    NPC* pumpkin7 = new NPC(mp_terrain, glm::vec3(3220.0,140.0,3225.0));
    NPC* pumpkin8 = new NPC(mp_terrain, glm::vec3(3225.0,140.0,3220.0));

    npcs.push_back(pumpkin1);
    npcs.push_back(pumpkin2);
    npcs.push_back(pumpkin3);
    npcs.push_back(pumpkin4);

    npcs.push_back(pumpkin5);
    npcs.push_back(pumpkin6);
    npcs.push_back(pumpkin7);
    npcs.push_back(pumpkin8);
}

MyGL::~MyGL()
{
    makeCurrent();
    glDeleteVertexArrays(1, &vao);
    mp_geomCube->destroy();
    //m_geomQuad.destroy();

    delete mp_geomCube;
    delete mp_worldAxes;
    delete mp_progLambert;
    delete mp_progFlat;
    delete mp_camera;
    delete mp_terrain;
    delete main_player;
    delete mp_geomCrosshair;
    delete mp_geomOverlay;

}

void MyGL::MoveMouseToCenter()
{
    QCursor::setPos(this->mapToGlobal(QPoint(width() / 2, height() / 2)));
}

void MyGL::initializeGL()
{
    // Create an OpenGL context using Qt's QOpenGLFunctions_3_2_Core class
    // If you were programming in a non-Qt context you might use GLEW (GL Extension Wrangler)instead
    initializeOpenGLFunctions();
    // Print out some information about the current OpenGL context
    debugContextVersion();

    // Set a few settings/modes in OpenGL rendering
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);
    glEnable(GL_LINE_SMOOTH);
    glEnable(GL_POLYGON_SMOOTH);
    glHint(GL_LINE_SMOOTH_HINT, GL_NICEST);
    glHint(GL_POLYGON_SMOOTH_HINT, GL_NICEST);
    // Set the size with which points should be rendered
    glPointSize(5);
    // Set the color with which the screen is filled at the start of each render call.
    // Chris: This is what fills the screen with light blue.
    glClearColor(0.37f, 0.74f, 1.0f, 1);

    printGLErrorLog();

    // Create a Vertex Attribute Object
    glGenVertexArrays(1, &vao);

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    createRenderBuffers();

    m_geomQuad.create();

    //Create the instance of Cube
    // mp_geomCube->create();
    mp_worldAxes->create();

    mp_geomCrosshair->setScreenWidthAndHeight(width(), height());
    mp_geomCrosshair->create(); // Chris added this

    // Create and set up the diffuse shader
    mp_progLambert->create(":/glsl/lambert.vert.glsl", ":/glsl/lambert.frag.glsl");
    // Create and set up the flat lighting shader
    mp_progFlat->create(":/glsl/flat.vert.glsl", ":/glsl/flat.frag.glsl");

    // Create and set up post process shader
    m_postProcessNoop->create(":/glsl/passthrough.vert.glsl", ":/glsl/noop.frag.glsl");
    m_postProcessUnderwater->create(":/glsl/passthrough.vert.glsl", ":/glsl/underwater.frag.glsl");
    m_postProcessUnderlava->create(":/glsl/passthrough.vert.glsl", ":/glsl/underlava.frag.glsl");

    // Chris added for the sky
    mp_progSky->create(":/glsl/sky.vert.glsl", ":/glsl/sky.frag.glsl");

    // Set a color with which to draw geometry since you won't have one
    // defined until you implement the Node classes.
    // This makes your geometry render green.
    mp_progLambert->setGeometryColor(glm::vec4(0,1,0,1));


    mp_texture->create(":/textures/minecraft_textures_all.png");
    mp_texture->load(0);


    // We have to have a VAO bound in OpenGL 3.2 Core. But if we're not
    // using multiple VAOs, we can just bind one once.
    //    vao.bind();
    glBindVertexArray(vao);

    // Initially create scene with x_min = -32, x_max = 32,
    int x_origin = 3200;
    int z_origin = 3200;
    int x_min = x_origin - 32;
    int x_max = x_origin + 32;
    int z_min = z_origin - 32;
    int z_max = z_origin + 32;
    this->linear_river_positions = mp_terrain->getLinearRiverPositions();
    this->delta_river_positions = mp_terrain->getDeltaRiverPositions();
    mp_terrain->createScene(x_min, x_max, z_min, z_max);
    mp_terrain->renderLinearRiver(this->linear_river_positions);
    mp_terrain->renderDeltaRiver(this->delta_river_positions);
}

void MyGL::resizeGL(int w, int h)
{
    //This code sets the concatenated view and perspective projection matrices used for
    //our scene's camera view.
    //    *mp_camera = Camera(w, h, glm::vec3(mp_terrain->dimensions.x, mp_terrain->dimensions.y * 0.75, mp_terrain->dimensions.z),
    //                       glm::vec3(mp_terrain->dimensions.x / 2, mp_terrain->dimensions.y / 2, mp_terrain->dimensions.z / 2), glm::vec3(0,1,0));
    // Chris updated this initial camera code so camera starts off in the world.
    // Helpful for debugging ray casting to remove and add blocks

    // Also it's helpful to start on the side where we can see more shadows and hence can see blocks more clearly.
//    glm::vec3 camera_pos = glm::vec3(3200, 140, 3200);
    glm::vec3 camera_pos = glm::vec3(3200, 140, 3200);
    glm::vec3 lookat_pos = camera_pos + glm::normalize(glm::vec3(0, 0, 1));
    *mp_camera = Camera(w, h, camera_pos, lookat_pos, glm::vec3(0, 1, 0));
    glm::mat4 viewproj = mp_camera->getViewProj();

    // Upload the view-projection matrix to our shaders (i.e. onto the graphics card)

    mp_progLambert->setViewProjMatrix(viewproj);
    mp_progFlat->setViewProjMatrix(viewproj);
    m_postProcessNoop->setDimensions(glm::ivec2(w, h));
    m_postProcessUnderwater->setDimensions(glm::ivec2(w, h));
    m_postProcessUnderlava->setDimensions(glm::ivec2(w, h));


    // Chris added for the sky
    // TODO: Chris needs to comment these for the Milestone 3
    mp_progSky->setViewProjMatrix(glm::inverse(viewproj));
//    mp_progSky->useMe();
    this->glUniform2i(mp_progSky->unifDimensions, width() * this->devicePixelRatio(), height() * this->devicePixelRatio());
//    this->glUniform3f(mp_progSky->unifEye, mp_camera->eye.x, mp_camera->eye.y, mp_camera->eye.z);

    printGLErrorLog();
}

// MyGL's constructor links timerUpdate() to a timer that fires 60 times per second.
// We're treating MyGL as our game engine class, so we're going to use timerUpdate
void MyGL::timerUpdate()
{
    //Determine time since the last time update was called
    if(firstCalled == false) {
        lastCall = QDateTime::currentMSecsSinceEpoch();
        firstCalled = true;
    }

    delta = QDateTime::currentMSecsSinceEpoch() - lastCall;
    lastCall = QDateTime::currentMSecsSinceEpoch();

    //update the velocity of the main player
    main_player->updateVelocity();

    mp_camera->TranslateAlongLook(main_player->vel[1] * delta);
    mp_camera->TranslateAlongRight(main_player->vel[0] * delta);
    mp_camera->TranslateAlongUp(main_player->vel[2] * delta);

    //update the player's position
    main_player->pos = mp_camera->eye;
    std::vector<glm::vec3> pointsOnPlayer;

    //iterate over the Y coordinates to create 3 levels of points
    for(double i = -1.25; i <= 1; i++) {
       glm::vec3 topRight = main_player->pos + glm::vec3(0.6,i,0.6);
       glm::vec3 topLeft = main_player->pos + glm::vec3(-0.6,i,0.6);
       glm::vec3 bottomRight = main_player->pos + glm::vec3(0.6,i,-0.6);
       glm::vec3 bottomLeft = main_player->pos + glm::vec3(-0.6,i,-0.6);

       pointsOnPlayer.push_back(topRight);
       pointsOnPlayer.push_back(topLeft);
       pointsOnPlayer.push_back(bottomRight);
       pointsOnPlayer.push_back(bottomLeft);
    }

    bool collision = false;

    //By default, assume we are not swimming or in lava.
        main_player->swimming = false;
        main_player->inLava = false;
        mp_geomOverlay->state = 0;

    //Raycast upwards to see if we are swimming or in lava.
    //This is important as a first step so that gravity is turned off, and walking is disabled
    for(double i = 1; i < 6; i ++) {

        BlockType bt = mp_terrain->getBlockAt(int(pointsOnPlayer[11].x),int(pointsOnPlayer[11].y + i),int(pointsOnPlayer[11].z));

        if( bt == WATER) {
           main_player->swimming = true;
           mp_geomOverlay->state = 1;
        }

        if( bt == LAVA) {
           main_player->inLava = true;
           mp_geomOverlay->state = 2;
        }
    }

    //If we are swimming or in lava or flying, then we are not walking
    main_player->cam->walking = true;

    if(main_player->swimming || main_player->inLava || main_player->flying) {
        main_player->cam->walking = false;
    }


    //for each of the points except for the, check if there is a collision with the block
    for(int i = 4; i < 12; i++) {
        BlockType colliding = mp_terrain->getBlockAt(int(pointsOnPlayer[i].x),int(pointsOnPlayer[i].y),int(pointsOnPlayer[i].z));
        if( colliding != EMPTY) {
            collision = true;

            if(colliding = NPChar) {
                mp_geomOverlay->state = 2;
            }
        }
    }

    if(collision && !main_player->flying) {
        //undo the translation
        mp_camera->TranslateAlongLook(-1 * main_player->vel[1] * delta);
        mp_camera->TranslateAlongRight(-1 * main_player->vel[0] * delta);
        mp_camera->TranslateAlongUp(-1 * main_player->vel[2] * delta);
    }

    //If not flying, the player should fall onto the ground and stay on the ground
    int pointsOnGround = 0;

    if(!main_player->flying && !main_player->swimming && !main_player->inLava) {
        //doing ground checking my making the velocity negative until 4 bottom points of the obstacle collide with something
        for(int i = 0; i < 4; i++) {
            BlockType bt = mp_terrain->getBlockAt(int(pointsOnPlayer[i].x),int(pointsOnPlayer[i].y - 1),int(pointsOnPlayer[i].z));
            if(bt != EMPTY && bt != WATER && bt != LAVA) {
                pointsOnGround += 1;
            }
        }

        //if there are less than 4 points on the ground and we're not swimming or in lava, then gravity applies
        if (pointsOnGround < 4) {
            main_player->vel[2] -= 0.001;
            mp_camera->eye[1] += main_player->vel[2] * delta;
        }

        //else, the player is sufficiently on the ground
        else {
            main_player->vel[2] = 0;
        }
    }

    else {
        main_player->vel[2] = 0;
    }

  //  Chris commented this out so easier to debug
    // MoveMouseToCenter();

    //update the player's position
    main_player->pos = mp_camera->eye;

   //For every NPC in the game, calculate their new position, and draw the NPC at a fixed frequency
   if (m_time - lastCharUpdate > 25) {
       if(main_player->followMe) {
            for(NPC* character : npcs) {
                lastCharUpdate = m_time;
                character->updatePos(main_player->pos);
                if(character->nearPlayer == true) {
                        if(creepy.isFinished()) {  creepy.play(); }
                }
            }
       }
    }

   if (m_time - lastSoundUpdate > 100) {
       lastSoundUpdate = m_time;
       if(main_player->cam->walking) {
           wind.stop();
           if(main_player->w_pressed || main_player->a_pressed || main_player->s_pressed || main_player->d_pressed) {
               if(footsteps.isFinished()) {  footsteps.play(); }
           }
       }
       if(main_player->flying) {
           footsteps.stop();
           if(wind.isFinished()) { wind.play(); }
       }
       if(mp_geomOverlay->state == 1) {
           footsteps.stop();
           wind.stop();
           if(water.isFinished()) { water.play(); }
       }
   }

   for(NPC* character : npcs) {
       character->enforceGravity();
   }

    //Each NPC should pursue a path to the beacon object and only if their path is immediately restricted should they invoke the path finder

    update();
}

// This function is called whenever update() is called.
// MyGL's constructor links update() to a timer that fires 60 times per second,
// so paintGL() called at a rate of 60 frames per second.
void MyGL::paintGL()
{
    int NEAR_EDGE_OFFSET = 5;

    // Clear the screen so that we only see newly drawn images
   // glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    mp_progLambert->setTime(m_time);
    m_postProcessNoop->setTime(m_time);
    m_postProcessUnderwater->setTime(m_time);
    m_postProcessUnderlava->setTime(m_time);
    mp_progSky->setTime(m_time);

//    // Chris added this for sky
//    glm::mat4 viewproj = mp_camera->getViewProj();
//    mp_progFlat->setViewProjMatrix(viewproj);
//    mp_progLambert->setViewProjMatrix(viewproj);
//    mp_progLambert->setTime(m_time);

//    // Chris added for the sky
//    mp_progSky->setViewProjMatrix(glm::inverse(viewproj));
//    mp_progSky->useMe();
//    // The eye matrix gets updated each frame I think
//    this->glUniform3f(mp_progSky->unifEye, mp_camera->eye.x, mp_camera->eye.y, mp_camera->eye.z);
//    // unifTime (u_Time in glsl) updates every 15ms, so around 60fps.
////    this->glUniform3f(mp_progSky->unifSunPos) // TODO: Set the sun uniform position vec3
//    mp_progSky->setTime(m_time);
////    this->glUniform1f(mp_progSky->unifTime, m_time);
//    mp_progSky->draw(*mp_geomQuad);

    m_time++;

    // if player world x is within 5 blocks of terrain's max_x, createScene(max_x, max_x + 16, min_z, max_z)
    glm::vec3 player_pos = main_player->pos;
    if (player_pos.x > mp_terrain->max_x - NEAR_EDGE_OFFSET) {

        //Create a new worker object
        Worker* newThread = new Worker(&mutex, mp_terrain, mp_terrain->max_x, mp_terrain->max_x + 16, mp_terrain->min_z, mp_terrain->max_z, this->linear_river_positions, this->delta_river_positions);
        //Run this thread
        threadPool->start(newThread);
    }
    // if player is near min_x
    if (player_pos.x < mp_terrain->min_x + NEAR_EDGE_OFFSET) {

        //Create a new worker object
        Worker* newThread = new Worker(&mutex, mp_terrain, mp_terrain->min_x - 16, mp_terrain->min_x, mp_terrain->min_z, mp_terrain->max_z, this->linear_river_positions, this->delta_river_positions);
        threadPool->start(newThread);

    }
    // if player is near max_z
    if (player_pos.z > mp_terrain->max_z - NEAR_EDGE_OFFSET) {

       Worker* newThread = new Worker(&mutex, mp_terrain, mp_terrain->min_x, mp_terrain->max_x, mp_terrain->max_z, mp_terrain->max_z + 16, this->linear_river_positions, this->delta_river_positions);
       threadPool->start(newThread);
    }

    // if player is near min_z
    if (player_pos.z < mp_terrain->min_z + NEAR_EDGE_OFFSET) {

       Worker* newThread = new Worker(&mutex, mp_terrain, mp_terrain->min_x, mp_terrain->max_x, mp_terrain->min_z - 16, mp_terrain->min_z, this->linear_river_positions, this->delta_river_positions);
       threadPool->start(newThread);
    }

    GLDrawScene();

//    if(main_player->swimming) {
//        mp_geomOverlay->create();
//        mp_progFlat->draw(*mp_geomOverlay, 0);
//    }

    performPostprocessRenderPass();

    // Draw cross hairs
    glDisable(GL_DEPTH_TEST);
    mp_progFlat->setViewProjMatrix(glm::mat4(1));
    mp_progFlat->setModelMatrix(glm::mat4(1));
    mp_progFlat->draw(*mp_geomCrosshair, 0);
    glEnable(GL_DEPTH_TEST);

}

void MyGL::GLDrawScene()
{

    // Render to our framebuffer rather than the viewport
    glBindFramebuffer(GL_FRAMEBUFFER, m_frameBuffer);
    // Render on the whole framebuffer, complete from the lower left corner to the upper right
    glViewport(0,0,this->width() * this->devicePixelRatio(), this->height() * this->devicePixelRatio());
    // Clear the screen so that we only see newly drawn images
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

//    // Chris added this for sky
    glm::mat4 viewproj = mp_camera->getViewProj();
    mp_progSky->setViewProjMatrix(glm::inverse(viewproj));
    // The eye matrix gets updated each frame I think
    this->glUniform3f(mp_progSky->unifEye, mp_camera->eye.x, mp_camera->eye.y, mp_camera->eye.z);
    // unifTime (u_Time in glsl) updates every 15ms, so around 60fps.
     // End of code Chris added for sky

    mp_progFlat->setViewProjMatrix(viewproj);
    mp_progLambert->setViewProjMatrix(viewproj);

    mp_progSky->draw(m_geomQuad);

    // TODO: Need to make sure to update the dimensions of terrain.
    for (int x = mp_terrain->min_x; x < mp_terrain->max_x; x += 16) {
        for (int z = mp_terrain->min_z; z < mp_terrain->max_z; z += 16) {
    // mp_geomCube->destroy();
            int64_t key = mp_terrain->locToKey(x, z);
            // if the QHash contains this key, then render the chunk
            if (mp_terrain->m_chunks->contains(key)) {
                mp_geomCube = (*mp_terrain->m_chunks)[key];
                if (mp_geomCube->hasChanged == true) {
                //     mp_geomCube->destroy();
                     mp_geomCube->create();
                 }

                mp_progLambert->setModelMatrix(glm::translate(glm::mat4(), glm::vec3(x, 0, z)));
                mp_texture->bind(0);
                mp_progLambert->draw(*mp_geomCube, 0);
            }
        }
    }
}

void MyGL::performPostprocessRenderPass()
{
    // Render the frame buffer as a texture on a screen-size quad

    // Tell OpenGL to render to the viewport's frame buffer
    glBindFramebuffer(GL_FRAMEBUFFER, this->defaultFramebufferObject());
    // Render on the whole framebuffer, complete from the lower left corner to the upper right
    glViewport(0,0,this->width() * this->devicePixelRatio(), this->height() * this->devicePixelRatio());
    // Clear the screen
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    // Bind our texture in Texture Unit 0
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, m_renderedTexture);

    if(main_player->swimming) {
        m_postProcessUnderwater->draw(m_geomQuad, 0);
    } else if (main_player->inLava) {
        m_postProcessUnderlava->draw(m_geomQuad, 0);
    } else {
        m_postProcessNoop->draw(m_geomQuad, 0);
    }
}

void MyGL::createRenderBuffers()
{
    // Initialize the frame buffers and render textures
    glGenFramebuffers(1, &m_frameBuffer);
    glGenTextures(1, &m_renderedTexture);
    glGenRenderbuffers(1, &m_depthRenderBuffer);

    glBindFramebuffer(GL_FRAMEBUFFER, m_frameBuffer);
    // Bind our texture so that all functions that deal with textures will interact with this one
    glBindTexture(GL_TEXTURE_2D, m_renderedTexture);
    // Give an empty image to OpenGL ( the last "0" )
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, this->width() * this->devicePixelRatio(), this->height() * this->devicePixelRatio(), 0, GL_RGBA, GL_UNSIGNED_BYTE, (void*)0);

    // Set the render settings for the texture we've just created.
    // Essentially zero filtering on the "texture" so it appears exactly as rendered
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    // Clamp the colors at the edge of our texture
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    // Initialize our depth buffer
    glBindRenderbuffer(GL_RENDERBUFFER, m_depthRenderBuffer);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, this->width() * this->devicePixelRatio(), this->height() * this->devicePixelRatio());
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, m_depthRenderBuffer);

    // Set m_renderedTexture as the color output of our frame buffer
    glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, m_renderedTexture, 0);

    // Sets the color output of the fragment shader to be stored in GL_COLOR_ATTACHMENT0, which we previously set to m_renderedTextures[i]
    GLenum drawBuffers[1] = {GL_COLOR_ATTACHMENT0};
    glDrawBuffers(1, drawBuffers); // "1" is the size of drawBuffers

    if(glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
    {
        std::cout << "Frame buffer did not initialize correctly..." << std::endl;
        printGLErrorLog();
    }
}

// Chris added this for adding and removing blocks
void MyGL::mousePressEvent(QMouseEvent *e) {


    main_player->updateMouseEvent(e,1, width(), height());

    if (e->buttons() == Qt::LeftButton) {
        // Left mouse button -> remove a block from the world
        // Call removeBlock function
        removeBlock();
    } else if (e->buttons() == Qt::RightButton) {
        // Right mouse button -> place a block in the world
        addBlock();
    }
}

void MyGL::mouseReleaseEvent(QMouseEvent *e) {
    main_player->updateMouseEvent(e,0,width(),height());
}

void MyGL::mouseMoveEvent(QMouseEvent *e) {
    main_player->updateMouseEvent(e,2,width(),height()); //2 symbolizes a move
}

void MyGL::keyPressEvent(QKeyEvent *e)
{
    main_player->updateKeyEvent(e,1);

//    float amount = 5.f;
//    if(e->modifiers() & Qt::ShiftModifier){
//        amount = 10.0f;
//    }
    // http://doc.qt.io/qt-5/qt.html#Key-enum
    // This could all be much more efficient if a switch
    // statement were used, but I really dislike their
    // syntax so I chose to be lazy and use a long
    // chain of if statements instead
    if (e->key() == Qt::Key_Escape) {
        QApplication::quit();
    } else if (e->key() == Qt::Key_R) {
        *mp_camera = Camera(this->width(), this->height());
    }
    mp_camera->RecomputeAttributes();
}

void MyGL::keyReleaseEvent(QKeyEvent *e) {
    main_player->updateKeyEvent(e,0);
}

// Helper method which will use ray casting to remove a block
void MyGL::removeBlock() {
    // Will use camera ref, eye, and direction from mp_camera
    // eye = position of the camera in world space
    // look = vector from eye to ref

    int num_iterations = 100;
    float max_distance = 10.0; // 3 units if farthest block that can remove
    float step_amount = max_distance / num_iterations;
    for (int i = 0; i < num_iterations; i++) {
        float t = float(i) * step_amount;
        glm::vec3 ray_point = mp_camera->eye + t * mp_camera->look;

        // I'm not sure why adding 0.5 works, but it does.
        int floor_x = ray_point.x + 0.5;
        int floor_y = ray_point.y + 0.5;
        int floor_z = ray_point.z + 0.5;

        // Check if ray point is inside of a cube, somehow get the index of the cube
        if (mp_terrain->getBlockAt(floor_x, floor_y, floor_z) != EMPTY) {
            // If the block at this raypoint is GRASS, DIRT, or STONE 

            mp_terrain->setBlockAt(floor_x, floor_y, floor_z, EMPTY);

            std::vector<glm::ivec3> toAdd;
            // check if water is adjacent
            if (mp_terrain->getBlockAt(floor_x - 1, floor_y, floor_z) == WATER) {
                bfs(glm::ivec3(floor_x, floor_y, floor_z));
            } else if (mp_terrain->getBlockAt(floor_x + 1, floor_y, floor_z) == WATER) {
                bfs(glm::ivec3(floor_x, floor_y, floor_z));
            } else if (mp_terrain->getBlockAt(floor_x, floor_y - 1, floor_z) == WATER) {
                bfs(glm::ivec3(floor_x, floor_y, floor_z));
            } else if (mp_terrain->getBlockAt(floor_x, floor_y + 1, floor_z) == WATER) {
                bfs(glm::ivec3(floor_x, floor_y , floor_z));
            } else if (mp_terrain->getBlockAt(floor_x, floor_y, floor_z - 1) == WATER) {
                bfs(glm::ivec3(floor_x, floor_y, floor_z));
            } else if (mp_terrain->getBlockAt(floor_x, floor_y, floor_z + 1) == WATER) {
                bfs(glm::ivec3(floor_x, floor_y, floor_z));
            } else if (mp_terrain->getBlockAt(floor_x - 1, floor_y, floor_z) == LAVA) {
                mp_terrain->setBlockAt(floor_x, floor_y, floor_z, LAVA);
            } else if (mp_terrain->getBlockAt(floor_x + 1, floor_y, floor_z) == LAVA) {
                mp_terrain->setBlockAt(floor_x, floor_y, floor_z, LAVA);
            } else if (mp_terrain->getBlockAt(floor_x, floor_y - 1, floor_z) == LAVA) {
                mp_terrain->setBlockAt(floor_x, floor_y, floor_z, LAVA);
            } else if (mp_terrain->getBlockAt(floor_x, floor_y + 1, floor_z) == LAVA) {
                mp_terrain->setBlockAt(floor_x, floor_y, floor_z, LAVA);
            } else if (mp_terrain->getBlockAt(floor_x, floor_y, floor_z - 1) == LAVA) {
                mp_terrain->setBlockAt(floor_x, floor_y, floor_z, LAVA);
            } else if (mp_terrain->getBlockAt(floor_x, floor_y, floor_z + 1) == LAVA) {
                mp_terrain->setBlockAt(floor_x, floor_y, floor_z, LAVA);
            }

            std::cout << "Removed a block" << std::endl;
            return;
        }
    }

    // If code gets to here outside for loop, then no block to remove.
    std::cout << "Did NOT remove a block" << std::endl;
}

// Helper method to add a block on right click
void MyGL::addBlock() {
    // Will use camera ref, eye, and direction from mp_camera
    // eye = position of the camera in world space
    // look = vector from eye to ref

    BlockType block_type = setBlockType.value(m_blockIdx);
    m_blockIdx++;
    if (m_blockIdx == 9) {
        m_blockIdx = 0;
    }

    int num_iterations = 100;
    float max_distance = 10.0; // 3 units if farthest block that can remove
    float step_amount = max_distance / num_iterations;
    for (int i = 0; i < num_iterations; i++) {
        float t = float(i) * step_amount;
        glm::vec3 ray_point = mp_camera->eye + t * mp_camera->look;

        int floor_x = ray_point.x + 0.5;
        int floor_y = ray_point.y + 0.5;
        int floor_z = ray_point.z + 0.5;

        // Check if ray point is inside of a cube, somehow get the index of the cube
        if (mp_terrain->getBlockAt(floor_x, floor_y, floor_z) != EMPTY) {
            // If the block at this raypoint is GRASS, DIRT, or STONE

            // Constructed the model matrix
            glm::mat4 model_matrix = glm::translate(glm::mat4(), glm::vec3(floor_x, floor_y, floor_z));
            // At this point we know the selected block / cube

            // Convert ray march point from world space to object space
            // This involves using the inverse of the model matrix which is 4x4
            // Model matrix has an identity rotation with x y z translations in 4th column
            glm::vec4 ray_point_object_space = glm::inverse(model_matrix) * glm::vec4(ray_point, 1);

            float x = ray_point_object_space[0]; // ray object x
            float y = ray_point_object_space[1];
            float z = ray_point_object_space[2];

            // Find x, y, or z value of point with greatest magnitude.
            // Take the sign of x, y, or z.
            // And know if surface normal is +x or -x through if/else statements.
            if (abs(x) >= abs(y) && abs(x) >= abs(z)) {
                // Right or left face of the cube in object space
                if (x > 0) {
                    // Right face of the cube
                    mp_terrain->setBlockAt(floor_x + 1, floor_y, floor_z, block_type);
                } else {
                    // Left face of the cube
                    mp_terrain->setBlockAt(floor_x - 1, floor_y, floor_z, block_type);
                }
            } else if (abs(y) >= abs(x) && abs(y) >= abs(z)) {
                // Top or bottom face of the cube in object space
                if (y > 0) {
                    // Top face of the cube
                    mp_terrain->setBlockAt(floor_x, floor_y + 1, floor_z, block_type);
                } else {
                    // Bottom face of the cube
                    mp_terrain->setBlockAt(floor_x, floor_y - 1, floor_z, block_type);
                }
            } else if (abs(z) >= abs(x) && abs(z) >= abs(y)) {
                // Front or back face of the cube in object space
                if (z > 0) {
                    // Front face of the cube
                    mp_terrain->setBlockAt(floor_x, floor_y, floor_z + 1, block_type);
                } else {
                    // Back face of the cube
                    mp_terrain->setBlockAt(floor_x, floor_y, floor_z - 1, block_type);
                }
            }
            return;
        }
    }
    // If code gets to here outside for loop, then no block to remove.
}

void MyGL::bfs(glm::ivec3 start) {

    // Hash map would be better but would need to write a custom hashing funciton for glm::ivec3 so searching
    // the vector shoudl suffice for our small search space
    std::vector<glm::ivec3> visited;

    // make queue
    std::queue<glm::ivec3> myQueue;

    // mark as visited
    // visited.insert(start, true);
    visited.push_back(start);

    // Add to queue
    myQueue.push(start);

    bool stop = false;
    int count = 0;

    while (!myQueue.empty() && !stop) {

        // get node from queue
        glm::ivec3 node = myQueue.front();
        myQueue.pop();

        // determine if water block should be placed
        if (mp_terrain->getBlockAt(node[0], node[1], node[2]) == EMPTY) {
            count++;
            // Small and smallerwater block types are used to simulate decreasing volume
            if (count == 7) {
                stop = true;
            }
            mp_terrain->setBlockAt(node[0], node[1], node[2], WATER);
        }

        // std::vector<glm::ivec3> neighbors = getNeighbors(node);
        for (glm::ivec3 neighbor : getNeighbors(node)) {
             if (!(std::find(visited.begin(), visited.end(), neighbor) != visited.end()) && (mp_terrain->getBlockAt(neighbor[0], neighbor[1], neighbor[2]) == EMPTY)) {
                 visited.push_back(neighbor);
                 myQueue.push(neighbor);
             }
        }

    }

}

std::vector<glm::ivec3> MyGL::getNeighbors(glm::ivec3 node) {

    std::vector<glm::ivec3> neighbors;

    // Look at all surrouding blocks except for blocks above as water will not expand upwards
    neighbors.push_back(node + ivec3(1, 0, 0));
    neighbors.push_back(node - ivec3(1, 0, 0));
    neighbors.push_back(node - ivec3(0, 0, 1));
    neighbors.push_back(node + ivec3(0, 0, 1));
    neighbors.push_back(node - ivec3(0, 1, 0));

    return neighbors;

}
