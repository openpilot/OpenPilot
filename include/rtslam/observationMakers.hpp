/**
 * \file observationMakers.hpp
 *
 * Header file for observation makers (atoms of the obs factory).
 *
 * \date 16/06/2010
 * \author nmansard from croussil
 *
 * \ingroup rtslam
 */

#include "rtslam/observationFactory.hpp"
#include "rtslam/featurePoint.hpp"
#include "rtslam/descriptorImagePoint.hpp"
#include "rtslam/descriptorImageSeg.hpp"
#include "rtslam/descriptorSeg.hpp"
#include "rtslam/simuData.hpp"

namespace jafar {
namespace rtslam {

template<class ObsType, class SenType, class LmkType, class AppType,
	 SensorAbstract::type_enum SenTypeId, LandmarkAbstract::type_enum LmkTypeId>
class ImagePointObservationMaker
  : public ObservationMakerAbstract
{
	private:
		double dmin;
		int patchSize;
	public:

		ImagePointObservationMaker(double _dmin, int _patchSize):
			ObservationMakerAbstract(SenTypeId, LmkTypeId), dmin(_dmin), patchSize(_patchSize) {}

		observation_ptr_t create(const sensor_ptr_t &senPtr, const landmark_ptr_t &lmkPtr)
		{
			boost::shared_ptr<ObsType> res(new ObsType(senPtr, lmkPtr));
			if (boost::is_same<AppType,AppearanceImagePoint>::value)
			{
				res->predictedAppearance.reset(new AppearanceImagePoint(patchSize, patchSize, CV_8U));
				res->observedAppearance.reset(new AppearanceImagePoint(patchSize, patchSize, CV_8U));
			} else
			if (boost::is_same<AppType,simu::AppearanceSimu>::value)
			{
				res->predictedAppearance.reset(new simu::AppearanceSimu());
				res->observedAppearance.reset(new simu::AppearanceSimu());
			}
			res->setup(dmin);
			return res;
		}
/*
		feature_ptr_t createFeat(const sensor_ptr_t &senPtr, const landmark_ptr_t &lmkPtr)
		{
			feature_ptr_t res(new FeatureImagePoint(patchSize, patchSize, CV_8U));
			return res;
		}

		descriptor_ptr_t createDesc(const sensor_ptr_t &senPtr, const landmark_ptr_t &lmkPtr, const feature_ptr_t &featPtr, const jblas::vec7 &senPoseInit, const observation_ptr_t &obsInitPtr)
		{
			feat_img_pnt_ptr_t featSpecPtr = SPTR_CAST<FeatureImagePoint>(featPtr);
			descriptor_ptr_t res(new DescriptorImagePoint(featSpecPtr, senPoseInit, obsInitPtr));
			return res;
		}
		*/
};

#ifdef HAVE_MODULE_DSEG

template<class ObsType, class SenType, class LmkType, class AppType,
    SensorAbstract::type_enum SenTypeId, LandmarkAbstract::type_enum LmkTypeId>
class ImageSegmentObservationMaker
  : public ObservationMakerAbstract
{
   private:
      double reparTh;
      int killSizeTh;
      int killSearchTh;
      double killMatchTh;
      double killConsistencyTh;
      double dmin;
      int patchSize;
   public:

      ImageSegmentObservationMaker(double _reparTh, int _killSizeTh, int _killSearchTh, double _killMatchTh, double _killConsistencyTh, double _dmin, int _patchSize):
         ObservationMakerAbstract(SenTypeId, LmkTypeId), reparTh(_reparTh), killSizeTh(_killSizeTh), killSearchTh(_killSearchTh),
         killMatchTh(_killMatchTh), killConsistencyTh(_killConsistencyTh), dmin(_dmin), patchSize(_patchSize) {}

      observation_ptr_t create(const sensor_ptr_t &senPtr, const landmark_ptr_t &lmkPtr)
      {
         boost::shared_ptr<ObsType> res(new ObsType(senPtr, lmkPtr));
         if (boost::is_same<AppType,AppearanceImageSegment>::value)
         {
            res->predictedAppearance.reset(new AppearanceImageSegment(patchSize, patchSize, CV_8U));
            res->observedAppearance.reset(new AppearanceImageSegment(patchSize, patchSize, CV_8U));
         } else
         if (boost::is_same<AppType,simu::AppearanceSimu>::value)
         {
            res->predictedAppearance.reset(new simu::AppearanceSimu());
            res->observedAppearance.reset(new simu::AppearanceSimu());
         }
         res->setup(reparTh, killSizeTh, killSearchTh, killMatchTh, killConsistencyTh, dmin);
         return res;
      }
};


template<class ObsType, class SenType, class LmkType, class AppType,
    SensorAbstract::type_enum SenTypeId, LandmarkAbstract::type_enum LmkTypeId>
class SegmentObservationMaker
  : public ObservationMakerAbstract
{
   private:
      double reparTh;
      int killSizeTh;
      int killSearchTh;
      double killMatchTh;
      double killConsistencyTh;
      double dmin;
      int patchSize;
   public:

      SegmentObservationMaker(double _reparTh, int _killSizeTh, int _killSearchTh, double _killMatchTh, double _killConsistencyTh, double _dmin, int _patchSize):
         ObservationMakerAbstract(SenTypeId, LmkTypeId), reparTh(_reparTh), killSizeTh(_killSizeTh), killSearchTh(_killSearchTh),
         killMatchTh(_killMatchTh), killConsistencyTh(_killConsistencyTh), dmin(_dmin), patchSize(_patchSize) {}

      observation_ptr_t create(const sensor_ptr_t &senPtr, const landmark_ptr_t &lmkPtr)
      {
         boost::shared_ptr<ObsType> res(new ObsType(senPtr, lmkPtr));
			if (boost::is_same<AppType,AppearanceImageSegment>::value)
         {
				res->predictedAppearance.reset(new AppearanceImageSegment(patchSize, patchSize, CV_8U));
				res->observedAppearance.reset(new AppearanceImageSegment(patchSize, patchSize, CV_8U));
         } else
         if (boost::is_same<AppType,simu::AppearanceSimu>::value)
         {
            res->predictedAppearance.reset(new simu::AppearanceSimu());
            res->observedAppearance.reset(new simu::AppearanceSimu());
         }
         res->setup(dmin);
         return res;
      }
};

#endif

}} // namespace jafar::rtslam
