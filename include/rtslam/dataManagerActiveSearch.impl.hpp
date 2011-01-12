/**
 * \file dataManagerActiveSearch.impl.hpp
 *
 * \date 22/06/2010
 * \author nmansard
 * \ingroup rtslam
 */
#if 0
#include "rtslam/dataManagerActiveSearch.hpp"
#include "rtslam/observationAbstract.hpp"
#include "rtslam/rawImage.hpp"
#include "rtslam/sensorPinHole.hpp"
#include "rtslam/descriptorImagePoint.hpp"

#include "rtslam/imageTools.hpp"

namespace jafar {
  namespace rtslam {

    template<class RawSpec,class SensorSpec, class Detector, class Matcher >
    void DataManagerActiveSearch<RawSpec,SensorSpec, Detector, Matcher >::
    processKnownObs( boost::shared_ptr<RawSpec> rawData )
    {
			int numObs = 0;
			asGrid->renew();
			obsListSorted.clear();

			// loop all observations
			for (ObservationList::iterator obsIter = observationList().begin(); obsIter
					!= observationList().end(); obsIter++) {
				observation_ptr_t obsPtr = *obsIter;

				obsPtr->clearEvents();
				obsPtr->measurement.matchScore = 0;


				// 1a. project
				obsPtr->project();

				// 1b. check visibility
				obsPtr->predictVisibility();

				if (obsPtr->isVisible()) {


					// Add to tesselation grid for active search
					asGrid->addObs(obsPtr->expectation.x());


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
						//cv::Rect roi = gauss2rect(obsPtr->expectation.x(), obsPtr->expectation.P() + matcherParams_.measVar*identity_mat(2), matcherParams_.regionSigma);
						image::ConvexRoi roi(obsPtr->expectation.x(), obsPtr->expectation.P() + matcherParams_.measVar*identity_mat(2), matcherParams_.regionSigma);
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


    template<>
    bool DataManagerActiveSearch<RawImage, SensorPinHole, QuickHarrisDetector, correl::FastTranslationMatcherZncc>::
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

//    template<>
//    bool DataManagerActiveSearch<RawImage, SensorPinHole, QuickHarrisDetector, correl::Explorer<correl::Zncc> >::
//		detect(const boost::shared_ptr<RawImage> & rawPtr, cv::Rect &roi)
//    {
//			feat_img_pnt_ptr_t featPtr(new FeatureImagePoint(detectorParams_.patchSize,
//			                                                 detectorParams_.patchSize,
//			                                                 CV_8U));
//
//			featPtr->measurement.std(detectorParams_.measStd);
//			if (detector->detectIn(*(rawData->img.get()), featPtr, &roi)) {
//				vec pix = featPtr->measurement.x();
//				extractAppearance(pix, featPtr->appearancePtr);
//
//  			app_img_pnt_ptr_t app = SPTR_CAST<AppearanceImagePoint>(appPtr);
//  			cv::Size size = app->patch.size();
//  			int shift_x = (size.width-1)/2;
//  			int shift_y = (size.height-1)/2;
//  			int x_src = pix(0)-shift_x;
//  			int y_src = pix(1)-shift_y;
//  			img->copy(app->patch, x_src, y_src, 0, 0, size.width, size.height);
//
//
//((AppearanceImagePoint*)(featPtr->appearancePtr.get()))->patch.save("detected_feature.png");
//((AppearanceImagePoint*)(featPtr->appearancePtr.get()))->patch.save("detected_feature.png");
//				cout << "Detected pix: " << featPtr->measurement << endl;
//
//				return true;
//
//    }

    //template<class SensorSpec>
    //void DataManagerActiveSearch<RawImage,SensorSpec>::
    // FIXME make this more abstract...
    template<>
    void DataManagerActiveSearch<RawImage, SensorPinHole, QuickHarrisDetector, correl::FastTranslationMatcherZncc>::
    detectNewObs( boost::shared_ptr<RawImage> rawData )
    {
    	if (mapManagerPtr()->mapSpaceForInit()) {
    		//boost::shared_ptr<RawImage> rawDataSpec = SPTR_CAST<RawImage>(rawData);
				ROI roi;
				if (asGrid->getRoi(roi)) {
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
						    descPtr(new DescriptorImagePointFirstView(featPtr, globalSensorPose,
						                                     obsPtr));
						obsPtr->landmarkPtr()->setDescriptor(descPtr);

					} // create&init
				} // getRoi()
			} // if space in map
    } // detect()

    template<class RawSpec,class SensorSpec, class Detector, class Matcher>
    void DataManagerActiveSearch<RawSpec,SensorSpec,Detector,Matcher>::
    process( boost::shared_ptr<RawAbstract> data )
    {
    	boost::shared_ptr<RawImage> dataSpec = SPTR_CAST<RawImage>(data);
      // 1. Observe known landmarks.
      processKnownObs(dataSpec); // process known landmarks
      // 2. Initialize new landmark.
      detectNewObs(dataSpec);
    }

  }  // namespace ::rtslam
}; // namespace jafar::

#endif
