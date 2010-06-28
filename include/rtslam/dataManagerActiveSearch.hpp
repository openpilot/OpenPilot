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

namespace jafar {
  namespace rtslam {

    template< class RawSpec,class SensorSpec, class Detector, class Matcher >
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


    protected:
			boost::shared_ptr<Detector> detector;
			boost::shared_ptr<Matcher> matcher;
      boost::shared_ptr<ActiveSearchGrid> asGrid;
			// the list of observations sorted by information gain
      typedef map<double, observation_ptr_t> ObservationListSorted;
      ObservationListSorted obsListSorted;
      struct detector_params_t {
      		int patchSize;
      		double measStd;
      } detectorParams_;
      struct matcher_params_t {
      		int patchSize;
      		double regionSigma;
      		double threshold;
      		double mahalanobisTh;
      		double measStd;
      		double measVar;
      } matcherParams_;
      struct alg_params_t {
      		int n_updates;
      } algorithmParams_;
    public:
      void setDetector(const boost::shared_ptr<Detector> & _detector, int patchSize, double _measStd){
      	detector = _detector;
      	detectorParams_.patchSize = patchSize;
      	detectorParams_.measStd = _measStd;
      }
      detector_params_t detectorParams(){return detectorParams_;}
      void setMatcher(boost::shared_ptr<Matcher> & _matcher, int patchSize, double nSigma, double threshold, double mahalanobisTh, double _measStd){
      	matcher = _matcher;
      	matcherParams_.patchSize = patchSize;
      	matcherParams_.regionSigma = nSigma;
      	matcherParams_.threshold = threshold;
      	matcherParams_.mahalanobisTh = mahalanobisTh;
      	matcherParams_.measStd = _measStd;
      	matcherParams_.measVar = _measStd * _measStd;
      }
      matcher_params_t matcherParams(){return matcherParams_;}
      void setActiveSearchGrid( boost::shared_ptr<ActiveSearchGrid> arg ) { asGrid=arg; }
      boost::shared_ptr<ActiveSearchGrid> activeSearchGrid( void ) { return asGrid; }
      void setAlgorithmParams(int n_updates){
      	algorithmParams_.n_updates = n_updates;
      }
      alg_params_t algorithmParams(){return algorithmParams_;}
    protected:
      void detectNewData( boost::shared_ptr<RawSpec> data );
      bool match(const boost::shared_ptr<RawImage> & rawPtr, const appearance_ptr_t & targetApp, cv::Rect &roi, Measurement & measure, const appearance_ptr_t & app);
      void processData( boost::shared_ptr<RawSpec> data );

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

//    typedef DataManagerActiveSearch<RawImage,SensorPinHole> DataManagerActiveSearch_Image_PH;

  }
}//namespace jafar::rtslam


#include "rtslam/dataManagerActiveSearch.t.cpp"

#endif /* DATAMANAGERACTIVESEARCH_HPP_ */
