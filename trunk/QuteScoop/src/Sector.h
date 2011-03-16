/**************************************************************************
 *  This file is part of QuteScoop. See README for license
 **************************************************************************/

#ifndef SECTOR_H_
#define SECTOR_H_

#include "_pch.h"


class Sector {
public:
    Sector();
    Sector(QStringList strings);
    ~Sector();

    bool isNull() const;

	const QString& icao() const { return _icao; }
	const QString& name() const { return _name; }
	const QString& countryCode() const { return _countryCode; }
	const QString& id() const { return _id; }
	const double& lat() const { return _lat; }
	const double& lon() const { return _lon; }
	const QList<QPair<double, double> > sector() const { return _points; }
	const QPolygonF& sectorPolygon() const;

	void setPointList(const QList<QPair<double, double> >& points);

	GLuint getPolygon();
	GLuint getBorderLine();
private:
	void compileDisplayLists();

	QList<QPair<double, double> > _points;
	QString _icao, _name, _countryCode, _id;
	double _lat, _lon;
	GLuint _polygon, _borderline;
};

#endif /*SECTOR_H_*/
