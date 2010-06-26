/*
 * \file dataManagerActiveSearch.t.cpp
 *
 *  Created on: June 22, 2010
 *      Author: nmansard
 *      \ingroup rtslam
 */

#include "rtslam/dataManagerActiveSearch.hpp"
#include "rtslam/observationAbstract.hpp"
#include "rtslam/rawImage.hpp"
#include "rtslam/sensorPinHole.hpp"
#include "rtslam/descriptorImagePoint.hpp"

namespace jafar {
  namespace rtslam {

    template<class RawSpec,class SensorSpec, class Detector, class Matcher >
    void DataManagerActiveSearch<RawSpec,SensorSpec, Detector, Matcher >::
    processData( boost::shared_ptr<RawSpec> rawData )
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


				// 1b. re-check visibility, just in case re-projection caused this obs to be invisible
				obsPtr->predictVisibility();

				if (obsPtr->isVisible()) {

					if (numObs < N_UPDATES) {


						// update counter
						obsPtr->counters.nSearch++;


						// 1c. predict appearance
						obsPtr->predictAppearance();


						//						senPtr->dataManager.match(rawPtr, obsPtr);

						// 1d. search appearence in raw
						int xmin, xmax, ymin, ymax;
						double dx, dy;
						dx = 3.0 * sqrt(obsPtr->expectation.P(0, 0) + 1.0);
						dy = 3.0 * sqrt(obsPtr->expectation.P(1, 1) + 1.0);
						xmin = (int) (obsPtr->expectation.x(0) - dx);
						xmax = (int) (obsPtr->expectation.x(0) + dx + 0.9999);
						ymin = (int) (obsPtr->expectation.x(1) - dy);
						ymax = (int) (obsPtr->expectation.x(1) + dy + 0.9999);

						cv::Rect roi(xmin, ymin, xmax - xmin + 1, ymax - ymin + 1);

						cout << "roi: " << roi << endl;

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
						if (obsPtr->getMatchScore() > 0.90) {
							obsPtr->counters.nMatch++;
							obsPtr->events.matched = true;
							obsPtr->computeInnovation();


							// 1f. if feature is inlier
							if (obsPtr->compatibilityTest(3.0)) { // use 3.0 for 3-sigma or the 5% proba from the chi-square tables.
								numObs++;
								obsPtr->counters.nInlier++;
								//								kernel::Chrono update_chrono;
								obsPtr->update();
								//								total_update_time += update_chrono.elapsedMicrosecond();
								obsPtr->events.updated = true;
							} // obsPtr->compatibilityTest(3.0)

						} // obsPtr->getScoreMatchInPercent()>80

					} // number of observations

				} // obsPtr->isVisible()

				// cout << "\n-------------------------------------------------- " << endl;
				// cout << *obsPtr << endl;

			} // foreach observation

    }


    template<>
    bool DataManagerActiveSearch<RawImage, SensorPinHole, QuickHarrisDetector, correl::Explorer<correl::Zncc> >::
		match(const boost::shared_ptr<RawImage> & rawPtr, const appearance_ptr_t & targetApp, cv::Rect &roi, Measurement & measure, const appearance_ptr_t & app)
    {
					app_img_pnt_ptr_t targetAppImg = SPTR_CAST<AppearanceImagePoint>(targetApp);
					app_img_pnt_ptr_t appImg = SPTR_CAST<AppearanceImagePoint>(app);

					measure.matchScore = correl::Explorer<correl::Zncc>::exploreTranslation(
							targetAppImg->patch, *(rawPtr->img), roi.x, roi.x+roi.width-1, 1, roi.y, roi.y+roi.height-1, 1,
							measure.x()(0), measure.x()(1));

					// set appearance
					// FIXME reenable this when Image::robustCopy will be fixed
//					rawPtr->img->robustCopy(appImg->patch, (int)(measure.x()(0)-0.5)-appImg->patch.width()/2,
//             (int)(measure.x()(1)-0.5)-appImg->patch.height()/2, 0, 0, appImg->patch.width(), appImg->patch.height());

					return true;

    }

    //template<class SensorSpec>
    //void DataManagerActiveSearch<RawImage,SensorSpec>::
    // FIXME make this more abstract...
    template<>
    void DataManagerActiveSearch<RawImage, SensorPinHole, QuickHarrisDetector, correl::Explorer<correl::Zncc> >::
    detectNewData( boost::shared_ptr<RawImage> rawData )
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

						// 2a. Create the lmk and associated obs object.
						observation_ptr_t obsPtr =
						    mapManagerPtr()->createNewLandmark(shared_from_this());

						// 2b. fill data for this obs
						obsPtr->events.visible = true;
						obsPtr->events.measured = true;
						obsPtr->measurement.x(featPtr->measurement.x());

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
    void DataManagerActiveSearch<RawSpec,SensorSpec,Detector,Matcher>::
    process( boost::shared_ptr<RawAbstract> data )
    {
    	boost::shared_ptr<RawImage> dataSpec = SPTR_CAST<RawImage>(data);
      // 1. Observe known landmarks.
      processData(dataSpec); // process known landmarks
      // 2. Initialize new landmark.
      detectNewData(dataSpec);
    }

  }  // namespace ::rtslam
}; // namespace jafar::

