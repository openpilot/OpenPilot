
#ifndef JMATH_INTERPOL_HPP
#define JMATH_INTERPOL_HPP

#include <math.h>

namespace jafar {
namespace jmath {


/**
	Compute the parabolic interpolation of an extremum from 3 points, 
	and return x and y extrema and parabol coefficients
	
	@param x0 abscissa of left point, x0 < x1 < x2
	@param y0 value of left point
	@param x1 abscissa of center point, x0 < x1 < x2
	@param y1 value of center point, must be (greater or equal) or (less or equal) than y0 and y2
	@param x2 abscissa of right point, x0 < x1 < x2
	@param y2 value of right point
	@param extremum_x abscissa of extremum
	@param extremum_y value of extremum
	@param a,b,c parabol coefficients (a.x^2 + b.x + c = 0)
*/
static inline void parabolicInterpolation(double  x0, double  y0, double  x1, double  y1, double  x2, double  y2, double & extremum_x, double & extremum_y, double  &a, double  &b, double  &c)
{
	double  x01 = x0-x1, x02 = x0-x2, x12 = x1-x2;
	double  t0 = y0/(x01*x02), t1 = y1/(x01*x12), t2 = y2/(x02*x12);
	a = t0 - t1 + t2;
	b = -((x1+x2)*t0 - (x0+x2)*t1 + (x0+x1)*t2);
	c = x1*x2*t0 - x0*x2*t1 + x0*x1*t2;

	extremum_x = -b/(a*2.0);
	extremum_y = c-b*b/(a*4.0);
}

/**
	Compute the parabolic interpolation of an extremum from 3 points, 
	and return x and y extrema.

	For parameters see static inline void parabolicInterpolation(double  x0, double  y0, double  x1, double  y1, double  x2, double  y2, double & extremum_x, double & extremum_y, double  &a, double  &b, double  &c)
*/
static inline void parabolicInterpolation(double  x0, double  y0, double  x1, double  y1, double  x2, double  y2, double & extremum_x, double & extremum_y)
{
	double  tempa, tempb, tempc;
	parabolicInterpolation(x0, y0, x1, y1, x2, y2, extremum_x, extremum_y, tempa, tempb, tempc);
}

/**
	Compute the parabolic interpolation of an extremum from 3 points, 
	and return x extremum.

	For parameters see static inline void parabolicInterpolation(double  x0, double  y0, double  x1, double  y1, double  x2, double  y2, double & extremum_x, double & extremum_y, double  &a, double  &b, double  &c)
*/
static inline void parabolicInterpolation(double  x0, double  y0, double  x1, double  y1, double  x2, double  y2, double & extremum_x)
{
	double  x01 = x0-x1, x02 = x0-x2, x12 = x1-x2;
	double  t0 = y0/(x01*x02), t1 = y1/(x01*x12), t2 = y2/(x02*x12);
	double  a = t0 - t1 + t2;
	double  b = -((x1+x2)*t0 - (x0+x2)*t1 + (x0+x1)*t2);

	extremum_x = -b/(a*2.0);
}

/**
	Compute the parabolic interpolation of an extremum from 3 points with constant x intervals, 
	and return x an y extrema and the curvature coefficient a

	For parameters see static inline void parabolicInterpolation(double  x0, double  y0, double  x1, double  y1, double  x2, double  y2, double & extremum_x, double & extremum_y, double  &a, double  &b, double  &c)
*/
static inline void parabolicInterpolation(double  y0, double  y1, double  y2, double & extremum_x, double & extremum_y, double & a)
{
	//double  x0 = -1, x1 = 0, x2 = 1;
	//double  x01 = -1, x02 = -2, x12 = -1;
	//double  t0 = y0/2, t1 = y1, t2 = y2/2;
	a = (y0+y2)/2.0 - y1;
	double  b = (y2-y0)/2.0;
	double  c = y1;

	if (a!=0.) {
		extremum_x = -b/(a*2.0);
		extremum_y = c-b*b/(a*4.0);
	} else {
		extremum_x = y1;
		extremum_y = y1;
	}
}

/**
	Compute the parabolic interpolation of an extremum from 3 points with constant x intervals, 
	and return x an y extrema.

	For parameters see static inline void parabolicInterpolation(double  x0, double  y0, double  x1, double  y1, double  x2, double  y2, double & extremum_x, double & extremum_y, double  &a, double  &b, double  &c)
*/
static inline void parabolicInterpolation(double  y0, double  y1, double  y2, double & extremum_x, double & extremum_y)
{
	double tempa;
	parabolicInterpolation(y0, y1, y2, extremum_x, extremum_y, tempa);
}

/**
	Compute the parabolic interpolation of an extremum from 3 points with constant x intervals, 
	and return x extremum.

	For parameters see static inline void parabolicInterpolation(double  x0, double  y0, double  x1, double  y1, double  x2, double  y2, double & extremum_x, double & extremum_y, double  &a, double  &b, double  &c)
*/
static inline void parabolicInterpolation(double  y0, double  y1, double  y2, double & extremum_x)
{
	//double  x0 = -1, x1 = 0, x2 = 1;
	//double  x01 = -1, x02 = -2, x12 = -1;
	//double  t0 = y0/2, t1 = y1, t2 = y2/2;
	double  a = (y0+y2)/2.0 - y1;
	double  b = (y2-y0)/2.0;

	extremum_x = -b/(a*2.0);
}

/**
	Return the parabol width of coefficients a, b, c at y=y0
*/
static inline double  parabolWidth(double a, double b, double c, double y0=0)
{
	c -= y0;
	double delta = b*b-4.0*a*c;
	if (delta <= 0) return 0; else return sqrt(delta)/(2*std::abs(a));
}



/**
	Cubic interpolation of 1D function
*/
class CubicInterpolate
{
	double a0,a1,a2,a3;
	double m1,m2,y1,y2,x1,x2;
	bool constant;
	
	public:
		/**
			Define the segment that will be interpolated in the general case
			x0 < x1 < x2 < x3
			The interpolation will be between x1 and x2
		*/
		inline void setSegment(double x0, double y0, double x1, double y1, double x2, double y2,double x3, double y3)
		{
			//m1 = (y2-y1)/(2*(x2-x1)) + (y1-y0)/(2*(x1-x0));
			//m2 = (y3-y2)/(2*(x3-x2)) + (y2-y1)/(2*(x2-x1));
			
			m1 = (y2-y1)/(2) + (x2-x1)*(y1-y0)/(2*(x1-x0));
			m2 = (x2-x1)*(y3-y2)/(2*(x3-x2)) + (y2-y1)/(2);
			this->y1 = y1;
			this->y2 = y2;
			this->x1 = x1;
			this->x2 = x2;
			constant = false;
		}
		/**
			Define the segment that will be interpolated in the case where there is a constant x interval
			x(yi) = C + i*dx
			The interpolation will be between x(y1) and x(y2)
		*/
		inline void setSegment(double y0,double y1,double y2,double y3)
		{
			a0 = y3 - y2 - y0 + y1;
			a1 = y0 - y1 - a0;
			a2 = y2 - y0;
			a3 = y1;
			constant = true;
		}
		
		/**
			Interpolate at x1 + t*(x2-x1) with t in [0;1]
		*/
		inline double interpolateCoeff(double t)
		{
			double t2 = t*t;
			if (constant)
			{
				return a0*t*t2 + a1*t2 + a2*t + a3;
			} else
			{
				double t3 = t2*t;
				
				double h00 = 2*t3 - 3*t2 + 1;
				double h10 = t3 - 2*t2 + t;
				double h01 = -2*t3 + 3*t2;
				double h11 = t3 - t2;
				
				//return h00*y1 + h10*(x2-x1)*m1 + h01*y2 + h11*(x2-x1)*m2;
				return h00*y1 + h10*m1 + h01*y2 + h11*m2;
			}
		}
		
		/**
			Interpolate at x with x in [x1;x2]
		*/
		inline double interpolateValue(double x)
		{
			if (constant)
			{
				return interpolateCoeff(x);
			} else
			{
				double t = (x - x1) / (x2 - x1);
				return interpolateCoeff(t);
			};
		}
		
};




}}

#endif

