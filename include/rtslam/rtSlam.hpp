/**
 * \file rtSlam.hpp
 *
 *  This file defines general typedefs and material for the whole rtslam project.
 *
 * \date 21/03/2010
 * \author jsola@laas.fr
 *
 *
 * \ingroup rtslam
 */

#ifndef RTSLAM_HPP_
#define RTSLAM_HPP_

#include "kernel/IdFactory.hpp"
#include "boost/shared_ptr.hpp"
#include "boost/weak_ptr.hpp"
#include <map>
#include <list>

#ifdef JFR_NDEBUG
	#define PTR_CAST static_cast
	#define SPTR_CAST boost::static_pointer_cast
#else
	#define PTR_CAST dynamic_cast
	#define SPTR_CAST boost::dynamic_pointer_cast
#endif

namespace jafar {
	namespace kernel {
		template<typename> class IdCollectorNone;
		template<typename> class IdCollectorList;
		template<typename> class IdCollectorSet;
		template<typename, template<typename> class> class IdFactory;
	}

	namespace rtslam {


		// forward declarations
		class WorldAbstract;
		class MapAbstract;
		class RobotAbstract;
		class SensorAbstract;
  	        class DataManagerAbstract;
		class LandmarkAbstract;
		class ObservationAbstract;
		class RawAbstract;
		class AppearanceAbstract;
		class FeatureAbstract;
		class DescriptorAbstract;
		class Gaussian;
		class ExtendedKalmanFilterIndirect;

		// Pointers with boost::shared_ptr:
		typedef boost::shared_ptr<WorldAbstract>       										world_ptr_t;
		typedef boost::shared_ptr<MapAbstract>         										map_ptr_t;
		typedef boost::shared_ptr<RobotAbstract>       										robot_ptr_t;
		typedef boost::shared_ptr<SensorAbstract>      										sensor_ptr_t;
		typedef boost::shared_ptr<DataManagerAbstract>      										data_manager_ptr_t;
		typedef boost::shared_ptr<LandmarkAbstract>    										landmark_ptr_t;
		typedef boost::shared_ptr<ObservationAbstract> 										observation_ptr_t;

		typedef boost::shared_ptr<RawAbstract>         										raw_ptr_t;
		typedef boost::shared_ptr<AppearanceAbstract>  										appearance_ptr_t;
		typedef boost::shared_ptr<FeatureAbstract>     										feature_ptr_t;
		typedef boost::shared_ptr<DescriptorAbstract>  										descriptor_ptr_t;

		typedef boost::shared_ptr<Gaussian>						 										gaussian_ptr_t;

		typedef boost::shared_ptr<ExtendedKalmanFilterIndirect>          	ekfInd_ptr_t;

		typedef kernel::IdFactory<unsigned int, kernel::IdCollectorNone> 	IdFactory; // FIXME maybe we should change for a smarter IdFactory ? eg:
		//typedef kernel::IdFactory<unsigned, kernel::IdCollectorList> IdFactory;
		//typedef kernel::IdFactory<unsigned, kernel::IdCollectorSet> IdFactory;
	}
}

#endif /* RTSLAM_HPP_ */
