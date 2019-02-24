#pragma once
#include <la.h>
#include "scene/terrain.h"

class NPC {
public:
    glm::vec3 pos;
    NPC(Terrain* terr, glm::vec3 position);
    Terrain* terrain;

    bool nearPlayer;

    void updatePos(glm::vec3 goalPos);
    void enforceGravity();
};
