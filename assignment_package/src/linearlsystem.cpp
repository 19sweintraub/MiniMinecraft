#include "linearlsystem.h"

LinearLSystem::LinearLSystem() : expansionOperations(), drawingOperations() {
    // For now only have one expansion operation. May try changing this up or playing
    // around with it in order to generate different types of rivers.
    std::cout << "Created LinearLSystem" << std::endl;
    expansionOperations.insert('X', QString("[PFX"));
}

// Expands a QString grammar using expansionOperations
QString LinearLSystem::expandGrammar(const QString &s) const {
    QString result(""); // initially result is just an empty Qstring
    for (QChar c : s) {
        if (expansionOperations.contains(c)) {
            result.push_back(expansionOperations.value(c));
        } else {
            // If c is not a key in the QHash, then just push itself back
            // This assumes that our initial axiom has only valid characters because
            // obviously we do not want to propagate erroneous characters.
            result.push_back(c);
        }
    }
    return result;
}
