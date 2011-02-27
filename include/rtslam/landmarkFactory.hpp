/**
 * \file landmarkFactory.hpp
 *
 * \date 28/02/2011
 * \author croussil
 * \ingroup rtslam
 */
 
#ifndef LANDMARKFACTORY_HPP_
#define LANDMARKFACTORY_HPP_

#include "rtslam/rtSlam.hpp"

namespace jafar {
namespace rtslam {

	class LandmarkFactoryAbstract
	{
		public:
			virtual landmark_ptr_t createInit(map_ptr_t mapPtr) = 0;
			virtual landmark_ptr_t createConverged(map_ptr_t mapPtr) = 0;
			virtual landmark_ptr_t createConverged(map_ptr_t mapPtr, landmark_ptr_t lmkinit, jblas::ind_array &_icomp) = 0;
			virtual size_t sizeComplement() = 0;
			virtual size_t sizeInit() = 0;
	};
	
	
	template<class LandmarkInit, class LandmarkConverged>
	class LandmarkFactory: public LandmarkFactoryAbstract
	{
		public:
			virtual landmark_ptr_t createInit(map_ptr_t mapPtr) {
				return boost::shared_ptr<LandmarkInit>(new LandmarkInit(mapPtr));
			}
			virtual landmark_ptr_t createConverged(map_ptr_t mapPtr) {
				return boost::shared_ptr<LandmarkConverged>(new LandmarkConverged(mapPtr));
			}
			virtual landmark_ptr_t createConverged(map_ptr_t mapPtr, landmark_ptr_t lmkinit, jblas::ind_array &_icomp) {
				return boost::shared_ptr<LandmarkConverged>(new LandmarkConverged(mapPtr, lmkinit, _icomp));
			}
			virtual size_t sizeInit() {
				return (LandmarkInit::size());
			}
			virtual size_t sizeComplement() {
				return (LandmarkInit::size() - LandmarkConverged::size());
			}
	};

}}

#endif // LANDMARKFACTORY_HPP
