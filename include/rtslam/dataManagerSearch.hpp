/**
 * \file dataManagerSearch.hpp
 * 
 * ## Add brief description here ##
 *
 * \author jsola@laas.fr
 * \date 28/04/2010
 *
 *  This class implements a very basic version of the active search algorithm. This is why it is just called Search.
 *
 * \ingroup rtslam
 */

#ifndef DATAMANAGERSEARCH_HPP_
#define DATAMANAGERSEARCH_HPP_

#include "rtslam/dataManagerAbstract.hpp"

namespace jafar {
	namespace rtslam {


		/**
		 * \ingroup rtslam
		 */
		class DataManagerSearch: public DataManagerAbstract {
			public:
				void process();
			private:
				void observeKnownLmks();
				void detectNewLmks();
		};

	}
}

#endif /* DATAMANAGERSEARCH_HPP_ */
