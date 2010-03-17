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
 * File defining the abstract landmark class
 * \ingroup rtslam
 */

#ifndef __LandmarkAbstract_H__
#define __LandmarkAbstract_H__


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
				LandmarkAbstract(MapAbstract & _map, const size_t _size);

				/**
				 * Mandatory virtual destructor.
				 */
				virtual ~LandmarkAbstract() {
				}

				DescriptorAbstract descriptor;

				/**
				 * Parent map
				 */
				MapAbstract * slamMap;

				typedef std::map<size_t, ObservationAbstract*> observations_t;
				/**
				 * A set of observations (one per sensor)
				 */
				observations_t observations;

				inline void addObservation(ObservationAbstract * _obsPtr) ;

				/**
				 * Reparametrize the landmark.
				 */
				//				virtual void reparametrize(LandmarkAbstract & landmark) = 0;

//				inline void setDescriptor(DescriptorAbstract _desc) {
//					descriptor = _desc;
//				}


		};

	}
}

#endif // #ifndef __LandmarkAbstract_H__
