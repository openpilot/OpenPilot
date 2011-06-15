/**
 * \file mapObject.cpp
 * \date 13/03/2010
 * \author jsola
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
		ostream& operator <<(ostream & s, MapObject const & obj) {
			s << obj.categoryName() << " " << obj.id() << ": ";
			if (obj.name().size() > 0)
				s << obj.name() << ", ";
			s << ".state:  " << obj.state;
			return s;
		}


		/**
		 * Selectable constructor with inFilter flag.
		 */
		MapObject::MapObject(const map_ptr_t & _mapPtr, const size_t _size, const filtered_obj_t inFilter) :
			ObjectAbstract(),
			state(inFilter == FILTERED ? Gaussian(_mapPtr->x(), _mapPtr->P(), _mapPtr->reserveStates(_size)) : _size)
		{
				category = MAPPABLE_OBJECT;
		}

		MapObject::MapObject(const map_ptr_t & _mapPtr, const MapObject & prevObj, const size_t _size, jblas::ind_array & _icomp) :
			ObjectAbstract(),
			state( Gaussian(_mapPtr->x(), _mapPtr->P(),
					_mapPtr->convertStates(prevObj.state.ia(),_size,_icomp)) )
		{
				category = MAPPABLE_OBJECT;
		}

	}
}
