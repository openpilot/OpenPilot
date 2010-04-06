/**
 * mapObject.cpp
 *
 *  Created on: 13/03/2010
 *      Author: jsola
 *
 *  \file mapObject.cpp
 *
 *  ## Add a description here ##
 *
 * \ingroup rtslam
 */

#include "rtslam/mapObject.hpp"
#include "jmath/indirectArray.hpp"

namespace jafar {
	namespace rtslam {
		using namespace std;


		/*
		 * Operator << for class MapObject.
		 * It shows different information of the object.
		 */
		ostream& operator <<(ostream & s, jafar::rtslam::MapObject & obj) {
			s << obj.categoryName() << " " << obj.id() << ": ";
			if (obj.name().size() > 0)
				s << obj.name() << ", ";
			s << "of type " << obj.type() << endl;
			s << ".state:  " << obj.state;
			return s;
		}


		/*
		 * Local constructor from size
		 */
		MapObject::MapObject(const std::size_t _size) :
			ObjectAbstract(), state(_size) {
			categoryName("MAP OBJECT");
		}


		/*
		 * Remote constructor from remote map and indirect array
		 */
		MapObject::MapObject(const map_ptr_t _mapPtr, const size_t _size) :
			ObjectAbstract(), state(_mapPtr->x(), _mapPtr->P(), _mapPtr->reserveStates(_size)) {
			categoryName("MAP OBJECT");
		}

	}
}
