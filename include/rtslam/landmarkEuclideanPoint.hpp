/*
 * landmarkEuclideanPoint.hpp
 *
 *  Created on: Apr 14, 2010
 *      Author: agonzale
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
				LandmarkEuclideanPoint(const simulation_t dummy, const map_ptr_t & mapPtr);

				static size_t size(void) {
					return 3;
				}

				virtual ~LandmarkEuclideanPoint() {
												}

				void reparametrize_func(const vec & lmk, vec & lnew, mat & LNEW_lmk){
					lnew = lmk;
				}

		}; // class LandmarkEuclideanPoint
	} // namespace rtslam
} // namespace jafar


#endif /* LANDMARKEUCLIDEANPOINT_HPP_ */
