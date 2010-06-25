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

    template<class RawSpec,class SensorSpec>
    void DataManagerActiveSearch<RawSpec,SensorSpec>::
    processData( boost::shared_ptr<RawAbstract> rawData )
    {
      int numObs = 0;
      asGrid->renew();

      for (ObservationList::iterator obsIter
	     = observationList().begin();
	   obsIter != observationList().end(); obsIter++)
	{
	  observation_ptr_t obsPtr = *obsIter;

	  obsPtr->clearEvents();
	  obsPtr->measurement.matchScore = 0;

	  // 1a. project
	  obsPtr->project();
	  // Add to tesselation grid for active search
	  asGrid->addPixel(obsPtr->expectation.x());

	  // 1b. check visibility
	  obsPtr->predictVisibility();
	  if (obsPtr->isVisible())
	    {
	      numObs ++;
	      if (numObs <= NUPDATES)
		{

		  // update counter
		  obsPtr->counters.nSearch++;

		  // 1c. predict appearance
		  obsPtr->predictAppearance();

		  // 1d. search appearence in raw
		  int xmin, xmax, ymin, ymax;
		  double dx, dy;
		  dx = 3.0*sqrt(obsPtr->expectation.P(0,0));
		  dy = 3.0*sqrt(obsPtr->expectation.P(1,1));
		  xmin = (int)(obsPtr->expectation.x(0)-dx);
		  xmax = (int)(obsPtr->expectation.x(0)+dx+0.9999);
		  ymin = (int)(obsPtr->expectation.x(1)-dy);
		  ymax = (int)(obsPtr->expectation.x(1)+dy+0.9999);

		  cv::Rect roi(xmin,ymin,xmax-xmin+1,ymax-ymin+1);

		  rawData->match(RawAbstract::ZNCC, obsPtr->predictedAppearance,
				 roi, obsPtr->measurement, obsPtr->observedAppearance);

		  // 1e. if feature is found
		  if (obsPtr->getMatchScore()>0.95)
		    {
		      obsPtr->counters.nMatch++;
		      obsPtr->events.matched = true;
		      obsPtr->computeInnovation() ;

		      // 1f. if feature is inlier
		      if (obsPtr->compatibilityTest(3.0))
			{ // use 3.0 for 3-sigma or the 5% proba from the chi-square tables.
			  obsPtr->counters.nInlier++;
			  obsPtr->update() ;
			  obsPtr->events.updated = true;
			} // obsPtr->compatibilityTest(3.0)
		    } // obsPtr->getScoreMatchInPercent()>80
		} // number of observations
	    } // obsPtr->isVisible()
	} // foreach observation
    }

    //template<class SensorSpec>
    //void DataManagerActiveSearch<RawImage,SensorSpec>::
    template<>
    void DataManagerActiveSearch<RawImage,SensorPinHole>::
    detectNewData( boost::shared_ptr<RawAbstract> rawData )
    {
      ROI roi;
      if (asGrid->getROI(roi))
	{
	  feat_img_pnt_ptr_t featPtr(new FeatureImagePoint(patchMatchSize*3,patchMatchSize*3,CV_8U));
	  if(rawData->detect(RawAbstract::HARRIS,featPtr,&roi))
	    {
	      // 2a. Create the lmk and associated obs object.
	      observation_ptr_t obsPtr = mapManagerPtr()->createNewLandmark( shared_from_this() );

	      // 2b. fill data for this obs
	      obsPtr->events.visible = true;
	      obsPtr->events.measured = true;
	      obsPtr->measurement.x(featPtr->measurement.x());

	      // 2c. compute and fill stochastic data for the landmark
	      obsPtr->backProject();

	      // 2d. Create lmk descriptor
	      vec7 globalSensorPose = sensorPtr()->globalPose();
	      desc_img_pnt_ptr_t descPtr(new DescriptorImagePoint(featPtr, globalSensorPose, obsPtr));
	      obsPtr->landmarkPtr()->setDescriptor(descPtr);
	    } // create&init
	} // getROI()
    } // detect()

    template<class RawSpec,class SensorSpec>
    void DataManagerActiveSearch<RawSpec,SensorSpec>::
    process( boost::shared_ptr<RawAbstract> data )
    {
      // 1. Observe known landmarks.
      processData(data); // process known landmarks
      // 2. Initialize new landmark.
      detectNewData(data);
    }

  }  // namespace ::rtslam
}; // namespace jafar::

