#ifndef DELTALSYSTEM_H
#define DELTALSYSTEM_H


#include <QHash>
#include <scene/terrain.h>
#include <iostream>
class Terrain;

// We can use Rule in place of the wordy void *(void)
// The expansion functions are defined inside Rasterizer.
typedef void (Terrain::*Rule)(void);

class DeltaLSystem {
public:
    QHash<QChar, QString> expansionOperations;
    QHash<QChar, Rule> drawingOperations;

    DeltaLSystem();

    QString expandGrammar(const QString& s) const;
};

#endif // DELTALSYSTEM_H
