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

#include "kernel/IdFactory.hpp"

#include "rtslam/rtSlam.hpp"

// include parents
#include "rtslam/mapAbstract.hpp"
#include "rtslam/mapObject.hpp"
#include "rtslam/DescriptorAbstract.hpp"
#include "rtslam/parents.hpp"

#include <boost/smart_ptr.hpp>

namespace jafar {
	namespace rtslam {
		using namespace std;


		// Forward declarations of children
		class ObservationAbstract;


		/** Base class for all landmarks defined in the module
		 * rtslam.
		 *
		 * @ingroup rtslam
		 */
		class LandmarkAbstract: public MapObject, public ChildOf<MapAbstract> , public boost::enable_shared_from_this<
		    LandmarkAbstract>, public ParentOf<ObservationAbstract> {


				// define the function linkToParentMap().
			ENABLE_LINK_TO_PARENT(MapAbstract,Map,LandmarkAbstract)
				;
				// define the functions mapPtr() and map().
			ENABLE_ACCESS_TO_PARENT(MapAbstract,map)
				;
				// define the type ObservationList, and the function observationList().
			ENABLE_ACCESS_TO_CHILDREN(ObservationAbstract,Observation,observation)
				;

			public:
				/**
				 * constructor from map and size
				 */
			LandmarkAbstract(const map_ptr_t & _mapPtr, const size_t _size);
			LandmarkAbstract(const simulation_t dummy, const map_ptr_t & _mapPtr, const size_t _size);

				/**
				 * Mandatory virtual destructor.
				 */
				virtual ~LandmarkAbstract() {
				}

				static kernel::IdFactory landmarkIds;

				typedef enum geometry_t {
						POINT,
						LINE,
						PLANE,
						ELLIPSE
				} geometry_t;

				geometry_t geomType;

				// \todo use a smart pointer here.
				DescriptorAbstract descriptor; ///<                       Landmark descriptor

				jblas::mat LNEW_lmk; //Jacobian comming from reparametrisation of old lmk wrt. new lmk

				//Reparametrize old Landmarks into new ones
				void reparametrize(){}
				virtual void reparametrize_func(const vec & lmk, vec & lnew, mat & LNEW_lmk) = 0;

				virtual void createDescriptor(const appearance_ptr_t & appPtr, const vec7 & sensorPose){
					// todo implement createDescriptor()
				}

		};

	}
}

#endif // #ifndef __LandmarkAbstract_H__
