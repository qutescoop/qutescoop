/**************************************************************************
 *  This file is part of QuteScoop. See README for license
 **************************************************************************/

#include "Airway.h"

#include "NavData.h"

Airway::Segment::Segment(Waypoint* from, Waypoint* to) {
	this->from = from;
	this->to = to;
}

bool Airway::Segment::operator==(const Airway::Segment& other) const {
	return (from == other.from || from == other.to) &&
			(to == other.from || to == other.to);
}

Airway::Airway(const QString& name) {
	this->name = name;
}

void Airway::addSegment(Waypoint* from, Waypoint* to) {
	Segment newSegment(from, to);
	// check if we already have this segment
	for(int i = 0; i < _segments.size(); i++) {
		if(_segments[i].from == newSegment.from && _segments[i].to == newSegment.to) {
			return;
		}
	}
	_segments.append(newSegment);
}

Airway* Airway::createFromSegments() {
	if(_segments.isEmpty())
		return 0;

	Airway* result = new Airway(name);
	Segment seg = _segments.first();
	_segments.removeFirst();
	result->_waypoints.append(seg.from);
	result->_waypoints.append(seg.to);

	bool nothingRemoved = false;
	while(!_segments.isEmpty() && !nothingRemoved) {
		nothingRemoved = true;

		for(int i = 0; i < _segments.size() && nothingRemoved; i++) {
			Segment s = _segments[i];
			Waypoint *p = result->_waypoints.last();

			if(s.from == p) {
				result->_waypoints.append(s.to);
				_segments.removeAt(i);
				nothingRemoved = false;
				continue;
			}
			if(s.to == p) {
				result->_waypoints.append(s.from);
				_segments.removeAt(i);
				nothingRemoved = false;
				continue;
			}

			p = result->_waypoints.first();
			if(s.from == p) {
				result->_waypoints.prepend(s.to);
				_segments.removeAt(i);
				nothingRemoved = false;
				continue;
			}
			if(s.to == p) {
				result->_waypoints.prepend(s.from);
				_segments.removeAt(i);
				nothingRemoved = false;
				continue;
			}
		}
	}

	return result;
}

QList<Airway*> Airway::sort() {
	QList<Airway*> result;
	if(_segments.isEmpty())
		return result;
	Airway *awy = 0;
	do {
		awy = createFromSegments();
		if(awy != 0)
			result.append(awy);
	} while(awy != 0);

        return result;
}

int Airway::index(const QString& id) const {
	for(int i = 0; i < _waypoints.size(); i++) {
		if(_waypoints[i]->label == id) {
			return i;
		}
	}
	return -1;
}

/**
 * Returns a list of all fixes along this airway from start
 * to end. The expanded list will not include the given start
 * point, but will include the given end point.
 */
QList<Waypoint*> Airway::expand(const QString& startId, const QString& endId) const {
	QList<Waypoint*> result;
	int startIndex = index(startId);
	int endIndex = index(endId);

	if(startIndex < 0 || endIndex < 0)
		return result;

	int direction = 1;
	if(startIndex > endIndex)
		direction = -1;

	for(int i = startIndex; i != endIndex; i += direction)
		if(i != startIndex)  // don't append first waypoint in list
			result.append(_waypoints[i]);
	result.append(_waypoints[endIndex]);
	return result;
}

Waypoint* Airway::closestPointTo(double lat, double lon) const {
	Waypoint* result = 0;
	double minDist = 9999.;
	for(int i = 0; i < _waypoints.size(); i++) {
		double d = NavData::distance(lat, lon, _waypoints[i]->lat, _waypoints[i]->lon);
		if(qFuzzyIsNull(d))
			return _waypoints[i];
		if(d < minDist) {
			minDist = d;
			result = _waypoints[i];
		}
	}
	return result;
}
