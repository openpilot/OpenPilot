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

//#include "kernel/IdFactory.hpp"

#include "rtslam/rtSlam.hpp"

// include parents
#include "rtslam/mapAbstract.hpp"
#include "rtslam/mapManager.hpp"
#include "rtslam/mapObject.hpp"
#include "rtslam/descriptorAbstract.hpp"
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
		class LandmarkAbstract: public MapObject, public ChildOf<MapManagerAbstract> , public boost::enable_shared_from_this<
		    LandmarkAbstract>, public ParentOf<ObservationAbstract> {


				// define the function linkToParentMapManager().
		       ENABLE_LINK_TO_PARENT(MapManagerAbstract,MapManager,LandmarkAbstract)
				;
				// define the functions mapManagerPtr() and mapManager().
			ENABLE_ACCESS_TO_PARENT(MapManagerAbstract,mapManager)
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

				static IdFactory landmarkIds;
				void setId(){id(landmarkIds.getId());}

				enum geometry_t {
						POINT,
						LINE,
						PLANE,
						ELLIPSE
				} ;

				enum type_enum{
						PNT_EUC, PNT_AH
				};
				type_enum type;

			protected:
				geometry_t geomType;
			public:
				geometry_t getGeomType(){return geomType;}
//				std::string typeName();
				std::string categoryName(){return "LANDMARK";}

				virtual landmark_ptr_t convertToStandardParametrization() = 0;
				/* FIXME how to implement convertToStandardParametrization in concrete types ?
				- for the standard type, if we return a shared_from_this, does it share the reference counter with the existing shared_ptr ?
				- for the other types, we need to construct a new landmark, but without any map, is it possible ?
				*/

				descriptor_ptr_t descriptorPtr; ///< Landmark descriptor

				jblas::mat LNEW_lmk; ///<Jacobian comming from reparametrisation of old lmk wrt. new lmk

				//Reparametrize old Landmarks into new ones
				void reparametrize();
				virtual vec reparametrize_func(const vec & lmk) = 0;
				virtual void reparametrize_func(const vec & lmk, vec & lnew, mat & LNEW_lmk) = 0;

				/**
				 * Size of the landmark state
				 */
				virtual size_t mySize() = 0;

				/**
				 * Size of the reparametrized landmark
				 */
				virtual size_t stdSize() = 0;

				// Create a landmark descriptor
				virtual void setDescriptor(const descriptor_ptr_t & descPtr)
				{
					descriptorPtr = descPtr;
				}
//				{
//					this->descriptor = desc_ptr_t (new DescriptorAbstract()) ;
//					this->descriptor->pose0 = sensorPose;
//					this->descriptor->app0Ptr = appPtr;
//				}

		};

	}
}

#endif // #ifndef __LandmarkAbstract_H__
