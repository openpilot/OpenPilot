/*
 * \file dataManagerActiveSearch.hpp
 *
 *  Created on: June 22, 2010
 *      Author: nmansard
 *      \ingroup rtslam
 */

#ifndef DATAMANAGERACTIVESEARCH_HPP_
#define DATAMANAGERACTIVESEARCH_HPP_

#include "rtslam/dataManagerAbstract.hpp"
#include "rtslam/activeSearch.hpp"
#include "rtslam/quickHarrisDetector.hpp"

namespace jafar {
  namespace rtslam {

    template< class RawSpec,class SensorSpec >
    class DataManagerActiveSearch
      : public DataManagerAbstract
      , public SpecificChildOf<SensorSpec>
    {
    public:
      // Define the function linkToParentSensorSpec.
      ENABLE_LINK_TO_SPECIFIC_PARENT(SensorAbstract,SensorSpec,
				     SensorSpec,DataManagerAbstract);
      // Define the functions sensorSpec() and sensorSpecPtr().
      ENABLE_ACCESS_TO_SPECIFIC_PARENT(SensorSpec,sensorSpec);

			//protected: // Parameters
    public: // TODO? Should be protected?
      static const int N_UPDATES = 1000;
      static const int PATCH_SIZE = 7;
			QuickHarrisDetector quickHarrisDetector;

    protected:
      boost::shared_ptr<ActiveSearchGrid> asGrid;
			// the list of observations sorted by information gain
      typedef map<double, observation_ptr_t> ObservationListSorted;
      ObservationListSorted obsListSorted;
      struct detector_params_t {
      		int convolutionMaskSize;
      		double cornernessThreshold;
      		double edgeRatioThreshold;
      		int patchSize;
      } detectorParams_;
      struct matcher_params_t {
      		int patchSize;
      		double regionSigma;
      		double threshold;
      } matcherParams_;
    public:
      void setDetectorParams(const int _convolutionSize, const double cornerTh, const double edgeRatio, const int patchSize){
      	detectorParams_.convolutionMaskSize = _convolutionSize;
      	detectorParams_.cornernessThreshold = cornerTh;
      	detectorParams_.edgeRatioThreshold = edgeRatio;
      	detectorParams_.patchSize = patchSize;
      }
      detector_params_t detectorParams(){return detectorParams_;}
      void setMatcherParams(const int patchSize, const double nSigma, const double threshold){
      	matcherParams_.patchSize = patchSize;
      	matcherParams_.regionSigma = nSigma;
      	matcherParams_.threshold = threshold;
      }
      matcher_params_t matcherParams(){return matcherParams_;}
      void setActiveSearchGrid( boost::shared_ptr<ActiveSearchGrid> arg ) { asGrid=arg; }
      boost::shared_ptr<ActiveSearchGrid> activeSearchGrid( void ) { return asGrid; }

    protected:
      void detectNewData( boost::shared_ptr<RawAbstract> data );
      void processData( boost::shared_ptr<RawAbstract> data );

    public:
      virtual ~DataManagerActiveSearch( void ) {}

      virtual void process( boost::shared_ptr<RawAbstract> data );

    };
  }
}//namespace jafar::rtslam


#include "rtslam/rawImage.hpp"
#include "rtslam/sensorPinHole.hpp"

namespace jafar {
  namespace rtslam {

    typedef DataManagerActiveSearch<RawImage,SensorPinHole> DataManagerActiveSearch_Image_PH;

  }
}//namespace jafar::rtslam


#include "rtslam/dataManagerActiveSearch.t.cpp"

#endif /* DATAMANAGERACTIVESEARCH_HPP_ */
