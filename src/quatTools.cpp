/**
 * \file quatTools.cpp
 * \date 24/03/2010
 * \author jsola
 * \ingroup rtslam
 */

#include "jmath/jblas.hpp"
#include "jmath/angle.hpp"
#include "rtslam/quatTools.hpp"

namespace jafar {
	namespace rtslam {
		namespace quaternion {
			using namespace std;

			jblas::vec4 identQ() {
				jblas::vec4 q0;
				q0.clear();
				q0(0) = 1.0;
				return q0;
			}

			jblas::vec7 originFrame(){
				jblas::vec7 F;
				F.clear();
				F(3) = 1.0;
				return F;
			}

			jblas::vec4 flu2rdfQuat(){
				jblas::vec3 e;
				e.clear();
				e(0) = jmath::degToRad(-90.0);
				e(1) = 0.0;
				e(2) = e(0);
				return e2q(e);
			}

			jblas::vec7 flu2rdfFrame(){
				vec7 F;
				F.clear();
				ublas::subrange(F, 3, 7) = flu2rdfQuat();
				return F;
			}

		}
	}
}
