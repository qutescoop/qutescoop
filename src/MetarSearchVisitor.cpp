#include "Airport.h"

#include "MetarSearchVisitor.h"

void MetarSearchVisitor::visit(MapObject* object) {
    Airport* a = dynamic_cast<Airport*>(object);
    if (a == 0) {
        return;
    }
    if (a->id.contains(_regex)) {
        _airportMap[a->id] = a;
    }
}

QList<Airport*> MetarSearchVisitor::airports() const {
    QList<Airport*> res;

    QList<QString> labels = _airportMap.keys();
    labels.sort();
    for (int i = 0; i < labels.size(); i++) {
        res.append(_airportMap[labels[i]]);
    }

    return res;
}

QList<MapObject*> MetarSearchVisitor::result() const {
    QList<MapObject*> res;
    QList<Airport*> airpts = airports();
    for (int i = 0; i < airpts.size(); i++) {
        res.append(airpts[i]);
    }
    return res;
}
