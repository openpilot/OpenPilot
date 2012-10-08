#include "rtslam/simuRawProcessors.hpp"

namespace jafar {
namespace rtslam {
namespace simu {

	void randomizeSegment(jblas::vec4& segment)
	{
		float ran1 = 0;
		float ran2 = 0;

		while(fabs(ran1 - ran2) < 0.5 || 
			ran1 < 0 || ran1 > 1 ||
			ran2 < 0 || ran2 > 1)
		{
			// choose 2 random values between 0 and 1
			ran1 = double(rand()) / RAND_MAX;
			ran2 = double(rand()) / RAND_MAX;
		}

		if(ran1 < ran2)
		{
			float tmp = ran1;
			ran1 = ran2;
			ran2 = tmp;
		}

		// compute new extremities
		vec modified(4);
		modified[0] = ran1*segment[0] + (1-ran1)*segment[2];
		modified[1] = ran1*segment[1] + (1-ran1)*segment[3];
		modified[2] = ran2*segment[0] + (1-ran2)*segment[2];
		modified[3] = ran2*segment[1] + (1-ran2)*segment[3];
		segment = modified;
	}

	void projectOnPrediction(const jblas::vec4& meas, const jblas::vec4& exp, jblas::vec4& newMeas, float* stdRatio)
	{
		// extract predicted points
		jblas::vec2 P1 = subrange(exp,0,2);
		jblas::vec2 P2 = subrange(exp,2,4);
		double P12_2 = (P2(0) - P1(0))*(P2(0) - P1(0)) // Square(distance(P1,P2))
					  +   (P2(1) - P1(1))*(P2(1) - P1(1));
		double P12 = sqrt(P12_2);
		// extract measured line
		jblas::vec2 L1 = subrange(meas,0,2);
		jblas::vec2 L2 = subrange(meas,2,4);
		double L12_2 = (L2(0) - L1(0))*(L2(0) - L1(0)) // Square(distance(L1,L2))
					+   (L2(1) - L1(1))*(L2(1) - L1(1));
		double L12 = sqrt(L12_2);

		// compute predicted center
		jblas::vec2 Pc = (P1 + P2) / 2;
		// project on measured line
		double u = (((Pc(0) - L1(0))*(L2(0) - L1(0)))
					  +((Pc(1) - L1(1))*(L2(1) - L1(1))))
					  /(L12_2);
		jblas::vec2 Lc = L1 + u*(L2 - L1);

		// compute measured orientation
		double angle = atan2(L2(1) - L1(1), L2(0) - L1(0));

		// compute extremities
		newMeas[0] = Lc[0] - P12 * cos(angle) / 2;
		newMeas[1] = Lc[1] - P12 * sin(angle) / 2;
		newMeas[2] = Lc[0] + P12 * cos(angle) / 2;
		newMeas[3] = Lc[1] + P12 * sin(angle) / 2;

		*stdRatio = P12 / L12;
	}

}
}
}
