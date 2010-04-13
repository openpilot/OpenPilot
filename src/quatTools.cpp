/**
 * \file quatTools.cpp
 *
 * \date 24/03/2010
 * \author jsola@laas.fr
 *
 *
 *  Definition of non-templatizable functions in namespace quatTools
 *
 * \ingroup rtslam
 */

#include "jmath/jblas.hpp"
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

		}
	}
}
