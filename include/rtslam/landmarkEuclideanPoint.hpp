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

				/**
				 * Constructor for simulated landmark.
				 */
				LandmarkEuclideanPoint(const simulation_t dummy, const map_ptr_t & mapPtr);

				virtual ~LandmarkEuclideanPoint() {
				}
				
				virtual landmark_ptr_t convertToStandardParametrization()
				{
					// TODO
					return shared_from_this(); // FIXME not sure that this does exactly what we want
				}

				static size_t size(void) {
					return 3;
				}

				virtual size_t mySize() {return size();}

				virtual size_t stdSize() {return size();}

				virtual vec reparametrize_func(const vec & lmk){
					return lmk;
				}

				void reparametrize_func(const vec & lmk, vec & lnew, mat & LNEW_lmk){
					lnew = lmk;
					LNEW_lmk = identity_mat(size());
				}

		}; // class LandmarkEuclideanPoint
	} // namespace rtslam
} // namespace jafar


#endif /* LANDMARKEUCLIDEANPOINT_HPP_ */
