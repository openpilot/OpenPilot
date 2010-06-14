/*
 * DesscriptorImagePointSimu.cpp
 *
 *  Created on: 20 avr. 2010
 *      Author: jeanmarie
 */

#include "jmath/ublasExtra.hpp"

#include "rtslam/descriptorImagePoint.hpp"
#include "rtslam/quatTools.hpp"

namespace jafar {
	namespace rtslam {
		using namespace ublasExtra;
		using namespace jblas;
		using namespace quaternion;

		DescriptorImagePoint::DescriptorImagePoint(const featurepoint_ptr_t & featPtr, const vec7 & pose0, observation_ptr_t & obsPtr) {
			// TODO Auto-generated constructor stub
		}

		DescriptorImagePoint::~DescriptorImagePoint() {
			// TODO Auto-generated destructor stub
		}

		bool DescriptorImagePoint::predictAppearance(const landmark_ptr_t & lmkPtr,
		    const observation_ptr_t & obsPtrNew, app_img_pnt_ptr_t & appPtr, unsigned char patchSize) {

			// First implementation: zoom and rotation
			vec7 poseNew = obsPtrNew->sensorPtr()->globalPose();
			vec3 position0 = ublas::subrange(pose0, 0, 3);
			vec3 positionNew = ublas::subrange(poseNew, 0, 3);
			vec4 quat0 = ublas::subrange(pose0, 3, 7);
			vec4 quatNew = ublas::subrange(poseNew, 3, 7);

			vec lmk = lmkPtr->state.x();
			vec pnt = lmkPtr->reparametrize_func(lmk);

			double zoom = (norm_2(pnt - position0))/(norm_2(pnt-positionNew));
			vec4 quatIncr = qProd(q2qc(quat0),quatNew);
			vec3 eulerIncr = q2e(quatIncr);
			double rotation = -eulerIncr(2); // take negative yaw angle

			// TODO: rotate and zoom the patch, and cut it to the appropriate size

			return true;
		}

	}
}
