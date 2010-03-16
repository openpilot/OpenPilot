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


#include <list>
#include <jmath/jblas.hpp>

// include parents
#include "rtslam/mapObject.hpp"
#include "rtslam/mapAbstract.hpp"

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
				virtual ~DescriptorAbstract(void) {
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
				 * constructor
				 */
				LandmarkAbstract(MapAbstract & _map, const size_t _size);

				/**
				 * Mandatory virtual destructor.
				 */
				virtual ~LandmarkAbstract(void) {
				}

				DescriptorAbstract descriptor;

				/**
				 * Parent map
				 */
				MapAbstract * map;

				/**
				 * A set of observations (one per sensor)
				 */
				typedef std::map<size_t, ObservationAbstract*> observations_t;
				observations_t observations;

				/**
				 * Reparametrize the landmark.
				 */
				//				virtual void reparametrize(LandmarkAbstract & landmark) = 0;

//				inline void setDescriptor(DescriptorAbstract _desc) {
//					descriptor = _desc;
//				}

//				inline void addObservation(ObservationAbstract & _obs) ;

		};

	}
}

#endif // #ifndef __LandmarkAbstract_H__
