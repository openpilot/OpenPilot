/*
 * \file dataManagerOnePointRansac.hpp
 *
 *     Project: jafar
 *  \date: Jun 30, 2010
 *      \author: jsola
 *      \ingroup rtslam
 */

#ifndef DATAMANAGERONEPOINTRANSAC_HPP_
#define DATAMANAGERONEPOINTRANSAC_HPP_

#include "rtslam/rtSlam.hpp"
#include "rtslam/parents.hpp"

namespace jafar {
	namespace rtslam {

		//		class DataManagerOnePointRansac;
		//		boost::shared_ptr<DataManagerOnePointRansac> one_pnt_ransac_ptr_t;

		template<class RawSpec, class SensorSpec, class Detector, class Matcher>
		class DataManagerOnePointRansac: public DataManagerAbstract,
		    public SpecificChildOf<SensorSpec> {

			public:
				// Define the function linkToParentSensorSpec.
			ENABLE_LINK_TO_SPECIFIC_PARENT(SensorAbstract,SensorSpec,
					SensorSpec,DataManagerAbstract);
				// Define the functions sensorSpec() and sensorSpecPtr().
			ENABLE_ACCESS_TO_SPECIFIC_PARENT(SensorSpec,sensorSpec);

			public:
				DataManagerOnePointRansac() {
				}
				virtual ~DataManagerOnePointRansac() {
				}

			protected:
				boost::shared_ptr<Detector> detector;
				boost::shared_ptr<Matcher> matcher;
				boost::shared_ptr<ActiveSearchGrid> asGrid;
				// the list of observations sorted by information gain
				typedef map<double, observation_ptr_t> ObservationListSorted;
				ObservationListSorted obsListSorted;
			public:
				void process(){}
		};

	}
}

#endif /* DATAMANAGERONEPOINTRANSAC_HPP_ */
