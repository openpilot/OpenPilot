/*
 * DesscriptorImagePoint.cpp
 *
 *  Created on: 20 avr. 2010
 *      Author: jmcodol@laas.fr
 */

#include "jmath/ublasExtra.hpp"
#include "jmath/angle.hpp"

#include "rtslam/descriptorImagePoint.hpp"
#include "rtslam/quatTools.hpp"

namespace jafar {
	namespace rtslam {
		using namespace ublasExtra;
		using namespace jblas;
		using namespace quaternion;

		DescriptorImagePoint::DescriptorImagePoint(const feat_img_pnt_ptr_t & featImgPntPtr_, const vec7 & senPoseInit_, const observation_ptr_t & obsInitPtr_):
			DescriptorAbstract(), senPoseInit(senPoseInit_), /*obsInitPtr(obsInitPtr_),*/ featImgPntPtr(featImgPntPtr_)
			// FIXME is obsInit useful, if it is how to copy the descriptor while letting obsInit die
		{
			// TODO Auto-generated constructor stub
		}

		DescriptorImagePoint::~DescriptorImagePoint() {
			// TODO Auto-generated destructor stub
		}


		bool DescriptorImagePoint::predictAppearance(const obs_ph_euc_ptr_t & obsPtrNew)
		{
			return predictAppearance_img_pt(obsPtrNew);
		}
		bool DescriptorImagePoint::predictAppearance(const obs_ph_ahp_ptr_t & obsPtrNew)
		{
			return predictAppearance_img_pt(obsPtrNew);
		}
		
		bool DescriptorImagePoint::predictAppearance_img_pt(const observation_ptr_t & obsPtrNew) {

			double zoom, rotation;
			landmark_ptr_t lmkPtr = obsPtrNew->landmarkPtr();
			vec lmk = lmkPtr->state.x();
			vec pnt = lmkPtr->reparametrize_func(lmk);
			quaternion::getZoomRotation(senPoseInit, obsPtrNew->sensorPtr()->globalPose(), pnt, zoom, rotation);
			// normally we must cast to the derived type
			app_img_pnt_ptr_t app_dst = SPTR_CAST<AppearanceImagePoint>(obsPtrNew->predictedAppearance);
			app_img_pnt_ptr_t app_src = SPTR_CAST<AppearanceImagePoint>(featImgPntPtr->appearancePtr);
			// rotate and zoom the patch, and cut it to the appropriate size
			app_src->patch.rotateScale(jmath::radToDeg(rotation), zoom, app_dst->patch);
//app_src->patch.save("descriptor_patch.png");
//app_dst->patch.save("predicted_patch.png");
			return true;
		}

	}
}
