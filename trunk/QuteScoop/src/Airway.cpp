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
	return (from == other.from || from == other.to) && (to == other.from || to == other.to);
}

Airway::Airway(const QString& name, Type type, int base, int top) {
	this->name = name;
	this->type = type;
	this->base = base;
	this->top = top;
}

Airway::~Airway() {
	// destructor
}

void Airway::addSegment(Waypoint* from, Waypoint* to) {
	Segment newSegment(from, to);
	// check if we already have this segment
	for(int i = 0; i < segments.size(); i++) {
		if(segments[i] == newSegment) {
			return;
		}
	}
	segments.append(newSegment);
}

Airway* Airway::createFromSegments() {
	if(segments.isEmpty())
		return 0;

	Airway* result = new Airway(name, type, base, top);
	Segment seg = segments.first();
	segments.removeFirst();
	result->waypoints.append(seg.from);
	result->waypoints.append(seg.to);

	bool nothingRemoved = false;
	while(!segments.isEmpty() && !nothingRemoved) {
		nothingRemoved = true;

		for(int i = 0; i < segments.size() && nothingRemoved; i++) {
			Segment s = segments[i];
			Waypoint *p = result->waypoints.last();

			if(s.from == p) {
				result->waypoints.append(s.to);
				segments.removeAt(i);
				nothingRemoved = false;
				continue;
			}
			if(s.to == p) {
				result->waypoints.append(s.from);
				segments.removeAt(i);
				nothingRemoved = false;
				continue;
			}

			p = result->waypoints.first();
			if(s.from == p) {
				result->waypoints.prepend(s.to);
				segments.removeAt(i);
				nothingRemoved = false;
				continue;
			}
			if(s.to == p) {
				result->waypoints.prepend(s.from);
				segments.removeAt(i);
				nothingRemoved = false;
				continue;
			}
		}
	}

	return result;
}

QList<Airway*> Airway::sort() {
	QList<Airway*> result;
	if(segments.isEmpty())
		return result;
	Airway *awy = 0;
	do {
		awy = createFromSegments();
		if(awy != 0)
			result.append(awy);
	} while(awy != 0);
	return result;
}

int Airway::getIndex(const QString& id) const {
	for(int i = 0; i < waypoints.size(); i++)
		if(waypoints[i]->label == id)
			return i;
	return -1;
}

QList<Waypoint*> Airway::expand(const QString& startId, const QString& endId) const {
	QList<Waypoint*> result;
	int startIndex = getIndex(startId);
	int endIndex = getIndex(endId);

	if(startIndex < 0 || endIndex < 0)
		return result;

	int direction = 1;
	if(startIndex > endIndex)
		direction = -1;

	for(int i = startIndex; i != endIndex; i += direction)
		if(i != startIndex)  // don't append first waypoint in list
			result.append(waypoints[i]);
	result.append(waypoints[endIndex]);
	return result;
}

Waypoint* Airway::getClosestPointTo(double lat, double lon) const {
	Waypoint* result = 0;
	double minDist = 9999.;
	for(int i = 0; i < waypoints.size(); i++) {
		double d = NavData::distance(lat, lon, waypoints[i]->lat, waypoints[i]->lon);
		if(qFuzzyIsNull(d))
			return waypoints[i];
		if(d < minDist) {
			minDist = d;
			result = waypoints[i];
		}
	}
	return result;
}
