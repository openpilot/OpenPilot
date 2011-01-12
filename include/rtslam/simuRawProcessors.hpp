/**
 * \file simuRawProcessors.hpp
 *
 * Raw processors for simulation.
 * This is NOT part of the ad-hoc simulator, but required for any estimator without realistic raw simulation.
 *
 * \date 24/07/2010
 * \author croussil
 *
 * \ingroup rtslam
 */

#ifndef SIMURAWPROCESSORS_HPP_
#define SIMURAWPROCESSORS_HPP_

#include "jmath/random.hpp"
#include "rtslam/landmarkAbstract.hpp"
#include "rtslam/simuData.hpp"

namespace jafar {
namespace rtslam {
namespace simu {

	template<class RoiSpec>
	class MatcherSimu
	{
		public:
			struct matcher_params_t {
				int patchSize;
				int maxSearchSize;
				double lowInnov;      ///<     search region radius for first RANSAC consensus
				double threshold;     ///<     matching threshold
				double mahalanobisTh; ///< Mahalanobis distance for outlier rejection
				double measStd;       ///<       measurement noise std deviation
				double measVar;       ///<       measurement noise variance
				double simuMeasStd;
			} params;
			LandmarkAbstract::geometry_t type;
			size_t size;
			MultiDimNormalDistribution *noise;

		public:
			MatcherSimu(LandmarkAbstract::geometry_t type, size_t size, int patchSize, int maxSearchSize, double lowInnov, double threshold, double mahalanobisTh, double measStd, double simuMeasStd):
				type(type), size(size), noise(NULL)
			{
				JFR_ASSERT(patchSize%2, "patchSize must be an odd number!");
				params.patchSize = patchSize;
				params.maxSearchSize = maxSearchSize;
				params.lowInnov = lowInnov;
				params.threshold = threshold;
				params.mahalanobisTh = mahalanobisTh;
				params.measStd = measStd;
				params.measVar = measStd * measStd;
				params.simuMeasStd = simuMeasStd;
			}
			~MatcherSimu()
			{
				delete noise;
			}

			bool match(const boost::shared_ptr<RawSimu> & rawPtr, const appearance_ptr_t & targetApp, const RoiSpec & roi, Measurement & measure, appearance_ptr_t & app)
			{
				if (noise == NULL)
				{
					jblas::vec x = jblas::zero_vec(size);
					jblas::scalar_vec P(size, params.simuMeasStd*params.simuMeasStd);
					noise = new MultiDimNormalDistribution(x, P, rtslam::rand());
				}
				
				boost::shared_ptr<simu::AppearanceSimu> targetAppSpec = SPTR_CAST<simu::AppearanceSimu>(targetApp);
				boost::shared_ptr<simu::AppearanceSimu> appSpec = SPTR_CAST<simu::AppearanceSimu>(app);
				*appSpec = *targetAppSpec;
				
				measure.std(params.measStd);
				RawSimu::ObsList::iterator it = rawPtr->obs.find(targetAppSpec->id);
				if (it != rawPtr->obs.end())
				{
					measure.matchScore = 1.0;
					measure.x() = it->second->measurement.x() + noise->get(); // simulation noise
					return roi.isIn(measure.x());
				} else
				{
					measure.matchScore = 0.0;
					return false;
				}
			}
	};

	/*
	*/

	template<class RoiSpec>
	class DetectorSimu
	{
		public:
			struct detector_params_t {
				int patchSize;  ///<       descriptor patch size
				double measStd; ///<       measurement noise std deviation
				double measVar; ///<       measurement noise variance
				double simuMeasStd;
			} params;
			LandmarkAbstract::geometry_t type;
			size_t size;
			MultiDimNormalDistribution *noise;
			
		public:
			DetectorSimu(LandmarkAbstract::geometry_t type, size_t size, int patchSize, double measStd, double simuMeasStd):
				type(type), size(size), noise(NULL)
			{
				JFR_ASSERT(patchSize%2, "patchSize must be an odd number!");
				params.patchSize = patchSize;
				params.measStd = measStd;
				params.measVar = measStd * measStd;
				params.simuMeasStd = simuMeasStd;
			}

			bool detect(const boost::shared_ptr<RawSimu> & rawData, const RoiSpec &roi, boost::shared_ptr<FeatureSimu> & featPtr)
			{
				if (noise == NULL)
				{
					jblas::vec x = jblas::zero_vec(size);
					jblas::scalar_vec P(size, params.simuMeasStd*params.simuMeasStd);
					noise = new MultiDimNormalDistribution(x, P, rtslam::rand());
				}
				
				std::vector<featuresimu_ptr_t> roiObs;
				
				for (RawSimu::ObsList::iterator it = rawData->obs.begin(); it != rawData->obs.end(); ++it)
				{
					boost::shared_ptr<AppearanceSimu> app = SPTR_CAST<AppearanceSimu>(it->second->appearancePtr);
					if (app->type == type && roi.isIn(it->second->measurement.x()))
						roiObs.push_back(it->second);
				}
				
				if (roiObs.size() > 0)
				{
					int n = rtslam::rand()%roiObs.size();
					featPtr = roiObs[n];
// JFR_DEBUG("detecting feature " << SPTR_CAST<AppearanceSimu>(featPtr->appearancePtr)->id << " in " << roi.x() << " " << roi.y() << " " << roi.w() << " " << roi.h());
					featPtr->measurement.matchScore = 1.0;
// JFR_DEBUG_BEGIN(); JFR_DEBUG_SEND("simu detected feat at " << featPtr->measurement.x());
					featPtr->measurement.x() += noise->get(); // simulation noise
// JFR_DEBUG_SEND(" and with noise at " << featPtr->measurement.x()); JFR_DEBUG_END();
					featPtr->measurement.std(params.measStd);
					return true;
				} else
				{
					featPtr.reset(new FeatureSimu(size));
					featPtr->measurement.matchScore = 0.0;
					return false;
				}
			}
			
			void fillDataObs(const boost::shared_ptr<FeatureSimu> & featPtr, boost::shared_ptr<ObservationAbstract> & obsPtr)
			{
				// extract observed appearance
				boost::shared_ptr<AppearanceSimu> app_src = SPTR_CAST<AppearanceSimu>(featPtr->appearancePtr);
				boost::shared_ptr<AppearanceSimu> app_dst = SPTR_CAST<AppearanceSimu>(obsPtr->observedAppearance);
				*app_dst = *app_src;
				
				// create descriptor
				boost::shared_ptr<simu::DescriptorSimu> descPtr(new simu::DescriptorSimu());
				descPtr->addObservation(obsPtr);
				obsPtr->landmarkPtr()->setDescriptor(descPtr);
			}

	};
	
	
	
	
	
	
	
	
	
	
}}}

#endif

