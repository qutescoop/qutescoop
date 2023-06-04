#ifndef MAPOBJECT_H_
#define MAPOBJECT_H_

#include <QtCore>

class MapObject
    : public QObject {
    Q_OBJECT
    public:
        MapObject();
        MapObject(QString label, QString toolTip);
        MapObject(const MapObject& obj);
        MapObject& operator=(const MapObject& obj);
        virtual ~MapObject();

        // @todo: move into TextSearchable interface
        virtual bool matches(const QRegExp& regex) const;

        virtual QString mapLabel() const;
        virtual QString mapLabelHovered() const;
        virtual QStringList mapLabelSecondaryLines() const;
        virtual QStringList mapLabelSecondaryLinesHovered() const;
        virtual QString toolTip() const;
        virtual bool hasPrimaryAction() const;
        virtual void primaryAction();

        double lat, lon;
        bool drawLabel;
    protected:
        // @todo just meant for non-derived objects that we currently use for airlines in the search for example
        QString m_label;
        QString m_toolTip;
};

#endif /*MAPOBJECT_H_*/
