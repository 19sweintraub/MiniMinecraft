#include "deltalsystem.h"

DeltaLSystem::DeltaLSystem() : expansionOperations(), drawingOperations() {
    std::cout << "Created DeltaLSystem" << std::endl;
    expansionOperations.insert('X', QString("[PFX"));
    // Y is another grammar used to represent that we need to add another pop.
    // Eventually we get sequences of ']' which allows us to trace back to where a river split/diverged
    expansionOperations.insert('Y', QString("Y]"));
}

// Expands a QString grammar using expansionOperations
QString DeltaLSystem::expandGrammar(const QString &s) const {
    QString result(""); // initially result is just an empty Qstring
    for (QChar c : s) {
        if (expansionOperations.contains(c)) {
            // If c is a key in the QHash, then append to result
//            result.push_back(expansionOperations.value(c));
            // With 95% probability, push back "[PFX".
            if (c == 'X') {
                float random = ((float)rand()) / RAND_MAX; // between 0 and 1 float
                if (random <= 0.9) {
                    result.push_back(QString("[PFX"));
                } else {
                    // With 5% probability push back "[PFX]PFX"
                    result.push_back(QString("[PFXY][PFX"));
                }
            }
            else {
                result.push_back(expansionOperations.value(c));
            }
        } else {
            // If c is not a key in the QHash, then just push itself back
            // This assumes that our initial axiom has only valid characters because
            // obviously we do not want to propagate erroneous characters.
            result.push_back(c);
        }
    }
    return result;
}
