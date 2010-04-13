/**
 * landmarkAbstract.cpp
 *
 * \date 10/03/2010
 * \author jsola@laas.fr
 *
 *  \file landmarkAbstract.cpp
 *
 *  ## Add a description here ##
 *
 * \ingroup rtslam
 */

#include "rtslam/landmarkAbstract.hpp"
#include "rtslam/observationAbstract.hpp"
#include "rtslam/mapAbstract.hpp"

namespace jafar {
	namespace rtslam {
		using namespace std;


		/*
		 * constructor.
		 */
		LandmarkAbstract::LandmarkAbstract(const map_ptr_t & _mapPtr, const size_t _size) :
			MapObject(_mapPtr, _size),
			mapPtr(_mapPtr)
		{
			categoryName("LANDMARK");
		}

		void LandmarkAbstract::linkToObservation(const observation_ptr_t & _obsPtr) {
			observationsPtrSet[_obsPtr->id()] = _obsPtr;
		}

		void LandmarkAbstract::linkToMap(const map_ptr_t & _mapPtr) {
			mapPtr = _mapPtr;
		}

	}
}
