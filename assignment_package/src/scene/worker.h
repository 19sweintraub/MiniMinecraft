#pragma once
#include <QRunnable>
#include <QMutex>
#include <iostream>
#include "terrain.h"

class Worker : public QRunnable
{
public:
    Worker(QMutex* mutex_p, Terrain* terr, int one, int two, int three, int four, std::vector<glm::vec2> lpos, std::vector<glm::vec2> dpos);

    void run() override;

    int first;
    int second;
    int third;
    int fourth;

    std::vector<glm::vec2> LRiverPos;
    std::vector<glm::vec2> DRiverPos;

    QMutex* mutex;
    Terrain* terrain;
};
