#ifndef SECTOR_H_
#define SECTOR_H_

#include <QtCore>
#include <QtOpenGL>

class Sector {
    public:
        Sector();
        Sector(const QStringList &fields, const int debugControllerLineNumber, const int debugSectorLineNumber = -1);
        ~Sector();

        bool isNull() const;

        bool containsPoint(const QPointF &pt) const;

        const QList<QPolygonF> &nonWrappedPolygons() const;

        const QList<QPair<double, double> > &points() const;
        void setPoints(const QList<QPair<double, double> >&);

        int debugControllerLineNumber();

        int debugSectorLineNumber();
        void setDebugSectorLineNumber(int newDebugSectorLineNumber);

        GLuint glPolygon();
        GLuint glBorderLine();
        GLuint glPolygonHighlighted();
        GLuint glBorderLineHighlighted();

        QPair<double, double> getCenter() const;

        const QStringList& controllerSuffixes() const;
        QString icao, name, id;

    private:
        int _debugControllerLineNumber = -1, _debugSectorLineNumber = -1;
        QStringList m_controllerSuffixes = QStringList();
        QList<QPolygonF> m_nonWrappedPolygons;
        QList<QPair<double, double> > m_points;
        GLuint _polygon = 0, _borderline = 0, _polygonHighlighted = 0, _borderlineHighlighted = 0;
};

#endif /*SECTOR_H_*/
