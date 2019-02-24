#include "worker.h"

Worker::Worker(QMutex* mutex_p, Terrain* terr, int one, int two, int three, int four, std::vector<glm::vec2> lpos, std::vector<glm::vec2> dpos) : mutex(mutex_p), terrain(terr), first(one), second(two), third(three), fourth(four), LRiverPos(lpos), DRiverPos(dpos)  {}

void Worker::run() {
    mutex->lock();

    terrain->createScene(first, second, third, fourth);

    // Need to re-render the river since new terrain was added and that terrain needs to be carved out
    terrain->renderLinearRiver(LRiverPos);
    terrain->renderDeltaRiver(DRiverPos);

    mutex->unlock();
}
