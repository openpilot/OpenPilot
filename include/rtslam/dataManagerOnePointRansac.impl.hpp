/*
 * dataManagerOnePointRansac.t.cpp
 *
 *     Project: jafar
 *  Created on: Jun 30, 2010
 *      Author: jsola
 */

#include "jmath/randomIntTmplt.hpp"

#include "correl/explorer.hpp"

#include "rtslam/dataManagerOnePointRansac.hpp"
#include "rtslam/kalmanTools.hpp"

#include "rtslam/rtSlam.hpp"
#include "rtslam/observationAbstract.hpp"
#include "rtslam/rawImage.hpp"
#include "rtslam/sensorPinHole.hpp"
#include "rtslam/descriptorImagePoint.hpp"

// TODO it should be possible to disable one point ransac in order to do
// basic active search with this same class, to avoid code duplication

namespace jafar {
	namespace rtslam {

		template<class RawSpec, class SensorSpec, class Detector, class Matcher>
		void
		DataManagerOnePointRansac<RawSpec, SensorSpec, Detector, Matcher>::
		processKnownObs( boost::shared_ptr<RawSpec> rawData)
			{
				map_ptr_t mapPtr = sensorPtr()->robotPtr()->mapPtr();
				int numObs = 0;
				asGrid->renew();

				boost::shared_ptr<RawSpec> rawSpecPtr = SPTR_CAST<RawSpec>(sensorPtr()->getRaw());

				// Project and isolate visible observations
				projectAndCollectVisibleObs();

				int current_try = 0;
				while (current_try < algorithmParams_.n_tries){

					// select random obs and match it
					observation_ptr_t obsBasePtr;
					getOneMatchedBaseObs(obsBasePtr, rawData);

					{// 1b. base obs is now matched

						ransac_set_ptr_t ransacSetPtr(new RansacSet);
						ransacSetList.push_back(ransacSetPtr);
						ransacSetPtr->obsBasePtr = obsBasePtr;

						current_try ++;
						vec x_copy = updateMean(obsBasePtr);

						// for each other obs
						for(ObsList::iterator obsIter = obsVisibleList.begin(); obsIter != obsVisibleList.end(); obsIter++)
						{
							observation_ptr_t obsCurrentPtr = *obsIter;
							if (obsCurrentPtr == obsBasePtr) continue; // ignore the tested observation

							// get obs things
							jblas::vec lmk = obsCurrentPtr->landmarkPtr()->state.x();
							vec pix(2);
							vec nobs(1);

							// project
							obsCurrentPtr->project_func(obsCurrentPtr->sensorPtr()->globalPose(), lmk, pix, nobs);

							// set low-innovation ROI
							cv::Rect roi(pix(0)-matcherParams_.regionPix/2, pix(1)-matcherParams_.regionPix/2, matcherParams_.regionPix, matcherParams_.regionPix);

							if(!obsCurrentPtr->events.matched){
								// try to match
								
								// FIXME only repredict if never done before
								obsBasePtr->predictAppearance();

								obsCurrentPtr->measurement.std(detectorParams_.measStd);

								match(rawSpecPtr, obsCurrentPtr->predictedAppearance, roi, obsCurrentPtr->measurement,
										obsCurrentPtr->observedAppearance);

								if ( obsCurrentPtr->getMatchScore() > matcherParams_.threshold ) {
									obsCurrentPtr->events.matched = true;
								}

							}

							if(obsCurrentPtr->events.matched)
							{
								// declare inlier
								ransacSetPtr->inlierObs.push_back(obsCurrentPtr);
							}
							else{
								// declare pending
								ransacSetPtr->pendingObs.push_back(obsCurrentPtr);
							}

						} // for each other obs

					} // base obs is matched

				} // for i = 0:n_tries

				{
					// 1. select ransacSet.inliers.size() max
					ransac_set_ptr_t best_set;
					for(RansacSetList::iterator rsIter = ransacSetList.begin(); rsIter != ransacSetList.end(); ++rsIter)
						if (!best_set || (*rsIter)->size() > best_set->size()) best_set = *rsIter;
					// 2. for each obs in inliers
					for(ObsList::iterator obsIter = best_set->inlierObs.begin(); obsIter != best_set->inlierObs.end(); ++obsIter)
					{
						observation_ptr_t obsPtr = *obsIter;
						// Add to tesselation grid for active search
						asGrid->addPixel(obsPtr->expectation.x());
						
						// 2a. add obs to buffer for EKF update
						mapPtr->filterPtr->stackCorrection(obsPtr->innovation, obsPtr->INN_rsl, obsPtr->ia_rsl);
						
					}
					// 3. perform buffered update
					mapPtr->filterPtr->correctAllStacked(mapPtr->ia_used_states());
					// 4. for each obs in pending: retake algorithm from active search
					obsListSorted.clear();
					for(ObsList::iterator obsIter = best_set->pendingObs.begin(); obsIter != best_set->pendingObs.end(); ++obsIter)
					{
						observation_ptr_t obsPtr = *obsIter;
						obsPtr->clearEvents();
						obsPtr->measurement.matchScore = 0;

						// 1a. project
						obsPtr->project();

						// 1b. check visibility
						obsPtr->predictVisibility();

						if (obsPtr->isVisible()) {

							// Add to tesselation grid for active search
							asGrid->addPixel(obsPtr->expectation.x());

							// predict information gain
							obsPtr->predictInfoGain();

							// add to sorted list of observations
							obsListSorted[obsPtr->expectation.infoGain] = obsPtr;

						} // visible obs
					} // for each obs

					// loop only the N_UPDATES most interesting obs, from largest info gain to smallest
					for (ObservationListSorted::reverse_iterator obsIter = obsListSorted.rbegin(); obsIter
							!= obsListSorted.rend(); obsIter++) {
						observation_ptr_t obsPtr = obsIter->second;

						// 1a. re-project to get up-to-date means and Jacobians
						obsPtr->project();

						obsPtr->events.predicted = true;

						// 1b. re-check visibility, just in case re-projection caused this obs to be invisible
						obsPtr->predictVisibility();

						if (obsPtr->isVisible()) {

							obsPtr->events.visible = true;

							if (numObs < algorithmParams_.n_updates) {

								obsPtr->events.measured = true;

								// update counter
								obsPtr->counters.nSearch++;

								// 1c. predict search area and appearance
								cv::Rect roi = gauss2rect(obsPtr->expectation.x(), obsPtr->expectation.P() + matcherParams_.measVar*identity_mat(2), matcherParams_.regionSigma);
								obsPtr->predictAppearance();

								// 1d. match predicted feature in search area
								//						kernel::Chrono match_chrono;
								obsPtr->measurement.std(detectorParams_.measStd);
								boost::shared_ptr<RawSpec> rawSpecPtr = SPTR_CAST<RawSpec>(sensorPtr()->getRaw());

								match(rawSpecPtr, obsPtr->predictedAppearance, roi, obsPtr->measurement,
																				obsPtr->observedAppearance);
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

								// 1e. if feature is found
								if (obsPtr->getMatchScore() > matcherParams_.threshold) {
									obsPtr->counters.nMatch++;
									obsPtr->events.matched = true;
									obsPtr->computeInnovation();

									// 1f. if feature is inlier
									if (obsPtr->compatibilityTest(matcherParams_.mahalanobisTh)) { // use 3.0 for 3-sigma or the 5% proba from the chi-square tables.
										numObs++;
										obsPtr->counters.nInlier++;
										//								kernel::Chrono update_chrono;
										obsPtr->update();
										//								total_update_time += update_chrono.elapsedMicrosecond();
										obsPtr->events.updated = true;
									} // obsPtr->compatibilityTest(M_TH)
								} // obsPtr->getScoreMatchInPercent()>SC_TH

		//						cout << *obsPtr << endl;
							} // number of observations
						} // obsPtr->isVisible()

						// cout << "\n-------------------------------------------------- " << endl;
						// cout << *obsPtr << endl;

					} // foreach observation

					obsListSorted.clear(); // clear the list now or it will prevent the observation to be destroyed until next frame, and will still be displayed
				}

				// clear all sets to liberate shared pointers
				ransacSetList.clear();
				obsVisibleList.clear();
			}

		template<>
		void
		DataManagerOnePointRansac<RawImage, SensorPinHole, QuickHarrisDetector, correl::Explorer<correl::Zncc> >::
		detectNewObs( boost::shared_ptr<RawImage> rawData )
		{
			if (mapManagerPtr()->mapSpaceForInit()) {
				//boost::shared_ptr<RawImage> rawDataSpec = SPTR_CAST<RawImage>(rawData);
				ROI roi;
				if (asGrid->getROI(roi)) {
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

					} // create&init
				} // getROI()
			} // if space in map
		} // detect()

		template<class RawSpec,class SensorSpec, class Detector, class Matcher>
		void
		DataManagerOnePointRansac<RawSpec,SensorSpec,Detector,Matcher>::
		process( boost::shared_ptr<RawAbstract> data )
		{
				boost::shared_ptr<RawImage> dataSpec = SPTR_CAST<RawImage>(data);
				// 1. Observe known landmarks.
				processKnownObs(dataSpec); // process known landmarks
				// 2. Initialize new landmark.
				detectNewObs(dataSpec);
		}


		template<class RawSpec, class SensorSpec, class Detector, class Matcher>
		void
		DataManagerOnePointRansac<RawSpec, SensorSpec, Detector, Matcher>::
		projectAndCollectVisibleObs() {
			obsVisibleList.clear();

			for(ObservationList::iterator obsIter = observationList().begin(); obsIter != observationList().end();obsIter++) {

				observation_ptr_t obsPtr = *obsIter;

				obsPtr->clearEvents();
				obsPtr->measurement.matchScore = 0;

				obsPtr->project();
				obsPtr->events.predicted = true;

				obsPtr->predictVisibility();
				if (obsPtr->isVisible()) {
					obsPtr->events.visible = true;
					asGrid->addPixel(obsPtr->expectation.x());
					// 1c. add to visible list
					obsVisibleList.push_back(obsPtr);
				} // visible obs
			} // for each obs

		}

		template<class RawSpec, class SensorSpec, class Detector, class Matcher>
		void
		DataManagerOnePointRansac<RawSpec, SensorSpec, Detector, Matcher>::
		getOneMatchedBaseObs(observation_ptr_t & obsBasePtr, boost::shared_ptr<RawSpec> rawData){
			bool matchedBase = false;
			while (!matchedBase){

				// select one random obs
				obsBasePtr = selectOneRandomObs();

				// try to match (if not yet matched)
				if (!obsBasePtr->events.matched)
				{
					obsBasePtr->counters.nSearch++;

					if (matchWithExpectedInnovation(rawData, obsBasePtr)){
						obsBasePtr->events.matched = true;
						obsBasePtr->counters.nMatch++;
						obsBaseList.push_back(obsBasePtr);
					}else{
						obsFailedList.push_back(obsBasePtr);
					}
				} else {
					obsBaseList.push_back(obsBasePtr);
				}
				matchedBase = obsBasePtr->events.matched;
			}
		}


		template<class RawSpec, class SensorSpec, class Detector, class Matcher>
		observation_ptr_t
		DataManagerOnePointRansac<RawSpec, SensorSpec, Detector, Matcher>::
		selectOneRandomObs() {
				// FIXME think about creating a lnew list of non-tested obs to make this search more efficient and robust.
			int N = obsVisibleList.size();
			int n;
			bool good = false;
			while (!good) {
				n = rand()%N;
				good = true;
				for (ObsList::iterator obsIter = obsBaseList.begin(); obsIter != obsBaseList.end(); obsIter++) {
					if (obsVisibleList[n] == *obsIter) {
						good = false;
						break;
					}
				}
				if (!good) break;
				for (ObsList::iterator obsIter = obsFailedList.begin(); obsIter != obsFailedList.end(); obsIter++) {
					if (obsVisibleList[n] == *obsIter) {
						good = false;
						break;
					}
				}
			}
			return obsVisibleList[n];
		}

		template<class RawSpec, class SensorSpec, class Detector, class Matcher>
		vec
		DataManagerOnePointRansac<RawSpec, SensorSpec, Detector, Matcher>::
		updateMean(const observation_ptr_t & obsPtr) {

			// get map things
			vec x_copy = mapManagerPtr()->mapPtr()->x();
			ind_array ia_x = mapManagerPtr()->mapPtr()->ia_used_states();
			sym_mat &P = mapManagerPtr()->mapPtr()->P();

			// compute Kalman gain
			obsPtr->computeInnovation();
			mat K(ia_x.size(), obsPtr->innovation.size());
			kalman::computeKalmanGain(P, ia_x, obsPtr->innovation, obsPtr->INN_rsl, obsPtr->ia_rsl, K);

			// perform state update to the mean, get temporary copy
			ublas::project(x_copy, ia_x) += ublas::prod(K, obsPtr->innovation.x());

			return x_copy;
		}

		template<class RawSpec, class SensorSpec, class Detector, class Matcher>
		void
		DataManagerOnePointRansac<RawSpec, SensorSpec, Detector, Matcher>::
		projectFromMean( observation_ptr_t & obsPtr, const vec & x){
			vec exp, nobs;

			// get global sensor pose and jacobians wrt x
			vec7 robPose = ublas::project(x, obsPtr->sensorPtr()->robotPtr()->pose.ia());
			vec7 senPose = ublas::project(x, obsPtr->sensorPtr()->pose.ia());
			vec7 senGlobPose = quaternion::composeFrames(robPose, senPose);

			// project landmark
			vec lmk = ublas::project(x, obsPtr->landmarkPtr()->state.ia());
			obsPtr->project_func(senGlobPose, lmk, exp, nobs);

			// Assignments:
			obsPtr->expectation.x() = exp;
			obsPtr->expectation.nonObs = nobs;
		}


		template<class RawSpec, class SensorSpec, class Detector, class Matcher>
		bool
		DataManagerOnePointRansac<RawSpec, SensorSpec, Detector, Matcher>::
		isLowInnovationInlier(const observation_ptr_t & obsPtr, double lowInnTh){
				return (jmath::ublasExtra::norm_2(obsPtr->measurement.x()-obsPtr->expectation.x()) < lowInnTh);
		}

		template<class RawSpec, class SensorSpec, class Detector, class Matcher>
		bool
		DataManagerOnePointRansac<RawSpec, SensorSpec, Detector, Matcher>::
		isExpectedInnovationInlier( observation_ptr_t & obsPtr, double highInnTh){
				obsPtr->computeInnovation();
				return (obsPtr->compatibilityTest(highInnTh));
		}

		template<class RawSpec, class SensorSpec, class Detector, class Matcher>
		bool
		DataManagerOnePointRansac<RawSpec, SensorSpec, Detector, Matcher>::
		matchWithExpectedInnovation(boost::shared_ptr<RawSpec> rawData,  observation_ptr_t obsPtr){
			obsPtr->predictAppearance();
			obsPtr->measurement.std(detectorParams_.measStd);
			cv::Rect roi = gauss2rect(
					obsPtr->expectation.x(),
			    obsPtr->expectation.P() + obsPtr->measurement.P(),
			    matcherParams_.regionSigma);
			match(
					rawData,
					obsPtr->predictedAppearance,
					roi,
					obsPtr->measurement,
					obsPtr->observedAppearance);

			return (obsPtr->getMatchScore() > matcherParams_.threshold);

		}

		template<>
		bool DataManagerOnePointRansac<RawImage, SensorPinHole, QuickHarrisDetector, correl::Explorer<correl::Zncc> >::
		match(const boost::shared_ptr<RawImage> & rawPtr, const appearance_ptr_t & targetApp, cv::Rect &roi, Measurement & measure, const appearance_ptr_t & app)
    {
			app_img_pnt_ptr_t targetAppImg = SPTR_CAST<AppearanceImagePoint>(targetApp);
			app_img_pnt_ptr_t appImg = SPTR_CAST<AppearanceImagePoint>(app);

			measure.matchScore = correl::Explorer<correl::Zncc>::exploreTranslation(
					targetAppImg->patch, *(rawPtr->img), roi.x, roi.x+roi.width-1, 2, roi.y, roi.y+roi.height-1, 2,
					measure.x()(0), measure.x()(1));

			// set appearance
			// FIXME reenable this when Image::robustCopy will be fixed
//					rawPtr->img->robustCopy(appImg->patch, (int)(measure.x()(0)-0.5)-appImg->patch.width()/2,
//             (int)(measure.x()(1)-0.5)-appImg->patch.height()/2, 0, 0, appImg->patch.width(), appImg->patch.height());

			return true;
		}


	} // namespace ::rtslam
}; // namespace jafar::

