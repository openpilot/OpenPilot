/*
 * dataManagerOnePointRansac.t.cpp
 *
 *     Project: jafar
 *  Created on: Jun 30, 2010
 *      Author: jsola
 */

#include "jmath/randomIntTmplt.hpp"
#include "jmath/misc.hpp"


#include "rtslam/dataManagerOnePointRansac.hpp"
#include "rtslam/kalmanTools.hpp"

#include "rtslam/rtSlam.hpp"
#include "rtslam/observationAbstract.hpp"

#include "rtslam/imageTools.hpp"

#define BUFFERED_UPDATE 1

namespace jafar {
	namespace rtslam {

		template<class RawSpec,class SensorSpec, class FeatureSpec, class RoiSpec, class FeatureManagerSpec, class DetectorSpec, class MatcherSpec>
		void DataManagerOnePointRansac<RawSpec,SensorSpec,FeatureSpec,RoiSpec,FeatureManagerSpec,DetectorSpec,MatcherSpec>::
		processKnownObs(boost::shared_ptr<RawSpec> rawData)
		{
// JFR_DEBUG("\n@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@\n");
			map_ptr_t mapPtr = sensorPtr()->robotPtr()->mapPtr();
			int numObs = 0;
			featMan->renew();

			// Project and isolate visible observations
			projectAndCollectVisibleObs();
// JFR_DEBUG_BEGIN(); JFR_DEBUG_SEND("######## Visible obs [" << obsVisibleList.size() << "]:");
// for(ObsList::iterator it = obsVisibleList.begin(); it != obsVisibleList.end(); ++it)
// 	JFR_DEBUG_SEND(" " << (*it)->id());
// JFR_DEBUG_END();

			int n_tries = algorithmParams.n_tries;
			if (obsVisibleList.size() < n_tries) n_tries = obsVisibleList.size();
// JFR_DEBUG("will try Ransac " << n_tries << " times");

			int current_try = 0;
			if (n_tries >= 2)
			while (current_try < n_tries)
			{
// JFR_DEBUG("#### go ransac try #" << current_try);
				// select random obs and match it
				observation_ptr_t obsBasePtr;
				getOneMatchedBaseObs(obsBasePtr, rawData);
				if (!obsBasePtr) break; // no more available matched obs
// JFR_DEBUG("chose matched obs " << obsBasePtr->id() << " as base");

				// 1b. base obs is now matched
				ransac_set_ptr_t ransacSetPtr(new RansacSet);
				ransacSetList.push_back(ransacSetPtr);
				ransacSetPtr->obsBasePtr = obsBasePtr;
				ransacSetPtr->inlierObs.push_back(obsBasePtr);

				current_try ++;
//JFR_DEBUG("old mean : " << mapManagerPtr()->mapPtr()->x());
				vec x_copy = updateMean(obsBasePtr);
// JFR_DEBUG("updated mean");//: " << x_copy);

				// for each other obs
				for(ObsList::iterator obsIter = obsVisibleList.begin(); obsIter != obsVisibleList.end(); obsIter++)
				{
					observation_ptr_t obsCurrentPtr = *obsIter;
					if (obsCurrentPtr == obsBasePtr) continue; // ignore the tested observation
// JFR_DEBUG("## testing obs " << obsCurrentPtr->id());

					// get obs things
					jblas::vec lmk = obsCurrentPtr->landmarkPtr()->state.x();
					vec exp(obsCurrentPtr->expectation.size());
					vec nobs(obsCurrentPtr->prior.size());

					// project
					projectFromMean(exp, obsCurrentPtr, x_copy);
// JFR_DEBUG("was predicted at " << obsCurrentPtr->expectation.x() << ", now predicted at " << exp);


					if(!obsCurrentPtr->events.matched){
						// try to match with low innovation
						obsCurrentPtr->predictAppearance();
						jblas::sym_mat P = jblas::identity_mat(obsCurrentPtr->expectation.size())*jmath::sqr(matcher->params.lowInnov);
						RoiSpec roi(exp, P, 1.0);
						
						obsCurrentPtr->events.measured = true;
						
// JFR_DEBUG("not yet matched, trying with lowInnov in roi " << roi);
						matcher->match(rawData, obsCurrentPtr->predictedAppearance, roi, obsCurrentPtr->measurement, obsCurrentPtr->observedAppearance);

						if (obsCurrentPtr->getMatchScore() > matcher->params.threshold && 
								isExpectedInnovationInlier(obsCurrentPtr, matcher->params.mahalanobisTh))
						{
							obsCurrentPtr->events.matched = true;
						}
					}
					
// JFR_DEBUG("matched: " << obsCurrentPtr->events.matched << " (measured " << obsCurrentPtr->events.measured << ") with score " << obsCurrentPtr->getMatchScore() << " at " << obsCurrentPtr->measurement.x());
					
					if(obsCurrentPtr->events.matched && isLowInnovationInlier(obsCurrentPtr, exp, matcher->params.lowInnov))
					{
						// declare inlier
						ransacSetPtr->inlierObs.push_back(obsCurrentPtr);
// JFR_DEBUG("push in inliers");
					}
					else{
						// declare pending
						ransacSetPtr->pendingObs.push_back(obsCurrentPtr);
// JFR_DEBUG("push in pendings");
					}

				} // for each other obs

// JFR_DEBUG_BEGIN(); JFR_DEBUG_SEND("#### Ransac set with base " << ransacSetPtr->obsBasePtr->id() << ":");
// for(ObsList::iterator it = ransacSetPtr->inlierObs.begin(); it != ransacSetPtr->inlierObs.end(); ++it)
// 	JFR_DEBUG_SEND(" " << (*it)->id());
// JFR_DEBUG_END();

			} // for i = 0:n_tries

			{
// JFR_DEBUG("######## Find best ransac set (among " << ransacSetList.size() << ")");
				ransac_set_ptr_t best_set;
				if (ransacSetList.size() != 0)
				{
					// 1. select ransacSet.inliers.size() max
					for(RansacSetList::iterator rsIter = ransacSetList.begin(); rsIter != ransacSetList.end(); ++rsIter)
						if (!best_set || (*rsIter)->size() > best_set->size()) best_set = *rsIter;
// JFR_DEBUG("best set is " << best_set->obsBasePtr->id() << " with size " << best_set->size());
					if (best_set->size() > 1)
					{
						// 2. for each obs in inliers
						for(ObsList::iterator obsIter = best_set->inlierObs.begin(); obsIter != best_set->inlierObs.end(); ++obsIter)
						{
							observation_ptr_t obsPtr = *obsIter;
							// Add to tesselation grid for active search
							//featMan->addObs(obsPtr->expectation.x());
							obsPtr->events.updated = true;
							numObs++;
							
							#if BUFFERED_UPDATE
							// 2a. add obs to buffer for EKF update
							mapPtr->filterPtr->stackCorrection(obsPtr->innovation, obsPtr->INN_rsl, obsPtr->ia_rsl);
// 	JFR_DEBUG("stacked correction for inlier " << obsPtr->id());
							#else
							obsPtr->project();
							obsPtr->computeInnovation();
							obsPtr->update();
// 	JFR_DEBUG("corrected inlier " << obsPtr->id());
							#endif
						}
						#if BUFFERED_UPDATE
						// 3. perform buffered update
						mapPtr->filterPtr->correctAllStacked(mapPtr->ia_used_states());
// 	JFR_DEBUG("corrected all stacked observations");
						#endif
					}
				}
				
				ObsList &activeSearchList = ((ransacSetList.size() == 0) || (best_set->size() <= 1) ? obsVisibleList : best_set->pendingObs);
				// FIXME don't search again landmarks that failed as base
				
				// 4. for each obs in pending: retake algorithm from active search
				obsListSorted.clear();
				
// JFR_DEBUG("######## Starting classic active search for the remaining");
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

// JFR_DEBUG("obs " << obsPtr->id() << " visible: " << obsPtr->isVisible());
					if (obsPtr->isVisible()) {

						// Add to tesselation grid for active search
						//featMan->addObs(obsPtr->expectation.x());

						// predict information gain
						obsPtr->predictInfoGain();

						// add to sorted list of observations
						obsListSorted[obsPtr->expectation.infoGain] = obsPtr;
// JFR_DEBUG("obs " << obsPtr->id() << " info gain " << obsPtr->expectation.infoGain);
					} // visible obs
				} // for each obs

// JFR_DEBUG("#### starting remaining corrections");
				// loop only the N_UPDATES most interesting obs, from largest info gain to smallest
				for (ObservationListSorted::reverse_iterator obsIter = obsListSorted.rbegin(); obsIter
						!= obsListSorted.rend(); obsIter++) {
					observation_ptr_t obsPtr = obsIter->second;

					// 1a. re-project to get up-to-date means and Jacobians
					obsPtr->project();

					// 1b. re-check visibility, just in case re-projection caused this obs to be invisible
					obsPtr->predictVisibility();
// JFR_DEBUG("obs " << obsPtr->id() << " visible : " << obsPtr->isVisible());
					if (obsPtr->isVisible()) {

						obsPtr->events.visible = true;

						if (numObs < algorithmParams.n_updates) {

							obsPtr->events.measured = true;

							// 1c. predict search area and appearance
							//cv::Rect roi = gauss2rect(obsPtr->expectation.x(), obsPtr->expectation.P() + matcherParams_.measVar*identity_mat(2), matcherParams_.mahalanobisTh);
							image::ConvexRoi roi(obsPtr->expectation.x(), obsPtr->expectation.P() + matcher->params.measVar*identity_mat(2), matcher->params.mahalanobisTh);
							obsPtr->predictAppearance();
// JFR_DEBUG("obs " << obsPtr->id() << " measured in " << roi);

							// 1d. match predicted feature in search area
							//						kernel::Chrono match_chrono;
							obsPtr->measurement.std(matcher->params.measStd);

							matcher->match(rawData, obsPtr->predictedAppearance, roi, obsPtr->measurement, obsPtr->observedAppearance);
							//						total_match_time += match_chrono.elapsedMicrosecond();

	/*
								// DEBUG: save some appearances to file
								((AppearanceImagePoint*)(((DescriptorImagePoint*)(obsPtr->landmark().descriptorPtr.get()))->featImgPntPtr->appearancePtr.get()))->patch.save("descriptor_app.png");
								((AppearanceImagePoint*)(obsPtr->predictedAppearance.get()))->patch.save("predicted_app.png");
								((AppearanceImagePoint*)(obsPtr->observedAppearance.get()))->patch.save("matched_app.png");
	*/
							// DEBUG: display predicted appearances on image, disable it when operating normally because can have side effects
	/*
							if (SHOW_PATCH) {
								AppearanceImagePoint * appImgPtr =
										PTR_CAST<AppearanceImagePoint*> (obsPtr->predictedAppearance.get());
								jblas::veci shift(2);
								shift(0) = (appImgPtr->patch.width() - 1) / 2;
								shift(1) = (appImgPtr->patch.height() - 1) / 2;
								appImgPtr->patch.robustCopy(*PTR_CAST<RawImage*> (senPtr->getRaw().get())->img, 0, 0,
																						obsPtr->expectation.x(0) - shift(0), obsPtr->expectation.x(1) - shift(1));
							}
	*/
// JFR_DEBUG("obs " << obsPtr->id() << " got match score " << obsPtr->getMatchScore());
							// 1e. if feature is found
							if (obsPtr->getMatchScore() > matcher->params.threshold) {
								obsPtr->events.matched = true;
								obsPtr->computeInnovation();

								// 1f. if feature is inlier
								if (obsPtr->compatibilityTest(matcher->params.mahalanobisTh)) { // use 3.0 for 3-sigma or the 5% proba from the chi-square tables.
									numObs++;
									//								kernel::Chrono update_chrono;
									obsPtr->update();
									//								total_update_time += update_chrono.elapsedMicrosecond();
									obsPtr->events.updated = true;
// JFR_DEBUG("corrected " << obsPtr->id());
								} // obsPtr->compatibilityTest(M_TH)
							} // obsPtr->getScoreMatchInPercent()>SC_TH

	//						cout << *obsPtr << endl;
						} // number of observations
					} // obsPtr->isVisible()

					// cout << "\n-------------------------------------------------- " << endl;
					// cout << *obsPtr << endl;

				} // foreach observation
// JFR_DEBUG("finished for this sensor !");
				obsListSorted.clear(); // clear the list now or it will prevent the observation to be destroyed until next frame, and will still be displayed
			}

			// update obs counters
			for(ObservationList::iterator obsIter = observationList().begin(); obsIter != observationList().end();obsIter++)
			{
				observation_ptr_t obs = *obsIter;
				if (obs->events.visible) JFR_ASSERT(obs->events.predicted, "obs visible without previous steps");
				if (obs->events.measured) JFR_ASSERT(obs->events.visible && obs->events.predicted, "obs measured without previous steps");
				if (obs->events.matched) JFR_ASSERT(obs->events.measured && obs->events.visible && obs->events.predicted, "obs matched without previous steps");
				if (obs->events.updated) JFR_ASSERT(obs->events.matched && obs->events.measured && obs->events.visible && obs->events.predicted, "obs updated without previous steps");
				
				if (obs->events.measured) obs->counters.nSearch++;
				if (obs->events.matched) obs->counters.nMatch++;
				if (obs->events.updated) obs->counters.nInlier++;
			}

			// clear all sets to liberate shared pointers
			ransacSetList.clear();
			obsVisibleList.clear();
			obsBaseList.clear();
			obsFailedList.clear();
		}

		#if 0
		template<>
		void
		DataManagerOnePointRansac<RawImage, SensorPinHole, QuickHarrisDetector, correl::FastTranslationMatcherZncc>::
		detectNewObs( boost::shared_ptr<RawImage> rawData )
		{
			for(int i = 0; i < 5; ++i)
			if (mapManagerPtr()->mapSpaceForInit()) {
				//boost::shared_ptr<RawImage> rawDataSpec = SPTR_CAST<RawImage>(rawData);
				ROI roi;
				if (asGrid->getRoi(roi)) {
					// FIXME if we already have searched some part of this roi without finding anything, we should not search again
					feat_img_pnt_ptr_t featPtr(new FeatureImagePoint(detectorParams_.patchSize,
							detectorParams_.patchSize,
							CV_8U));
					featPtr->measurement.std(detectorParams_.measStd);
					if (detector->detectIn(*(rawData->img.get()), featPtr, &roi)) {
						vec pix = featPtr->measurement.x();
						app_img_pnt_ptr_t appPtr = SPTR_CAST<AppearanceImagePoint>(featPtr->appearancePtr);

						// FIXME see if we can use detectorParams_.patchSize instead.
						cv::Size size = appPtr->patch.size();

						int shift_x = (size.width-1)/2;
						int shift_y = (size.height-1)/2;
						int x_src = pix(0)-shift_x;
						int y_src = pix(1)-shift_y;
						rawData->img->copy(appPtr->patch, x_src, y_src, 0, 0, size.width, size.height);

						//appPtr->patch.save("detected_feature.png");

						//						cout << "Detected pix: " << featPtr->measurement << endl;

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
						obsPtr->events.updated = false;
						obsPtr->measurement = featPtr->measurement;

						app_img_pnt_ptr_t app_src = SPTR_CAST<AppearanceImagePoint>(featPtr->appearancePtr);
						app_img_pnt_ptr_t app_dst = SPTR_CAST<AppearanceImagePoint>(obsPtr->observedAppearance);
						app_src->patch.copy(app_dst->patch, (app_src->patch.width()-app_dst->patch.width())/2,
								(app_src->patch.height()-app_dst->patch.height())/2, 0, 0,
								app_dst->patch.width(), app_dst->patch.height());

						// 2c. compute and fill stochastic data for the landmark
						obsPtr->backProject();

						// 2d. Create lmk descriptor
						vec7 globalSensorPose = sensorPtr()->globalPose();
						desc_img_pnt_ptr_t
						descPtr(new DescriptorImagePoint(featPtr, globalSensorPose,
								obsPtr));
						obsPtr->landmarkPtr()->setDescriptor(descPtr);

						asGrid->addObs(obsPtr->measurement.x());
					} // create&init
				} else break; // getRoi()
			} else break; // if space in map
		} // detect()
		#endif


		template<class RawSpec,class SensorSpec, class FeatureSpec, class RoiSpec, class FeatureManagerSpec, class DetectorSpec, class MatcherSpec>
		void DataManagerOnePointRansac<RawSpec,SensorSpec,FeatureSpec,RoiSpec,FeatureManagerSpec,DetectorSpec,MatcherSpec>::
		detectNewObs(boost::shared_ptr<RawSpec> rawData)
		{
			for(int i = 0; i < algorithmParams.n_init; ++i)
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
						obsPtr->events.updated = false;
						obsPtr->measurement = featPtr->measurement;

						// 2c. compute and fill stochastic data for the landmark
						obsPtr->backProject();

						// 2d. Create lmk descriptor
						detector->fillDataObs(featPtr, obsPtr);

						featMan->addObs(obsPtr->measurement.x());
					} // create&init
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

				obsPtr->project();
				obsPtr->predictVisibility();
				
				if (obsPtr->isVisible())
				{
					obsPtr->events.visible = true;
					featMan->addObs(obsPtr->expectation.x());
					obsVisibleList.push_back(obsPtr);
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
//JFR_DEBUG("robPose " << robPose << " was " << obsPtr->sensorPtr()->robotPtr()->pose.x());
			vec7 senPose;
			if (obsPtr->sensorPtr()->isInFilter) senPose = ublas::project(x, obsPtr->sensorPtr()->pose.ia());
			                                else senPose = obsPtr->sensorPtr()->pose.x();
//JFR_DEBUG("senPose " << senPose << " was " << obsPtr->sensorPtr()->pose.x());
			vec7 senGlobPose = quaternion::composeFrames(robPose, senPose);
//JFR_DEBUG("senGlobPose " << senGlobPose << " was " << obsPtr->sensorPtr()->globalPose());

			// project landmark
			vec lmk = ublas::project(x, obsPtr->landmarkPtr()->state.ia());
//JFR_DEBUG("lmk " << lmk << " was " << obsPtr->landmarkPtr()->state.x());
//(dbg)			obsPtr->project_func(obsPtr->sensorPtr()->globalPose(), obsPtr->landmarkPtr()->state.x(), exp, nobs);
//JFR_DEBUG("exp online " << exp);
			obsPtr->project_func(senGlobPose, lmk, exp, nobs);
//JFR_DEBUG("exp offline " << exp);

			// Assignments: 
			// (we should not modify obsPtr->expectation because it has already been computed with initial prediction and will be used for full/base search)
			//obsPtr->expectation.x() = exp;
			//obsPtr->expectation.nonObs = nobs;
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
			obsPtr->predictAppearance();
			image::ConvexRoi roi(obsPtr->expectation.x(), obsPtr->expectation.P() + obsPtr->measurement.P(), matcher->params.mahalanobisTh);
			matcher->match(rawData, obsPtr->predictedAppearance, roi, obsPtr->measurement, obsPtr->observedAppearance);

			return (obsPtr->getMatchScore() > matcher->params.threshold && isExpectedInnovationInlier(obsPtr, matcher->params.mahalanobisTh));
		}

		#if 0
		template<>
		bool DataManagerOnePointRansac<RawImage, SensorPinHole, QuickHarrisDetector, correl::FastTranslationMatcherZncc>::
		match(const boost::shared_ptr<RawImage> & rawPtr, const appearance_ptr_t & targetApp, image::ConvexRoi &roi, Measurement & measure, const appearance_ptr_t & app)
    {
			app_img_pnt_ptr_t targetAppImg = SPTR_CAST<AppearanceImagePoint>(targetApp);
			app_img_pnt_ptr_t appImg = SPTR_CAST<AppearanceImagePoint>(app);

			measure.matchScore = matcher->match(targetAppImg->patch, *(rawPtr->img),
					roi, measure.x()(0), measure.x()(1));
			/*
			measure.matchScore = correl::Explorer<correl::Zncc>::exploreTranslation(
					targetAppImg->patch, *(rawPtr->img), roi.x, roi.x+roi.width-1, 2, roi.y, roi.y+roi.height-1, 2,
					measure.x()(0), measure.x()(1));
			*/
			// set appearance
			// FIXME reenable this when Image::robustCopy will be fixed
//					rawPtr->img->robustCopy(appImg->patch, (int)(measure.x()(0)-0.5)-appImg->patch.width()/2,
//             (int)(measure.x()(1)-0.5)-appImg->patch.height()/2, 0, 0, appImg->patch.width(), appImg->patch.height());

			return true;
		}
		#endif


	} // namespace ::rtslam
}; // namespace jafar::

