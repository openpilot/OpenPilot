/*
 * DesscriptorImagePoint.cpp
 *
 *  Created on: 20 avr. 2010
 *      Author: jmcodol@laas.fr
 */

#include "jmath/ublasExtra.hpp"

#include "rtslam/descriptorImagePoint.hpp"
#include "rtslam/quatTools.hpp"

namespace jafar {
	namespace rtslam {
		using namespace ublasExtra;
		using namespace jblas;
		using namespace quaternion;

		DescriptorImagePoint::DescriptorImagePoint(const feat_img_pnt_ptr_t & featImgPntPtr_, const vec7 & senPoseInit_, observation_ptr_t & obsInitPtr_):
			DescriptorAbstract(), senPoseInit(senPoseInit_), obsInitPtr(obsInitPtr_), featImgPntPtr(featImgPntPtr_)
		{
			// TODO Auto-generated constructor stub
		}

		DescriptorImagePoint::~DescriptorImagePoint() {
			// TODO Auto-generated destructor stub
		}


		bool DescriptorImagePoint::predictAppearance(obs_ph_euc_ptr_t & obsPtrNew)
		{
			return predictAppearance_img_pt(obsPtrNew);
		}
		bool DescriptorImagePoint::predictAppearance(obs_ph_ahp_ptr_t & obsPtrNew)
		{
			return predictAppearance_img_pt(obsPtrNew);
		}
		
		bool DescriptorImagePoint::predictAppearance_img_pt(const observation_ptr_t & obsPtrNew) {

			double zoom, rotation;
			landmark_ptr_t lmkPtr = obsPtrNew->landmarkPtr();
			vec lmk = lmkPtr->state.x();
			vec pnt = lmkPtr->reparametrize_func(lmk);
			quaternion::getZoomRotation(senPoseInit, obsPtrNew->sensorPtr()->globalPose(), pnt, zoom, rotation);

			// rotate and zoom the patch, and cut it to the appropriate size
			app_img_pnt_ptr_t app = boost::static_pointer_cast<AppearenceImagePoint>(obsPtrNew->predictedAppearance);
			featImgPntPtr->appImgPntPtr->patch.rotateScale(rotation, zoom, app->patch);
			
			return true;
		}

	}
}
