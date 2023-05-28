/**************************************************************************
 *  This file is part of QuteScoop. See README for license
 **************************************************************************/

#ifndef SECTOR_H_
#define SECTOR_H_

#include <QtCore>
#include <QtOpenGL>

class Sector {
    public:
        Sector() :
            icao(), name(), id(), m_controllerSuffixes(QStringList()), _polygon(0), _borderline(0)
        {}
        Sector(QStringList fields, unsigned int debugLineNumber);
        ~Sector();

        bool isNull() const { return icao.isNull(); }

        bool containsPoint(const QPointF &pt) const;

        const QList<QPolygonF> &nonWrappedPolygons() const;
        const QList<QPair<double, double> > &points() const;
        void setPoints(const QList<QPair<double, double> >&);
        QString icao, name, id;

        GLuint glPolygon();
        GLuint glBorderLine();
        GLuint glPolygonHighlighted();
        GLuint glBorderLineHighlighted();

        QPair<double, double> getCenter() const;

        const QStringList& controllerSuffixes() const { return m_controllerSuffixes; }
    private:
        QStringList m_controllerSuffixes = QStringList();
        QList<QPolygonF> m_nonWrappedPolygons;
        QList<QPair<double, double> > m_points;
        GLuint _polygon, _borderline, _polygonHighlighted, _borderlineHighlighted;
};

#endif /*SECTOR_H_*/
