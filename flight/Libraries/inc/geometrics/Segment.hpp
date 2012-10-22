
#ifndef GEOSEG_HPP
#define GEOSEG_HPP

#include "opencv/cv.h"
#include "geometrics/Plane.hpp"
#include <exception>

namespace geo {

class Segment: public geo::Line {

public:
	
	Segment() : geo::Line() {}
	Segment(cv::Point3d _P0, cv::Point3d _P1) : geo::Line(_P0,_P1) {}

	// returns true if a point is on the line segment
	bool isOn(cv::Point3d P) {
		if (!geo::Line::isOn(P)) return false;
		double gamma = fraction(P);
		if (gamma<0) return false;
		if (gamma>1) return false;
		return true;
	}
	
	// returns true if two segments ever cross
	bool intersects(geo::Line L) {
		if (!useable && !L.useable) {
			if (P0==L.P0) return true; else return false;
		}
		if (!useable) {
			if (L.isOn(P0)) return true; else return false;
		}
		if (!L.useable) {
			if (isOn(L.P0)) return true; else return false;
		}
		// actual check - if perpendicular line has length 0, its an intersection
		geo::Line x(perpendicular(L));
		if (x.useable) return false;
		return isOn(x.P0);
	}

	// almost the same, but it needs to be on both of them
	bool intersects(Segment L) {
		if (!useable && !L.useable) {
			if (P0==L.P0) return true; else return false;
		}
		if (!useable) {
			if (L.isOn(P0)) return true; else return false;
		}
		if (!L.useable) {
			if (isOn(L.P0)) return true; else return false;
		}
		// actual check - if perpendicular line has length 0, its an intersection
		geo::Line x(perpendicular(L));
		if (x.useable) return false;
		if (!L.isOn(x.P0)) return false;
		return isOn(x.P0);
	}

	// cuts a Segment with a Plane
	Segment cut(geo::Plane P) {
		if (!useable) return Segment(P0,P0);
		cv::Point3d x;
		try {
			x=P.intersect(geo::Line(P0,P1));
		} catch (geo::paraexception &e) {
			// no intersection, this means the line is parallel to the plane
			// eithe both P0 and P1 are on the right side or neither, so x
			// won't be needed.
			x=P0;
		}

		// creates Lines that run through P0 and P1 along the plane's normal
		geo::Line g(P.perpendicular(P0));
		geo::Line h(P.perpendicular(P1));

		geo::Line g2(g.P0,P.n);
		geo::Line h2(h.P0,P.n);

		if (g2.fraction(P0)>=0.) {
			// p0 is valid
			if (h2.fraction(P1)>=0.) {
				// p1 is valid, entire segment returned
				return Segment(P0,P1);
			} else {
				// p1 is invalid, return only till intersection point
				return Segment(P0,x);
			}
		} else {
			// p0 is invalid
			if (h2.fraction(P1)>=0.) {
				// p1 is valid, return from intersection point to P1
				return Segment(x,P1);
			} else {
				// p1 is invalid, return empty line
				return Segment(P0,P0);
			}
		}
	}
};



}



#endif
