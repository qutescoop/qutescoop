#include "MapObject.h"

MapObject::MapObject()
    : QObject(),
      lat(0.),
      lon(0.),
      drawLabel(true) {}

MapObject::MapObject(QString label, QString toolTip)
    : QObject(),
      m_label(label),
      m_toolTip(toolTip) {}

MapObject::MapObject(const MapObject& obj)
    : QObject() {
    if (this == &obj) {
        return;
    }

    lat = obj.lat;
    lon = obj.lon;
    m_label = obj.m_label;
    drawLabel = obj.drawLabel;
}

MapObject& MapObject::operator=(const MapObject& obj) {
    if (this == &obj) {
        return *this;
    }

    lat = obj.lat;
    lon = obj.lon;
    m_label = obj.m_label;
    drawLabel = obj.drawLabel;
    return *this;
}

MapObject::~MapObject() {}

// todo move into TextSearchable interface
bool MapObject::matches(const QRegExp &regex) const {
    return m_label.contains(regex)
        || mapLabel().contains(regex)
    ;
}

QString MapObject::mapLabel() const {
    return m_label;
}

QString MapObject::mapLabelHovered() const {
    return mapLabel();
}

QStringList MapObject::mapLabelSecondaryLines() const {
    return {};
}

QStringList MapObject::mapLabelSecondaryLinesHovered() const {
    return mapLabelSecondaryLines();
}

QString MapObject::toolTip() const {
    return m_toolTip;
}

bool MapObject::hasPrimaryAction() const {
    return false;
}

void MapObject::primaryAction() {}
