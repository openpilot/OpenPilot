/**
 * \file dataManagerOnePointRansac.impl.hpp
 *
 * \date 30/06/2010
 * \author jsola
 * \ingroup rtslam
 */
#include "kernel/misc.hpp"

#include "jmath/randomIntTmplt.hpp"
#include "jmath/misc.hpp"


#include "rtslam/dataManagerOnePointRansac.hpp"
#include "rtslam/kalmanTools.hpp"

#include "rtslam/rtSlam.hpp"
#include "rtslam/observationAbstract.hpp"

#include "rtslam/imageTools.hpp"

/*
 * STATUS: working fine, use it
 * Buffered update is faster than iterative update, but it doesn't allow
 * active search, so we use a mixed approach
 */
#define BUFFERED_UPDATE 1

/*
 * STATUS: working fine, use it
 * The goal is to improve performance by not computing jacobians
 * and covariance matrix for observations that are not visible
 * (but we compute twice the mean for those that are visible)
 * It brings approx 2% speedup
 */
#define PROJECT_MEAN_VISIBILITY 1

namespace jafar {
	namespace rtslam {

		template<class RawSpec,class SensorSpec, class FeatureSpec, class RoiSpec, class FeatureManagerSpec, class DetectorSpec, class MatcherSpec>
		void DataManagerOnePointRansac<RawSpec,SensorSpec,FeatureSpec,RoiSpec,FeatureManagerSpec,DetectorSpec,MatcherSpec>::
		processKnownObs(boost::shared_ptr<RawSpec> rawData)
		{
			
			//###
			//### Init, collect visible observations
			//### 
			map_ptr_t mapPtr = sensorPtr()->robotPtr()->mapPtr();
			unsigned numObs = 0;
			featMan->renew();

			projectAndCollectVisibleObs();

			unsigned n_tries = algorithmParams.n_tries;
			if (obsVisibleList.size() < n_tries) n_tries = obsVisibleList.size();

			
			//###
			//### Create the different Ransac sets
			//### 
			unsigned current_try = 0;
			if (n_tries >= 2)
			while (current_try < n_tries)
			{
				// select random obs and match it
				observation_ptr_t obsBasePtr;
				getOneMatchedBaseObs(obsBasePtr, rawData);
				if (!obsBasePtr) break; // no more available matched obs

				// 1b. base obs is now matched
				ransac_set_ptr_t ransacSetPtr(new RansacSet);
				ransacSetList.push_back(ransacSetPtr);
				ransacSetPtr->obsBasePtr = obsBasePtr;
				ransacSetPtr->inlierObs.push_back(obsBasePtr);

				current_try ++;
				vec x_copy = updateMean(obsBasePtr);

				// for each other obs
				for(ObsList::iterator obsIter = obsVisibleList.begin(); obsIter != obsVisibleList.end(); obsIter++)
				{
					observation_ptr_t obsCurrentPtr = *obsIter;
					if (obsCurrentPtr == obsBasePtr) continue; // ignore the tested observation

					// get obs things
					jblas::vec lmk = obsCurrentPtr->landmarkPtr()->state.x();
					vec exp(obsCurrentPtr->expectation.size());
					vec nobs(obsCurrentPtr->prior.size());

					// project
					projectFromMean(exp, obsCurrentPtr, x_copy);

					// FIXME rematch if isLowInnovationInlier is false
					if(!obsCurrentPtr->events.matched)
						if (obsCurrentPtr->predictAppearance())
						{
							// try to match with low innovation
							jblas::sym_mat P = jblas::identity_mat(obsCurrentPtr->expectation.size())*jmath::sqr(matcher->params.lowInnov);
							RoiSpec roi(exp, P, 1.0);
							obsCurrentPtr->searchSize = roi.count();
							
							obsCurrentPtr->events.measured = true;
							
							matcher->match(rawData, obsCurrentPtr->predictedAppearance, roi, obsCurrentPtr->measurement, obsCurrentPtr->observedAppearance);
							if (obsCurrentPtr->getMatchScore() > matcher->params.threshold)
							{
								#if PROJECT_MEAN_VISIBILITY
								obsCurrentPtr->project();
								#endif
								if (isExpectedInnovationInlier(obsCurrentPtr, matcher->params.mahalanobisTh))
								{
									obsCurrentPtr->events.matched = true;
								}
							}
						}
					
					if(obsCurrentPtr->events.matched && isLowInnovationInlier(obsCurrentPtr, exp, matcher->params.lowInnov))
					{
						// declare inlier
						ransacSetPtr->inlierObs.push_back(obsCurrentPtr);
					}
					else{
						// declare pending
						ransacSetPtr->pendingObs.push_back(obsCurrentPtr);
					}
				} // for each other obs
			} // for i = 0:n_tries


			//###
			//### Process the best Ransac set
			//### 
			ransac_set_ptr_t best_set;
			if (ransacSetList.size() != 0)
			{
				// 1. select ransacSet.inliers.size() max
				for(RansacSetList::iterator rsIter = ransacSetList.begin(); rsIter != ransacSetList.end(); ++rsIter)
					if (!best_set || (*rsIter)->size() > best_set->size()) best_set = *rsIter;

				// if there are too much updates to do bufferized, randomly move out some of them
				// to pending, they may be processed in active search if necessary
				while (best_set->size() > algorithmParams.n_updates_ransac)
				{
					int n = (rand() % (best_set->size() - 1)) + 1; // keep the first one which is the base obs
					best_set->pendingObs.push_back(best_set->inlierObs[n]);
					kernel::fastErase(best_set->inlierObs, n);
				}

				if (best_set->size() > 1)
				{
					// 2. for each obs in inliers
					JFR_DEBUG_BEGIN(); JFR_DEBUG_SEND("Updating with Ransac:");
					for(ObsList::iterator obsIter = best_set->inlierObs.begin(); obsIter != best_set->inlierObs.end(); ++obsIter)
					{
						observation_ptr_t obsPtr = *obsIter;
						// Add to tesselation grid for active search
						//featMan->addObs(obsPtr->expectation.x());
						obsPtr->events.updated = true;
						numObs++;
						JFR_DEBUG_SEND(" " << obsPtr->id());
						
						// 2a. add obs to buffer for EKF update
						#if BUFFERED_UPDATE
						mapPtr->filterPtr->stackCorrection(obsPtr->innovation, obsPtr->INN_rsl, obsPtr->ia_rsl);
						#else
						obsPtr->project();
						obsPtr->computeInnovation();
						obsPtr->update();
						#endif
					}
					#if BUFFERED_UPDATE
					// 3. perform buffered update
					mapPtr->filterPtr->correctAllStacked(mapPtr->ia_used_states());
					#endif
					JFR_DEBUG_END();
				}
			}
			
			//###
			//### Process some other observations with Active Search
			//### 
			ObsList &activeSearchList = ((ransacSetList.size() == 0) || (best_set->size() <= 1) ? obsVisibleList : best_set->pendingObs);
			// FIXME don't search again landmarks that failed as base
			
			JFR_DEBUG_BEGIN(); JFR_DEBUG_SEND("Updating with ActiveSearch:");
			for (unsigned i = 0; i < algorithmParams.n_recomp_gains; ++i)
			{
				// 4. for each obs in pending: retake algorithm from active search
				for(ObsList::iterator obsIter = activeSearchList.begin(); obsIter != activeSearchList.end(); ++obsIter)
				{
					observation_ptr_t obsPtr = *obsIter;
					// FIXME maybe don't clear events and don't rematch if already did, especially if didn't reestimate
					obsPtr->clearEvents();
					obsPtr->measurement.matchScore = 0;

					// 1a. project
					obsPtr->project();

					// 1b. check visibility
					obsPtr->predictVisibility();

					if (obsPtr->isVisible()) {

						// Add to tesselation grid for active search
						//featMan->addObs(obsPtr->expectation.x());
						
						/*
						quicly check if expectation has some negative variance values,
						to ignore the observation and prevent from crashing
						(it means that the filter is corrupted and that we should stop
						everything anyway)
						*/
						bool valid = true;
						for (unsigned i = 0; i < obsPtr->expectation.P().size1(); ++i)
							if (obsPtr->expectation.P()(i,i) < 0.0) { valid = false; break; }

						if (valid)
						{
							// predict information gain
							obsPtr->predictInfoGain();

							// add to sorted list of observations
							obsListSorted[obsPtr->expectation.infoGain] = obsIter;
						}
					} // visible obs
				} // for each obs

				// loop only the N_UPDATES most interesting obs, from largest info gain to smallest
				for (ObservationListSorted::reverse_iterator obsIter = obsListSorted.rbegin();
					obsIter != obsListSorted.rend(); ++obsIter)
				{
					if (i != algorithmParams.n_recomp_gains-1 && obsIter != obsListSorted.rbegin()) break;
					observation_ptr_t obsPtr = *(obsIter->second);

					// 1a. re-project to get up-to-date means and Jacobians
					obsPtr->project();

					// 1b. re-check visibility, just in case re-projection caused this obs to be invisible
					obsPtr->predictVisibility();
					if (obsPtr->isVisible())
					{
						obsPtr->events.visible = true;

						if (numObs < algorithmParams.n_updates_total)
							if (obsPtr->predictAppearance())
							{
								obsPtr->events.measured = true;

								// 1c. predict search area and appearance
								RoiSpec roi(obsPtr->expectation.x(), obsPtr->expectation.P() + matcher->params.measVar*identity_mat(2), matcher->params.mahalanobisTh);
								obsPtr->searchSize = roi.count();
								if (obsPtr->searchSize > matcher->params.maxSearchSize) roi.scale(sqrt(matcher->params.maxSearchSize/(double)obsPtr->searchSize));

								// 1d. match predicted feature in search area
								//						kernel::Chrono match_chrono;
								matcher->match(rawData, obsPtr->predictedAppearance, roi, obsPtr->measurement, obsPtr->observedAppearance);
								//						total_match_time += match_chrono.elapsedMicrosecond();

								// 1e. if feature is found
								if (obsPtr->getMatchScore() > matcher->params.threshold) {
									obsPtr->events.matched = true;
									obsPtr->computeInnovation();

									// 1f. if feature is inlier
									if (obsPtr->compatibilityTest(matcher->params.mahalanobisTh)) { // use 3.0 for 3-sigma or the 5% proba from the chi-square tables.
										numObs++;
										JFR_DEBUG_SEND(" " << obsPtr->id());
										//								kernel::Chrono update_chrono;
										obsPtr->update();
										//								total_update_time += update_chrono.elapsedMicrosecond();
										obsPtr->events.updated = true;
									} // obsPtr->compatibilityTest(M_TH)
								} // obsPtr->getScoreMatchInPercent()>SC_TH

							} // number of observations
					} // obsPtr->isVisible()
				} // foreach observation

				if (i+1 != algorithmParams.n_recomp_gains && obsListSorted.rbegin() != obsListSorted.rend())
					kernel::fastErase(activeSearchList, obsListSorted.rbegin()->second);
				obsListSorted.clear(); // clear the list now or it will prevent the observation to be destroyed until next frame, and will still be displayed
			}
			JFR_DEBUG_END();

			//###
			//### Update obs counters and some other stuff
			//### 
			for(ObservationList::iterator obsIter = observationList().begin(); obsIter != observationList().end();obsIter++)
			{
				observation_ptr_t obs = *obsIter;
				if (obs->events.visible) JFR_ASSERT(obs->events.predicted, "obs visible without previous steps");
				if (obs->events.measured) JFR_ASSERT(obs->events.visible && obs->events.predicted, "obs measured without previous steps");
				if (obs->events.matched) JFR_ASSERT(obs->events.measured && obs->events.visible && obs->events.predicted, "obs matched without previous steps");
				if (obs->events.updated) JFR_ASSERT(obs->events.matched && obs->events.measured && obs->events.visible && obs->events.predicted, "obs updated without previous steps");
				
				obs->updateDescriptor();
				#if VISIBILITY_MAP
				obs->updateVisibilityMap();
				#endif
				
				if (not (obs->events.measured && !obs->events.matched && !obs->isDescriptorValid()))
				{
					if (obs->events.measured) obs->counters.nSearch++;
					if (obs->events.matched) obs->counters.nMatch++;
					if (obs->events.updated) obs->counters.nInlier++;
				}
			}

			// clear all sets to liberate shared pointers
			ransacSetList.clear();
			obsVisibleList.clear();
			obsBaseList.clear();
			obsFailedList.clear();
		}


		template<class RawSpec,class SensorSpec, class FeatureSpec, class RoiSpec, class FeatureManagerSpec, class DetectorSpec, class MatcherSpec>
		void DataManagerOnePointRansac<RawSpec,SensorSpec,FeatureSpec,RoiSpec,FeatureManagerSpec,DetectorSpec,MatcherSpec>::
		detectNewObs(boost::shared_ptr<RawSpec> rawData)
		{
			for(unsigned i = 0; i < algorithmParams.n_init; )
			if (mapManagerPtr()->mapSpaceForInit()) {
				//boost::shared_ptr<RawImage> rawDataSpec = SPTR_CAST<RawImage>(rawData);
				RoiSpec roi;
				if (featMan->getRoi(roi)) {
					// FIXME if we already have searched some part of this roi without finding anything, we should not search again
					// this should be done in featMan, as long as no renew has been done, it remembers the getRoi, and detects when it is not followed by a addObs
					
					boost::shared_ptr<FeatureSpec> featPtr;
					if (detector->detect(rawData, roi, featPtr))
					{
						// 2a. Create the lmk and associated obs object.
						observation_ptr_t obsPtr =
								mapManagerPtr()->createNewLandmark(shared_from_this());

						// 2b. fill data for this obs
						obsPtr->counters.nSearch = 1;
						obsPtr->counters.nMatch = 1;
						obsPtr->counters.nInlier = 1;
						obsPtr->events.visible = true;
						obsPtr->events.predicted = false;
						obsPtr->events.measured = true;
						obsPtr->events.matched = false;
						obsPtr->events.updated = true;
						obsPtr->measurement = featPtr->measurement;

						// 2c. compute and fill stochastic data for the landmark
						obsPtr->backProject();

						// 2d. Create lmk descriptor
						detector->fillDataObs(featPtr, obsPtr);
						
						obsPtr->updateDescriptor();
						#if VISIBILITY_MAP
						obsPtr->updateVisibilityMap();
						#endif
						featMan->addObs(obsPtr->measurement.x());
						
//#ifndef JFR_NDEBUG
#if 0
						// check that point is correlated very close from the source (because of interpolation, and to check bugs)
						obsPtr->project();
						if (obsPtr->predictAppearance())
						{
							jblas::sym_mat P = jblas::identity_mat(obsPtr->expectation.size())*jmath::sqr(4.0);
							RoiSpec roi(obsPtr->expectation.x(), P, 1.0);
							obsPtr->searchSize = roi.count();
							matcher->match(rawData, obsPtr->predictedAppearance, roi, obsPtr->measurement, obsPtr->observedAppearance);
							JFR_ASSERT(ublas::norm_2(obsPtr->measurement.x()-obsPtr->expectation.x()) <= 0.01);
						}
#endif
						
						++i;
					} else // create&init
					{
						featMan->setFailed(roi);
					}
				} else break; // getRoi()
			} else break; // if space in map
		} // detect()


		template<class RawSpec,class SensorSpec, class FeatureSpec, class RoiSpec, class FeatureManagerSpec, class DetectorSpec, class MatcherSpec>
		void DataManagerOnePointRansac<RawSpec,SensorSpec,FeatureSpec,RoiSpec,FeatureManagerSpec,DetectorSpec,MatcherSpec>::
		process( boost::shared_ptr<RawAbstract> data )
		{
				boost::shared_ptr<RawSpec> dataSpec = SPTR_CAST<RawSpec>(data);
				// 1. Observe known landmarks.
				processKnownObs(dataSpec); // process known landmarks
				// 2. Initialize new landmark.
				detectNewObs(dataSpec);
		}


		template<class RawSpec,class SensorSpec, class FeatureSpec, class RoiSpec, class FeatureManagerSpec, class DetectorSpec, class MatcherSpec>
		void DataManagerOnePointRansac<RawSpec,SensorSpec,FeatureSpec,RoiSpec,FeatureManagerSpec,DetectorSpec,MatcherSpec>::
		projectAndCollectVisibleObs()
		{
			obsVisibleList.clear();

			for(ObservationList::iterator obsIter = observationList().begin(); obsIter != observationList().end();obsIter++)
			{
				observation_ptr_t obsPtr = *obsIter;

				obsPtr->clearEvents();
				obsPtr->measurement.matchScore = 0;

				#if PROJECT_MEAN_VISIBILITY
				obsPtr->projectMean();
				#else
				obsPtr->project();
				#endif
				
				if (obsPtr->predictVisibility())
				{
					#if VISIBILITY_MAP
					double visibility, viscertainty;
					obsPtr->landmark().visibilityMap.estimateVisibility(obsPtr, visibility, viscertainty);
					bool add = false;
					if (visibility > 0.75 && viscertainty > 0.75) add = true; else
					if (!obsPtr->landmark().converged)
						{ if (visibility < 0.25) visibility = 0.25; }
					else
						{ if (visibility < 0.1) visibility = 0.1; } // allow closing the loop! maybe look at neighbors
					if (viscertainty < 0.25) visibility = 0.25;
					if (rand()%1024 < visibility*1024) add = true;
					#else
					bool add = true;
					#endif
					
					if (add)
					{
						featMan->addObs(obsPtr->expectation.x());
						obsVisibleList.push_back(obsPtr);
					} 
					//else std::cout << __FILE__ << ":" << __LINE__ << " ignore lmk " << obsPtr->id() << std::endl;
				} // visible obs
			} // for each obs
			remainingObsCount = obsVisibleList.size();
		}


		template<class RawSpec,class SensorSpec, class FeatureSpec, class RoiSpec, class FeatureManagerSpec, class DetectorSpec, class MatcherSpec>
		void DataManagerOnePointRansac<RawSpec,SensorSpec,FeatureSpec,RoiSpec,FeatureManagerSpec,DetectorSpec,MatcherSpec>::
		getOneMatchedBaseObs(observation_ptr_t & obsBasePtr, boost::shared_ptr<RawSpec> rawData)
		{
			bool matchedBase = false;
			while (!matchedBase) {
				// select one random obs
//				obsBasePtr = selectOneRandomObs();
				if (remainingObsCount <= 0) { obsBasePtr.reset(); return; }
				int n = rand()%remainingObsCount;
				obsBasePtr = obsVisibleList[n];
// JFR_DEBUG("getOneMatchedBaseObs: trying obs " << obsBasePtr->id() << " already matched " << obsBasePtr->events.matched);
				// try to match (if not yet matched)
				if (!obsBasePtr->events.matched)
				{
					obsBasePtr->events.measured = true;

					if (matchWithExpectedInnovation(rawData, obsBasePtr)){
						obsBasePtr->events.matched = true;
					} else {
						obsFailedList.push_back(obsBasePtr);
					}
// JFR_DEBUG("getOneMatchedBaseObs: obs " << obsBasePtr->id() << " matched " << obsBasePtr->events.matched << " at " << obsBasePtr->measurement.x() << " predicted at " << obsBasePtr->expectation.x() << " with score " << obsBasePtr->getMatchScore());
				}
				if (obsBasePtr->events.matched)
				{
					obsBaseList.push_back(obsBasePtr);
					matchedBase = true;
				}
				
				--remainingObsCount;
				obsVisibleList[n] = obsVisibleList[remainingObsCount];
				obsVisibleList[remainingObsCount] = obsBasePtr;
			}
		}


		template<class RawSpec,class SensorSpec, class FeatureSpec, class RoiSpec, class FeatureManagerSpec, class DetectorSpec, class MatcherSpec>
		vec DataManagerOnePointRansac<RawSpec,SensorSpec,FeatureSpec,RoiSpec,FeatureManagerSpec,DetectorSpec,MatcherSpec>::
		updateMean(const observation_ptr_t & obsPtr)
		{
			// get map things
			vec x_copy = mapManagerPtr()->mapPtr()->x();
			ind_array ia_x = mapManagerPtr()->mapPtr()->ia_used_states();
			sym_mat &P = mapManagerPtr()->mapPtr()->P();

			// compute Kalman gain
			mat K(ia_x.size(), obsPtr->innovation.size());
			kalman::computeKalmanGain(P, ia_x, obsPtr->innovation, obsPtr->INN_rsl, obsPtr->ia_rsl, K);

			// perform state update to the mean, get temporary copy
			ublas::project(x_copy, ia_x) += ublas::prod(K, obsPtr->innovation.x());

			return x_copy;
		}


		template<class RawSpec,class SensorSpec, class FeatureSpec, class RoiSpec, class FeatureManagerSpec, class DetectorSpec, class MatcherSpec>
		void DataManagerOnePointRansac<RawSpec,SensorSpec,FeatureSpec,RoiSpec,FeatureManagerSpec,DetectorSpec,MatcherSpec>::
		projectFromMean(vec & exp, const observation_ptr_t & obsPtr, const vec & x)
		{
			vec nobs;

			// get global sensor pose
			vec7 robPose = ublas::project(x, obsPtr->sensorPtr()->robotPtr()->pose.ia());
			vec7 senPose;
			if (obsPtr->sensorPtr()->isInFilter) senPose = ublas::project(x, obsPtr->sensorPtr()->pose.ia());
			                                else senPose = obsPtr->sensorPtr()->pose.x();
			vec7 senGlobPose = quaternion::composeFrames(robPose, senPose);

			// project landmark
			vec lmk = ublas::project(x, obsPtr->landmarkPtr()->state.ia());
			obsPtr->model->project_func(senGlobPose, lmk, exp, nobs);

			// (we should not modify obsPtr->expectation because it has already been computed with initial prediction and will be used for full/base search)
		}


		template<class RawSpec,class SensorSpec, class FeatureSpec, class RoiSpec, class FeatureManagerSpec, class DetectorSpec, class MatcherSpec>
		bool DataManagerOnePointRansac<RawSpec,SensorSpec,FeatureSpec,RoiSpec,FeatureManagerSpec,DetectorSpec,MatcherSpec>::
		isLowInnovationInlier(const observation_ptr_t & obsPtr, const vec & exp, double lowInnTh)
		{
			vec inn;
			obsPtr->computeInnovationMean(inn, obsPtr->measurement.x(), exp);
			return (jmath::ublasExtra::norm_2(inn) < lowInnTh);
		}


		template<class RawSpec,class SensorSpec, class FeatureSpec, class RoiSpec, class FeatureManagerSpec, class DetectorSpec, class MatcherSpec>
		bool DataManagerOnePointRansac<RawSpec,SensorSpec,FeatureSpec,RoiSpec,FeatureManagerSpec,DetectorSpec,MatcherSpec>::
		isExpectedInnovationInlier( observation_ptr_t & obsPtr, double highInnTh)
		{
			obsPtr->computeInnovation();
			return (obsPtr->compatibilityTest(highInnTh));
		}


		template<class RawSpec,class SensorSpec, class FeatureSpec, class RoiSpec, class FeatureManagerSpec, class DetectorSpec, class MatcherSpec>
		bool DataManagerOnePointRansac<RawSpec,SensorSpec,FeatureSpec,RoiSpec,FeatureManagerSpec,DetectorSpec,MatcherSpec>::
		matchWithExpectedInnovation(boost::shared_ptr<RawSpec> rawData,  observation_ptr_t obsPtr)
		{
			#if PROJECT_MEAN_VISIBILITY
			obsPtr->project();
			#endif
			
			if (obsPtr->predictAppearance())
			{
				RoiSpec roi(obsPtr->expectation.x(), obsPtr->expectation.P() + matcher->params.measVar*identity_mat(2), matcher->params.mahalanobisTh);
				obsPtr->searchSize = roi.count();
				if (obsPtr->searchSize > matcher->params.maxSearchSize) roi.scale(sqrt(matcher->params.maxSearchSize/(double)obsPtr->searchSize));
				matcher->match(rawData, obsPtr->predictedAppearance, roi, obsPtr->measurement, obsPtr->observedAppearance);
// JFR_DEBUG("obs " << obsPtr->id() << " expected at " << obsPtr->expectation.x() << " measured with innovation " << obsPtr->measurement.x()-obsPtr->expectation.x());

				return (obsPtr->getMatchScore() > matcher->params.threshold && isExpectedInnovationInlier(obsPtr, matcher->params.mahalanobisTh));
			} else
				return false;
		}


	} // namespace ::rtslam
} // namespace jafar::

