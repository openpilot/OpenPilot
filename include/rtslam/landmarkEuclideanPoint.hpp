/*
 * \file landmarkEuclideanPoint.hpp
 *
 *  Created on: Apr 14, 2010
 *      Author: agonzale
 *      \ingroup rtslam
 */

#ifndef LANDMARKEUCLIDEANPOINT_HPP_
#define LANDMARKEUCLIDEANPOINT_HPP_

#include "boost/shared_ptr.hpp"
#include "rtslam/landmarkAbstract.hpp"
#include "rtslam/quatTools.hpp"
//#include "rtslam/ahpTools.hpp"

/**
 * General namespace for Jafar environment.
 * \ingroup jafar
 */
namespace jafar {
	namespace rtslam {

		class LandmarkEuclideanPoint;
		typedef boost::shared_ptr<LandmarkEuclideanPoint> eucp_ptr_t;



		/**
		 * Class for Euclidean 3D points
		 * \ingroup rtslam
		 */
		class LandmarkEuclideanPoint: public LandmarkAbstract {
			public:


				/**
				 * Constructor from map
				 */
				LandmarkEuclideanPoint(const map_ptr_t & mapPtr);

				/**
				 * Constructor for simulated landmark.
				 */
				LandmarkEuclideanPoint(const simulation_t dummy, const map_ptr_t & mapPtr);
		                /**
				 * Constructor by replacement: occupied the same filter state as a specified previous lmk. _icomp is the complementary memory, to be relaxed by the user.
				 */
		                LandmarkEuclideanPoint(const map_ptr_t & _mapPtr, const landmark_ptr_t _prevLmk,jblas::ind_array & _icomp);

				virtual ~LandmarkEuclideanPoint() {
//					cout << "Deleted landmark: " << id() << ": " << typeName() << endl;
				}

				virtual std::string typeName() const {
					return "Euclidean-point";
				}


				static size_t size(void) {
					return 3;
				}

				virtual size_t mySize() {return size();}

				virtual size_t reparamSize() {return size();}

				virtual vec reparametrize_func(const vec & lmk){
					return lmk;
				}

				void reparametrize_func(const vec & lmk, vec & lnew, mat & LNEW_lmk){
					lnew = lmk;
					LNEW_lmk = identity_mat(size());
				}

				virtual bool needToReparametrize(){
					return false; // never reparametrize an Euclidean point
				}
		}; // class LandmarkEuclideanPoint
	} // namespace rtslam
} // namespace jafar


#endif /* LANDMARKEUCLIDEANPOINT_HPP_ */
