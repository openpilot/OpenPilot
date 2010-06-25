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
      static const int NUPDATES = 1000;
      static const int patchMatchSize = 7;

    protected:
      boost::shared_ptr<ActiveSearchGrid> asGrid;
    public:
      void setActiveSearchGrid( boost::shared_ptr<ActiveSearchGrid> arg ) { asGrid=arg; }
      boost::shared_ptr<ActiveSearchGrid> activeSearchGrid( void ) { return asGrid; }

    public:
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
