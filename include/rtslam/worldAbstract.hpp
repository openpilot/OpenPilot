/**
 * \file worldAbstract.hpp
 *
 * \date 2010/05/15
 *     \author croussil@laas.fr
 *
 * Header file for abstract world
 *
 * \ingroup rtslam
 */

#ifndef WORLDABSTRACT_HPP_
#define WORLDABSTRACT_HPP_

#include "jmath/jblas.hpp"
#include "rtslam/rtSlam.hpp"
#include "rtslam/objectAbstract.hpp"

#include "rtslam/parents.hpp"

namespace jafar {
	/**
	 * Namespace rtslam for real-time slam module.
	 * \ingroup rtslam
	 */
	namespace rtslam {
		using namespace std;


		// some forward declarations.
		class MapAbstract;


		/** Base class for all world types defined in the module rtslam.
		 *
		 * \author croussil@laas.fr
		 * \ingroup rtslam
		 */
		class WorldAbstract: public ObjectAbstract, public ParentOf<MapAbstract> {

			ENABLE_ACCESS_TO_CHILDREN(MapAbstract,Map,map);

			public:

				/**
				 * Constructor
				 */
				WorldAbstract() {}

				/**
				 * Mandatory virtual destructor - Map is used as-is, non-abstract by now
				 */
				virtual ~WorldAbstract() {
				}

			void addMap(map_ptr_t map)
			{
				mapList().push_back(map);
			}


		};

	}
}

#endif /* MAPABSTRACT_HPP_ */

