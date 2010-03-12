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

/* --------------------------------------------------------------------- */
/* --- INCLUDE --------------------------------------------------------- */
/* --------------------------------------------------------------------- */

#include <list>
#include <jmath/jblas.hpp>

#include "rtslam/blocks.hpp"
// include parents
#include "rtslam/mapAbstract.hpp"

namespace jafar {

	namespace rtslam {

		using namespace std;

		// Forward declarations of children
		class ObservationAbstract;

		/** Base class for all landmark descriptors defined in the module
		 * rtslam.
		 *
		 * @ingroup rtslam
		 */
		class DescriptorAbstract {
			public:
				/**
				 * Mandatory virtual destructor
				 */
				virtual ~DescriptorAbstract(void) {
				}
		};

		/* --------------------------------------------------------------------- */
		/* --- CLASS ----------------------------------------------------------- */
		/* --------------------------------------------------------------------- */

		/** Base class for all landmarks defined in the module
		 * rtslam.
		 *
		 * @ingroup rtslam
		 */
		class LandmarkAbstract {
			private:
				size_t id_;
				std::string type_;

			public:

				Gaussian state;
				DescriptorAbstract descriptor;

				MapAbstract * map;
				list<ObservationAbstract*> observationsList;

				/**
				 * Mandatory virtual destructor.
				 */
				virtual ~LandmarkAbstract(void) {
				}

				/**
				 * constructor
				 */
				LandmarkAbstract(MapAbstract & _map, const jblas::ind_array & _ial);

				inline void id(size_t _id) {
					id_ = _id;
				}
				inline void type(std::string _type) {
					type_ = _type;
				}
				inline size_t id(void) {
					return id_;
				}
				inline std::string type(void) {
					return type_;
				}

				static size_t size(void) {
					return 0;
				}

				/**
				 * Reparametrize the landmark.
				 */
				//				virtual void reparametrize(LandmarkAbstract & landmark) = 0;

				inline void setDescriptor(DescriptorAbstract _desc) {
					descriptor = _desc;
				}

				inline void addObservation(ObservationAbstract & _obs) {
					observationsList.push_front(&_obs);
				}

				/**
				 * Frame transformations : force definition in derived classes
				 */
				//				virtual jblas::vec fromFrame(const jblas::vec & F, const jblas::vec & lf) = 0;
				//				virtual void fromFrame(const jblas::vec & F, const jblas::vec & lf, jblas::vec & l, jblas::mat & L_f,
				//				    jblas::mat & L_lf) = 0;
				//				virtual jblas::vec toFrame(const jblas::vec & F, const jblas::vec & l) = 0;
				//				virtual void toFrame(const jblas::vec & F, const jblas::vec & l, jblas::vec & lf, jblas::mat & LF_f,
				//				    jblas::mat & LF_l) = 0;

				/**
				 * Operator << for class LandmarkAbstract.
				 * It shows different information of the landmark.
				 */
				friend ostream& operator <<(ostream & s, jafar::rtslam::LandmarkAbstract & lmk) {
					s << "LANDMARK " << lmk.id() << " of type " << lmk.type() << std::endl;
					s << ".state:  " << lmk.state << std::endl;
					return s;
				}

		};

	}
}

#endif // #ifndef __LandmarkAbstract_H__
/*
 * Local variables:
 * mode: c++
 * indent-tabs-mode: t
 * tab-width: 2
 * c-basic-offset: 2
 * End:
 */
