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

#include "boost/shared_ptr.hpp"
#include "boost/weak_ptr.hpp"
#include <map>
#include <list>

namespace jafar {
	namespace rtslam {


		// forward declarations
		class MapAbstract;
		class RobotAbstract;
		class SensorAbstract;
		class LandmarkAbstract;
		class ObservationAbstract;
		class FeatureAbstract;
		class RawAbstract;
		class AppearanceAbstract;
		class DescriptorAbstract;
		class Gaussian;
		class ExtendedKalmanFilterIndirect;

		// Pointers with boost::shared_ptr:
		typedef boost::shared_ptr<MapAbstract>         map_ptr_t;
		typedef boost::shared_ptr<RobotAbstract>       robot_ptr_t;
		typedef boost::shared_ptr<SensorAbstract>      sensor_ptr_t;
		typedef boost::shared_ptr<LandmarkAbstract>    landmark_ptr_t;
		typedef boost::shared_ptr<ObservationAbstract> observation_ptr_t;
		typedef boost::shared_ptr<FeatureAbstract>     feature_ptr_t;
		typedef boost::shared_ptr<RawAbstract>         raw_ptr_t;
		typedef boost::shared_ptr<AppearanceAbstract>  appearance_ptr_t;
		typedef boost::shared_ptr<DescriptorAbstract>  desc_ptr_t;

		typedef boost::shared_ptr<Gaussian>						gaussian_ptr_t;
		typedef boost::shared_ptr<ExtendedKalmanFilterIndirect> ekfInd_ptr_t;
	}
}

#endif /* RTSLAM_HPP_ */
