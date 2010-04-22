/**
 *  \file mapObject.hpp
 *
 * \date 13/03/2010
 * \author jsola@laas.fr
 *
 *  Header file for mappable objects
 *
 * \ingroup rtslam
 */

#ifndef MAPOBJECT_HPP_
#define MAPOBJECT_HPP_

#include "jmath/jblas.hpp"
#include "rtslam/rtSlam.hpp"
#include "rtslam/objectAbstract.hpp"
#include "rtslam/mapAbstract.hpp"
#include "rtslam/gaussian.hpp"

namespace jafar {
	namespace rtslam {
		using namespace std;


		/**
		 * Class for generic mappable objects.
		 * \author jsola@laas.fr
		 * \ingroup rtslam
		 */
		class MapObject: public ObjectAbstract {

				friend ostream& operator <<(ostream & s, jafar::rtslam::MapObject & obj);

			public:
				typedef enum {
					FILTERED, ///<  Object's state vector is part of the SLAM filter.
					UNFILTERED ///< Object's state vector is no part of the SLAM filter.
				} filtered_obj_t;

				Gaussian state;

//				/**
//				 * Local constructor from size.
//				 * With this constructor the object is not linked to any map. Use it for eg. sensors.
//				 * \param _size the state size.
//				 */
//				MapObject(const size_t _size);

				/**
				 * Selectable constructor with boolean inFilter flag.
				 * \param _mapPtr pointer to map
				 * \param _size size of the state vector
				 * \param inFilter flag for filtered state
				 */
				MapObject(const map_ptr_t & _mapPtr, const size_t _size, const filtered_obj_t inFilter = FILTERED);

				/**
				 * Mandatory virtual destructor
				 */
				virtual ~MapObject() {
				}

				inline static size_t size() {
					return 0;
				}

		};

	}
}

#endif /* MAPOBJECT_HPP_ */
