#include "npc.h"
NPC::NPC(Terrain* terr, glm::vec3 position) : terrain(terr), pos(position), nearPlayer(false){}

void NPC::updatePos(glm::vec3 goalPos) {

    //Calculate the vector to the goalPosition from the current position
    glm::vec3 vector = goalPos - pos;
    float distance = glm::length(vector);

    //step in the direction of the vector in (distance) # of steps
    if(abs(distance) < 10) {
        nearPlayer = true;
    }

    if(abs(distance) > 1) {

        terrain->setBlockAt(pos.x,pos.y,pos.z, EMPTY);

        glm::vec3 delta = vector/distance;
        pos = pos + delta;

        pos.x = (int) round(pos.x);
        pos.y = (int) round(pos.y);
        pos.z = (int) round(pos.z);

        //If the block ahead is not empty, then try going up one spot
        if(terrain->getBlockAt(pos.x,pos.y - 1,pos.z) != EMPTY) {
            pos.y += 1;
        }

        terrain->setBlockAt(pos.x,pos.y,pos.z, NPChar);

    }
}

void NPC::enforceGravity() {
    //Subject the character to gravity by checking if they're on the ground
    while(terrain->getBlockAt(pos.x,pos.y - 1,pos.z) == EMPTY) {
        terrain->setBlockAt(pos.x,pos.y,pos.z, EMPTY);
        pos.y -= 1;
        terrain->setBlockAt(pos.x,pos.y,pos.z, NPChar);
    }
}
