/**
 * \file rtSlam.hpp
 *
 *  This file defines general typedefs and material for the whole rtslam project.
 *
 * \date 21/03/2010
 * \author jsola
 *
 *
 * \ingroup rtslam
 */

#ifndef RTSLAM_HPP_
#define RTSLAM_HPP_

#include <sys/time.h> 

#include <map>
#include <list>

#include "boost/shared_ptr.hpp"
#include "boost/weak_ptr.hpp"

#include "kernel/IdFactory.hpp"
#include "kernel/jafarMacro.hpp"

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

		extern unsigned int rand_state;
		inline void srand(unsigned int seed)
		{
// 			JFR_DEBUG("!rand: seed " << seed);
			rand_state = seed;
		}
		inline int rand()
		{
// 			JFR_DEBUG_BEGIN(); JFR_DEBUG_SEND("!rand: state " << rand_state);
			int r = rand_r(&rand_state);
// 			JFR_DEBUG_SEND(" value " << r << " state " << rand_state); JFR_DEBUG_END();
			return r;
		}
		

		// forward declarations
		class WorldAbstract;
		class MapAbstract;
		class RobotAbstract;
		class SensorAbstract;
		class SensorExteroAbstract;
		class SensorProprioAbstract;
  	class DataManagerAbstract;
		class LandmarkAbstract;
		class LandmarkFactoryAbstract;
		class ObservationModelAbstract;
		class ObservationAbstract;
	  class MapManagerAbstract;
		class RawAbstract;
		class AppearanceAbstract;
		class FeatureAbstract;
		class DescriptorAbstract;
		class Gaussian;
		class ExtendedKalmanFilterIndirect;
		class SensorManagerAbstract;

		// Pointers with boost::shared_ptr:
		typedef boost::shared_ptr<WorldAbstract>       										world_ptr_t;
		typedef boost::shared_ptr<MapAbstract>         										map_ptr_t;
		typedef boost::shared_ptr<RobotAbstract>       										robot_ptr_t;
		typedef boost::shared_ptr<SensorAbstract>      										sensor_ptr_t;
		typedef boost::shared_ptr<SensorExteroAbstract>      							sensorext_ptr_t;
		typedef boost::shared_ptr<SensorProprioAbstract>      						sensorprop_ptr_t;
		typedef boost::shared_ptr<DataManagerAbstract>      							data_manager_ptr_t;
		typedef boost::shared_ptr<LandmarkAbstract>    										landmark_ptr_t;
		typedef boost::shared_ptr<LandmarkFactoryAbstract>   							landmark_factory_ptr_t;
		typedef boost::shared_ptr<ObservationModelAbstract> 							observation_model_ptr_t;
		typedef boost::shared_ptr<ObservationAbstract> 										observation_ptr_t;
		typedef boost::shared_ptr<MapManagerAbstract>    									map_manager_ptr_t;

		typedef boost::shared_ptr<RawAbstract>         										raw_ptr_t;
		typedef boost::shared_ptr<AppearanceAbstract>  										appearance_ptr_t;
		typedef boost::shared_ptr<FeatureAbstract>     										feature_ptr_t;
		typedef boost::shared_ptr<DescriptorAbstract>  										descriptor_ptr_t;

		typedef boost::shared_ptr<Gaussian>						 										gaussian_ptr_t;

		typedef boost::shared_ptr<ExtendedKalmanFilterIndirect>          	ekfInd_ptr_t;
		typedef boost::shared_ptr<SensorManagerAbstract>                  sensor_manager_ptr_t;

		typedef kernel::IdFactory<unsigned int, kernel::IdCollectorNone> 	IdFactory; // FIXME maybe we should change for a smarter IdFactory ? eg:
		//typedef kernel::IdFactory<unsigned, kernel::IdCollectorList> IdFactory;
		//typedef kernel::IdFactory<unsigned, kernel::IdCollectorSet> IdFactory;
	}
}

#endif /* RTSLAM_HPP_ */
