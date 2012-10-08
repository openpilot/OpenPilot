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
#include "rtslam/featureAbstract.hpp"
#include "rtslam/observationAbstract.hpp"
#include "rtslam/landmarkAbstract.hpp"
#include "rtslam/simuData.hpp"

namespace jafar {
namespace rtslam {
namespace simu {

	template<class RoiSpec>
	bool segmentInRect(const RoiSpec& roi, jblas::vec& measure)
	{
		// Compute line parameters (y = ax + b)
		float a = (measure[3]-measure[1])/(measure[2]-measure[0]);
		float b = measure[1]-(a*measure[0]);

		float rect_left = roi.x();
		float rect_top = roi.y();
		float rect_right = rect_left + roi.w();
		float rect_bottom = rect_top + roi.h();

		if(a!=a || fabs(a) > 10000)
		{
			if(measure[0] < rect_left || measure[0] > rect_right ||
				min(measure[1],measure[3]) > rect_bottom || 
				max(measure[1],measure[3]) < rect_top) {
				return false;
			}

			if(max(measure[1],measure[3]) == measure[1]) {
				if(measure[1] > rect_bottom)
					measure[1] = rect_bottom;
				if(measure[3] < rect_top)
					measure[3] = rect_top;
			} else {
				if(measure[3] > rect_bottom)
					measure[3] = rect_bottom;
				if(measure[1] < rect_top)
					measure[1] = rect_top;
			}

			return true;
		}
/*
		float minx = min(measure[0],measure[2]);
		float miny = min(measure[1],measure[3]);
		float maxx = max(measure[0],measure[2]);
		float maxy = max(measure[1],measure[3]);
*/
		// Compute y on the left and right border of the roi
		float lefty = a*rect_left + b;
		float righty = a*rect_right + b;

		// Reduce 
		float bar1,bar2;
		vec4 intersect;
		if(lefty < rect_top)
		{
			if(righty < rect_top) // no edge crossed
			{
				return false;
			}
			else // top edge crossed
			{
				// compute top edge intersection
				float topx = (rect_top-b)/a;
				intersect[0] = topx;
				intersect[1] = rect_top;

				if(righty <= rect_bottom) // right edge crossed
				{
					intersect[2] = rect_right;
					intersect[3] = righty;
					bar1 = (topx - measure[0]) / (measure[2]-measure[0]);
					bar2 = (righty - measure[1]) / (measure[3]-measure[1]);

				}
				else // bottom edge crossed
				{
					// compute bottom edge intersection
					float bottomx = (rect_bottom-b)/a;

					intersect[2] = bottomx;
					intersect[3] = rect_bottom;
					bar1 = (topx - measure[0]) / (measure[2]-measure[0]);
					bar2 = (bottomx - measure[0]) / (measure[2]-measure[0]);
				}
			}
		}
		else if(lefty <= rect_bottom)
		{
			intersect[0] = rect_left;
			intersect[1] = lefty;

			if(righty < rect_top) // left and top edges crossed
			{
				// compute top edge intersection
				float topx = (rect_top-b)/a;
				intersect[2] = topx;
				intersect[3] = rect_top;
				bar1 = (lefty - measure[1]) / (measure[3]-measure[1]);
				bar2 = (topx - measure[0]) / (measure[2]-measure[0]);
			}
			else if(righty <= rect_bottom) // left and right edges crossed
			{
				intersect[2] = rect_right;
				intersect[3] = righty;
				bar1 = (lefty - measure[1]) / (measure[3]-measure[1]);
				bar2 = (righty - measure[1]) / (measure[3]-measure[1]);
			}
			else // left and bottom edges crossed
			{
				// compute bottom edge intersection
				float bottomx = (rect_bottom-b)/a;
				intersect[2] = bottomx;
				intersect[3] = rect_right;
				bar1 = (lefty - measure[1]) / (measure[3]-measure[1]);
				bar2 = (bottomx - measure[0]) / (measure[2]-measure[0]);
			}
		}
		else
		{
			if(righty > rect_bottom) // no edge crossed
			{
					return false;
			}
			else // bottom edge crossed
			{
				// compute bottom edge intersection
				float bottomx = (rect_bottom-b)/a;
				intersect[0] = bottomx;
				intersect[1] = rect_left;

				if(righty >= rect_top) // right edge crossed
				{
					intersect[2] = rect_right;
					intersect[3] = righty;
					bar1 = (bottomx - measure[0]) / (measure[2]-measure[0]);
					bar2 = (righty - measure[1]) / (measure[3]-measure[1]);
				}
				else // top edge crossed 
				{
					// compute top edge intersection
					float topx = (rect_top-b)/a;
					intersect[2] = topx;
					intersect[3] = rect_right;
					bar1 = (bottomx - measure[0]) / (measure[2]-measure[0]);
					bar2 = (topx - measure[0]) / (measure[2]-measure[0]);
				}
			}
		}

		if((bar1 < 0 && bar2 < 0) || (bar1 > 1 && bar2 > 1))
		{
			return false;
		}
		else
		{
			vec2 meas1, meas2;
			meas1[0] = measure[0];
			meas1[1] = measure[1];
			meas2[0] = measure[2];
			meas2[1] = measure[3];

			if(bar1 > 0 && bar1 < 1) {
				measure[0] = intersect[0];
				measure[1] = intersect[1];
			}
			if(bar2 > 0 && bar2 < 1) {
				measure[2] = intersect[2];
				measure[3] = intersect[3];
			}
		}

		return true;
	}

	void randomizeSegment(jblas::vec4& segment);
	
	void projectOnPrediction(const jblas::vec4& meas, const jblas::vec4& exp, jblas::vec4& newMeas, float* stdRatio);

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
				double relevanceTh; ///< Mahalanobis distance for no information rejection
				double measStd;       ///<       measurement noise std deviation
				double measVar;       ///<       measurement noise variance
				double simuMeasStd;
			} params;
			LandmarkAbstract::geometry_t type;
			size_t size;
			MultiDimNormalDistribution *noise;

		public:
			MatcherSimu(LandmarkAbstract::geometry_t type, size_t size, int patchSize, int maxSearchSize, double lowInnov, double threshold, double mahalanobisTh, double relevanceTh, double measStd, double simuMeasStd):
				type(type), size(size), noise(NULL)
			{
				JFR_ASSERT(patchSize%2, "patchSize must be an odd number!");
				params.patchSize = patchSize;
				params.maxSearchSize = maxSearchSize;
				params.lowInnov = lowInnov;
				params.threshold = threshold;
				params.mahalanobisTh = mahalanobisTh;
				params.relevanceTh = relevanceTh;
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

					bool matched = false;
					vec2 meas1, meas2;

					switch(type)
					{
						case LandmarkAbstract::POINT :
							matched = roi.isIn(measure.x());
							break;
						case LandmarkAbstract::LINE :
						{
							/*
							meas1[0] = measure.x()[0] ; meas1[1] = measure.x()[1];
							meas2[0] = measure.x()[2] ; meas2[1] = measure.x()[3];
							matched = roi.isIn(meas1) || roi.isIn(meas2);
							*/
							vec meas(4);
							for(int i=0 ; i<4 ; i++)
								meas[i] = measure.x()[i];
							matched = segmentInRect(roi, meas);
							vec4 pred;
							vec4 obs;
							vec4 projected;
							float ratio;

							for(int i=0 ; i<4 ; i++)
								obs[i] = measure.x()[i];
								//obs[i] = meas[i];
							for(int i=0 ; i<4 ; i++)
								pred[i] = targetAppSpec->exp[i];
							//randomizeSegment(obs);
							appSpec->obs = obs;
							projectOnPrediction(obs,pred,projected,&ratio);

/*
							JFR_DEBUG("\n"
								<< "measure   " << measure.x() << "\n"
								<< "corrected " << meas << "\n"
								<< "projected " << projected << "\n"
								<< "for rect " << roi << "\n"
							);
*/

							// Debug, if not the same we do it again
							bool equal = true;
							for(int i=0 ;i<4 ; i++)
								if(fabs(obs[i] - projected[i]) > 0.001)
									equal = false;
							if(!equal)
							{
								projectOnPrediction(obs,pred,projected,&ratio);
							}

							for(int i=0 ; i<4 ; i++)
								measure.x()[i] = projected[i];
							measure.std(params.measStd * ratio);
						}
							break;
						default :
							break;
					}

					return matched;
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
					vec measurement = it->second->measurement.x();
					if (app->type == type)
					{
						bool push = false;
						vec2 meas1, meas2;

						switch(type)
						{
							case LandmarkAbstract::POINT :
								push = roi.isIn(measurement);
								break;
							case LandmarkAbstract::LINE :
								/*
								meas1[0] = measurement[0] ; meas1[1] = measurement[1];
								meas2[0] = measurement[2] ; meas2[1] = measurement[3];
								push = roi.isIn(meas1) || roi.isIn(meas2);
								*/
								app->obs = measurement;
								push = segmentInRect(roi, measurement);
//								app->obs = measurement;
//								it->second->measurement.x() = measurement;
								break;
							default :
								break;
						}

						if(push)
							roiObs.push_back(it->second);
					}
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
				//descPtr->addObservation(obsPtr);
				obsPtr->landmarkPtr()->setDescriptor(descPtr);
			}

	};
	
	
	
	
	
	
	
	
	
	
}}}

#endif

