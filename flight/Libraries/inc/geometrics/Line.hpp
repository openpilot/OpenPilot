
#ifndef GEOLINE_HPP
#define GEOLINE_HPP

#include "opencv/cv.h"
#include <exception>

namespace geo {

class lineexception : public std::exception
{
	virtual const char* what() const throw()
	{
		return "Line with zero vector";
	}
} lineex;


class Line {

public:
	cv::Point3d P0,P1;
	cv::Vec3d u;
	bool useable;

	// dummy constructor
	Line() {
		P0=cv::Point3d(0,0,0);
		P1=cv::Point3d(0,0,0);
		u=cv::Vec3d(0,0,0);
		useable=false;
	}
	// constructors for line given by 2 points 
	Line(cv::Point3d _P0, cv::Point3d _P1) {
		P0=_P0;
		P1=_P1;
		u=P1-P0;
		if (P0!=P1) useable=true; else useable=false;
	}
	// constructors for line given by point + vector
	Line(cv::Point3d _P0, cv::Vec3d _u) {
		P0=_P0;
		P1= cv::Vec3d(P0) + _u;
		u=_u;
		if (P0!=P1) useable=true; else useable=false;
	}

	// returns perpendicular line that reaches from closest point
	// S on this line to Q. Returns Q and 0 vector if Q is on this line
	Line perpendicular(cv::Point3d Q) {
		if (!useable) throw lineex;
		double alpha = cv::Vec3d(Q-P0).dot(u)/u.dot(u);
		return Line(cv::Vec3d(P0) + alpha*u,Q); // using 2 point constructor
	}

	// returns perpendicular line that reaches from closest point P0 on this line to P1 on line L
	Line perpendicular(Line L) {
		if (!useable) throw lineex;
		cv::Vec3d c=u.cross(L.u);
		if (isParallel(L)) {
			// lines are parallel
			return perpendicular(L.P0);
		}

		double alpha = cv::Vec3d(L.P0-P0).dot(L.u.cross(u.cross(L.u))) / c.dot(c);

		double gamma = cv::Vec3d(L.P0-P0).dot(c) / c.dot(c);
		return Line(cv::Vec3d(P0)+alpha*u,cv::Vec3d(gamma*c));
	}

	// returns 0 if P=P0 1 if P=P1 <0 or >1 if not between
	// and >0 <1 if between
	double fraction(cv::Point3d P) {
		// we can calculate this on any dimension that has a non zero derivative
		cv::Vec3d x(perpendicular(P).P0 - P0);
		for (int t=0;t<2;t++) {
			if (u[t]!=0) return x[t]/u[t];
		}
		return x[2]/u[2];
	}
	
	// returns true if two lines are parallel
	bool isParallel(Line L) {
		if (!(useable && L.useable)) throw lineex;
		cv::Vec3d c=u.cross(L.u);
		if (c.dot(c)==0) {
			// lines are parallel
			return true;
		}
		return false;
	}

	// returns true if two lines ever cross
	bool intersects(Line L) {
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
		if (!perpendicular(L).useable) return true; else return false;
	}
	
	// returns true if a point is on the line
	bool isOn(cv::Point3d P) {
		if (P==P0) return true;
		cv::Point3d X=perpendicular(P).P0;
		if (X!=P) return false;
		return true;
	}
	

};




}



#endif
