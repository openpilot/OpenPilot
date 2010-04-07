/*+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
 * Copyright RIA-LAAS/CNRS, 2010
 *+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
 *
 * File:      landmarkAbstract.h
 * Project:   RT-Slam
 * Author:    Joan Sola
 *
 * Version control
 * ===============
 *
 *  $Id$
 *
 * Description
 * ============
 *
 *
 * ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/

/**
 * \file landmarkAbstract.hpp
 * \author jsola@laas.fr
 * File defining the abstract landmark class
 * \ingroup rtslam
 */

#ifndef __LandmarkAbstract_H__
#define __LandmarkAbstract_H__

#include "rtslam/rtSlam.hpp"

// include parents
#include "rtslam/mapAbstract.hpp"
#include "rtslam/mapObject.hpp"

namespace jafar {
	namespace rtslam {
		using namespace std;

		// Forward declarations of children
		class ObservationAbstract;

		/** Base class for all landmark descriptors defined in the module rtslam.
		 *
		 * \author jsola@laas.fr
		 * \ingroup rtslam
		 */
		class DescriptorAbstract {

			public:
				/**
				 * Mandatory virtual destructor
				 */
				virtual ~DescriptorAbstract() {
				}
		};


		/** Base class for all landmarks defined in the module
		 * rtslam.
		 *
		 * @ingroup rtslam
		 */
		class LandmarkAbstract : public MapObject {

			public:

				/**
				 * constructor from map and size
				 */
				LandmarkAbstract(const map_ptr_t & _mapPtr, const size_t _size);

				/**
				 * Mandatory virtual destructor.
				 */
				virtual ~LandmarkAbstract() {
				}

				map_ptr_t mapPtr; ///<                                   Parent map
				observations_ptr_set_t observationsPtrSet; ///<                 A set of observations (one per sensor)

				DescriptorAbstract descriptor; ///<                       Landmark descriptor

				void linkToObservation(const observation_ptr_t & _obsPtr); ///<   Link to observation
				void linkToMap(const map_ptr_t & _mapPtr); ///<                   Link to map

		};

	}
}

#endif // #ifndef __LandmarkAbstract_H__
