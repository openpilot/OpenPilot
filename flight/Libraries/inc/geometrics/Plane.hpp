
#ifndef GEOPLANE_HPP
#define GEOPLANE_HPP

#include "opencv/cv.h"
#include "geometrics/Line.hpp"
#include <exception>

using namespace cv;
using namespace std;

namespace geo {

class planeexception: public std::exception
{
  virtual const char* what() const throw()
  {
    return "Plane exception";
  }
} planeex;

class paraexception: public std::exception
{
  virtual const char* what() const throw()
  {
    return "Plane does not intersect (parallel in space)";
  }
} paraex;


class Plane {

private:
	void init(cv::Point3d _P0, cv::Vec3d _n) {
		P0=_P0;
		cv::Vec3d ref(1,0,0);
		if (ref.cross(_n)==cv::Vec3d(0,0,0)) ref=cv::Vec3d(0,1,0);
		P1=cv::Vec3d(_P0)+_n.cross(ref);
		P2=cv::Vec3d(_P0)+_n.cross(cv::Vec3d(P1-_P0));
		if (!(P0!=P1 && P0!=P2)) throw planeex;
		n=_n;
	}

public:
	cv::Point3d P0,P1,P2;
	cv::Vec3d n;

	// constructors for plane given by 3 points
	Plane(cv::Point3d _P0, cv::Point3d _P1, cv::Point3d _P2) {
		P0=_P0;
		P1=_P1;
		P2=_P2;
		n=cv::Vec3d(_P1-_P0).cross(cv::Vec3d(_P2-_P0));
		if (!(P0!=P1 && P0!=P2)) throw planeex;
	}
	// constructors for plane given by point and normal vector
	Plane(cv::Point3d _P0, cv::Vec3d _n) {
		init(_P0,_n);
	}
	Plane(geo::Line _L) {
		init(_L.P0,_L.u);
	}

	// returns perpendicular line that reaches from closest point
	// S on this plane to Q. Returns Q and 0 vector if Q is on this plane
	geo::Line perpendicular(cv::Point3d Q) {
		
		double alpha = cv::Vec3d(P0-Q).dot(n)/n.dot(n);
		return geo::Line( cv::Vec3d(Q) + alpha*n,Q );
	}

	// returns intersection point of line and plane
	cv::Point3d intersect(geo::Line L) {
		if (isParallel(L)) throw paraex;

		if (!L.useable) {
			if (isOn(L.P0)) return perpendicular(L.P0).P0;
			throw paraex;
		}
		double gamma = cv::Vec3d(P0-L.P0).dot(n) / L.u.dot(n);
		return cv::Vec3d(L.P0) + gamma*L.u;
	}

	// returns the intersection line
	geo::Line cross(Plane P) {

		if (isParallel(P)) throw paraex;

		cv::Vec3d a = n.cross(P.n);

		// get a point on that line, find out if P0.P1 intersects plane
		cv::Point3d X;
		try {
			X=intersect(geo::Line(P.P0,P.P1));
		} catch (paraexception& e) {
			X=intersect(geo::Line(P.P0,P.P2));
		}
		return geo::Line(X,a);

	}

	// returns 0 if P=P0 1 if P=P1 <0 or >1 if not between
	// and >0 <1 if between
	cv::Vec2d fraction(cv::Point3d P) {
		cv::Point3d Px = perpendicular(P).P0;

		return cv::Vec2d(
			geo::Line(P0,P1).fraction(Px),
			geo::Line(P0,P2).fraction(Px)
		);
	}
	
	// returns true if a point is on the plane
	bool isOn(cv::Point3d P) {
		cv::Point3d X=perpendicular(P).P0;
		if (X!=P) return false;
		return true;
	}

	// returns true if plane and line are parallel
	bool isParallel(geo::Line L) {
		if (n.dot(L.u)==0) return true; else return false;
	}

	// returns true if two planes are parallel
	bool isParallel(Plane P) {
		cv::Vec3d c=n.cross(P.n);
		if (c.dot(c)==0) {
			// lines are parallel
			return true;
		}
		return false;
	}


};

class Triangle: public Plane {

public:
	Triangle(cv::Point3d _P0, cv::Point3d _P1, cv::Point3d _P2) : geo::Plane(_P0,_P1,_P2) {}

	// returns true if a point is within a triangle P0,P1,P2
	bool isOn(cv::Point3d P) {
		cv::Point3d X=perpendicular(P).P0;
		if (X!=P) return false;
		cv::Vec2d gamma = fraction(P);
		if (gamma[0]<0 || gamma[1]<0) return false;
		if (gamma[0]>1 || gamma[1]>1) return false;
		return true;
	}
};



}



#endif
