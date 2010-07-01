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

#include "rtslam/observationAbstract.hpp"
#include "rtslam/rawImage.hpp"
#include "rtslam/sensorPinHole.hpp"
#include "rtslam/descriptorImagePoint.hpp"

namespace jafar {
	namespace rtslam {

		template<class RawSpec, class SensorSpec, class Detector, class Matcher>
		void DataManagerOnePointRansac<RawSpec, SensorSpec, Detector, Matcher>::processKnownObs(
		    boost::shared_ptr<RawSpec> rawData) {
			int numObs = 0;
			asGrid->renew();
			obsVisibleList.clear();

			// cast raw data
			boost::shared_ptr<RawSpec> rawSpecPtr =
			    SPTR_CAST<RawSpec>(sensorPtr()->getRaw());

			    // loop all observations
			    for (ObservationList::iterator obsIter = observationList().begin();
			    		obsIter	!= observationList().end();
			    		obsIter++) {

					observation_ptr_t obsPtr = *obsIter;

					obsPtr->clearEvents();
					obsPtr->measurement.matchScore = 0;

					// 1a. project
					obsPtr->project();
					obsPtr->events.predicted = true;

					// 1b. check visibility
					obsPtr->predictVisibility();
					if (obsPtr->isVisible()) {
						obsPtr->events.visible = true;
						asGrid->addPixel(obsPtr->expectation.x());
						// 1c. add to visible list
						obsVisibleList.push_back(obsPtr);
					} // visible obs
				} // for each obs

				// Select N visible observations for RANSAC tries:
				jmath::RandomIntVectTmplt<int> randomizer(algorithmParams_.n_tries, 0, obsVisibleList.size());
				tries = randomizer.getDifferent(); // Get n_tries indices to selected obs.

				// start ransac test sets
				//				for (RansacSetList::iterator ransacIter = ransacSetList.begin();
				//						ransacIter != ransacSetList.end();
				//						ransacIter++){

				{ observation_ptr_t obsBasePtr; // FIXME this line goes out, need for() loop above
					// get base observation
					size_t i;
					//					observation_ptr_t obsBasePtr = obsListVisible;

					// get map things
					vec x = mapManagerPtr()->mapPtr()->x();
					ind_array ia_x = mapManagerPtr()->mapPtr()->ia_used_states();

					// 1. match the hypothesis
					if (!obsBasePtr->events.matched) // 1a. not matched yet: match!
					{
						// ii. match this obs
						cv::Rect roi = gauss2rect(obsBasePtr->expectation.x(), obsBasePtr->expectation.P() + matcherParams_.measVar*identity_mat(2), matcherParams_.regionSigma);
						obsBasePtr->predictAppearance();

						// 1d. match predicted feature in search area
						obsBasePtr->measurement.std(detectorParams_.measStd);
						boost::shared_ptr<RawSpec> rawSpecPtr = SPTR_CAST<RawSpec>(sensorPtr()->getRaw());

						match(rawSpecPtr, obsBasePtr->predictedAppearance, roi, obsBasePtr->measurement,
								obsBasePtr->observedAppearance);
						if (obsBasePtr->getMatchScore() > matcherParams_.threshold) {
							obsBasePtr->counters.nMatch++;
							obsBasePtr->events.matched = true;
						} // else : TODO see what happens when the base obs does not match !

					}

					if (obsBasePtr->events.matched) { // 1b. base obs is now matched

						vec x_copy = x;

						// compute Kalman gain
						obsBasePtr->computeInnovation();
						mat K(mapManagerPtr()->mapPtr()->ia_used_states().size(), obsBasePtr->innovation.size());
						mapManagerPtr()->mapPtr()->filterPtr->computeKalmanGain(ia_x,obsBasePtr->innovation, obsBasePtr->INN_rsl, obsBasePtr->ia_rsl);
						kalman::computeKalmanGain(mapManager().map().P(),ia_x, obsBasePtr->innovation, obsBasePtr->INN_rsl,obsBasePtr->ia_rsl, K);

						// perform state update to the mean, get temporary copy
						ublas::project(x_copy, ia_x) += ublas::prod(K , obsBasePtr->innovation.x());

						// for each other obs
						for(ObservationVisibleList::iterator obsIter = obsVisibleList.begin(); obsIter != obsVisibleList.end(); obsIter++)
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
							cv::Rect roi(pix(0) - 1, pix(1) -1, 3, 3);

							if(!obsCurrentPtr->events.matched){
								// try to match

								obsBasePtr->predictAppearance();

								obsCurrentPtr->measurement.std(detectorParams_.measStd);

								match(rawSpecPtr, obsCurrentPtr->predictedAppearance, roi, obsCurrentPtr->measurement,
										obsCurrentPtr->observedAppearance);

								if ( obsCurrentPtr->getMatchScore() > matcherParams_.threshold ) {
									obsCurrentPtr->events.matched = true;
								}

							}if(1/* inside ROI */){
							}else{
								// ransacSetList(i).pendingObs.add(obs)

							} // if match

						} // for each other obs

					} // already matched obs

				} // for i = 0:n_tries

				// clear all sets to liberate shared pointers
				ransacSetList.clear();
				obsVisibleList.clear();

		}

			//    template<>
			//    bool DataManagerOnePointRansac<RawImage, SensorPinHole, QuickHarrisDetector, correl::Explorer<correl::Zncc> >::
			//		match(const boost::shared_ptr<RawImage> & rawPtr, const appearance_ptr_t & targetApp, cv::Rect &roi, Measurement & measure, const appearance_ptr_t & app)
			//    {
			//					app_img_pnt_ptr_t targetAppImg = SPTR_CAST<AppearanceImagePoint>(targetApp);
			//					app_img_pnt_ptr_t appImg = SPTR_CAST<AppearanceImagePoint>(app);
			//
			//					measure.matchScore = correl::Explorer<correl::Zncc>::exploreTranslation(
			//							targetAppImg->patch, *(rawPtr->img), roi.x, roi.x+roi.width-1, 2, roi.y, roi.y+roi.height-1, 2,
			//							measure.x()(0), measure.x()(1));
			//
			//					// set appearance
			//					// FIXME reenable this when Image::robustCopy will be fixed
			////					rawPtr->img->robustCopy(appImg->patch, (int)(measure.x()(0)-0.5)-appImg->patch.width()/2,
			////             (int)(measure.x()(1)-0.5)-appImg->patch.height()/2, 0, 0, appImg->patch.width(), appImg->patch.height());
			//
			//					return true;
			//
			//    }


			//template<class SensorSpec>
			//void DataManagerActiveSearch<RawImage,SensorSpec>::
			// FIXME make this more abstract...
			template<>
			void DataManagerOnePointRansac<RawImage, SensorPinHole, QuickHarrisDetector, correl::Explorer<correl::Zncc> >::
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
							//						obsPtr->measurement.x(featPtr->measurement.x());
							//						obsPtr->measurement.P(featPtr->measurement.P());
							//						obsPtr->measurement.matchScore = featPtr->measurement.matchScore;

							//						cout << "Measured pix: " << obsPtr->measurement << endl;

							//						obsPtr->setup(sensorSpecPtr()->params.pixNoise, matcherParams_.patchSize);

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
			void DataManagerOnePointRansac<RawSpec,SensorSpec,Detector,Matcher>::
			process( boost::shared_ptr<RawAbstract> data )
			{
				boost::shared_ptr<RawImage> dataSpec = SPTR_CAST<RawImage>(data);
				// 1. Observe known landmarks.
				processKnownObs(dataSpec); // process known landmarks
				// 2. Initialize new landmark.
				detectNewObs(dataSpec);
			}

		} // namespace ::rtslam
	}; // namespace jafar::

