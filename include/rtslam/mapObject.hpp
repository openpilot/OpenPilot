/**
 * mapObject.hpp
 *
 *  Created on: 13/03/2010
 *      Author: jsola
 *
 *  \file mapObject.hpp
 *
 *  ## Add a description here ##
 *
 * \ingroup rtslam
 */

#ifndef MAPOBJECT_HPP_
#define MAPOBJECT_HPP_

#include "jmath/jblas.hpp"
#include "rtslam/objectAbstract.hpp"
#include "rtslam/mapAbstract.hpp"
#include "rtslam/gaussian.hpp"

namespace jafar {
	namespace rtslam {
		using namespace std;


		/**
		 * Class for generic mappable objects.
		 * \author jsola
		 * \ingroup rtslam
		 */
		class MapObject: public ObjectAbstract {

			public:
				MapAbstract * slamMap;

				Gaussian state;

				/**
				 * Local constructor from size.
				 * With this constructor the object is not linked to any map. Use it for sensors.
				 * \param _size the state size.
				 */
				MapObject(const size_t _size);

				/**
				 * Remote constructor from remote map and size
				 * \param _map the remote map
				 * \param _begin the first index pointing to the remote storage
				 */
				MapObject(MapAbstract & _map, const size_t _size);

				/**
				 * Mandatory virtual destructor
				 */
				virtual ~MapObject() {
				}

				inline static size_t size() {
					return 0;
				}

				/**
				 * Operator << for class MapObject.
				 * It shows different information of the object.
				 */
				friend ostream& operator <<(ostream & s, jafar::rtslam::MapObject & obj) {
					s << obj.categoryName() << " " << obj.id() << ": ";
					if (obj.name().size() > 0)
						s << obj.name() << ", ";
					s << "of type " << obj.type() << endl;
					s << ".state:  " << obj.state;
					return s;
				}

		};

	}
}

#endif /* MAPOBJECT_HPP_ */
