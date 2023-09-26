#include "SearchVisitor.h"

SearchVisitor::SearchVisitor(const QString& searchStr) {
    // @todo this tries to cater for both ways (wildcards and regexp) but it does a bad job at that.
    QStringList tokens = QString(searchStr)
        .replace(QRegExp("\\*"), ".*")
        .split(QRegExp("[ \\,]+"), Qt::SkipEmptyParts);

    if (tokens.size() == 1) {
        _regex = QRegExp("^" + tokens.first() + ".*", Qt::CaseInsensitive);
        return;
    }

    QString regExpStr = "^(" + tokens.first();
    for (int i = 1; i < tokens.size(); i++) {
        regExpStr += "|" + tokens[i];
    }
    regExpStr += ".*)";
    _regex = QRegExp(regExpStr, Qt::CaseInsensitive);
}

void SearchVisitor::visit(MapObject* object) {
    if (!object->matches(_regex)) {
        return;
    }

    _resultFromVisitors.append(object);
}

QList<MapObject*> SearchVisitor::result() const {
    QList<MapObject*> result(_resultFromVisitors);

    // airlines - this is not using the visitor model
    foreach (const Airline* _airline, airlines) {
        if (_airline->code.contains(_regex) || _airline->name.contains(_regex) || _airline->callsign.contains(_regex)) {
            // we make it into a MapObject, because that fits the results here well
            MapObject* object = new MapObject(
                _airline->label(),
                _airline->toolTip()
            );
            result.append(object);
        }
    }

    std::sort(
        result.begin(),
        result.end(),
        [](const MapObject* a, const MapObject* b) {
            // we had a crash here
            return a->mapLabel() < b->mapLabel();
        }
    );

    return result;
}
