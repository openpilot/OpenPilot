/**
 * \file dataManagerSearch.cpp
 * 
 * ## Add brief description here ##
 *
 * \author jsola@laas.fr
 * \date 28/04/2010
 *
 *  ## Add a description here ##
 *
 * \ingroup rtslam
 */

#include "rtslam/dataManagerSearch.hpp"


namespace jafar {
	namespace rtslam {
		using namespace std;


		void DataManagerSearch::process(){
			observeKnownLmks();
			detectNewLmks();
		}

		void DataManagerSearch::observeKnownLmks() {
		}

		void DataManagerSearch::detectNewLmks() {
		}

	}
}
