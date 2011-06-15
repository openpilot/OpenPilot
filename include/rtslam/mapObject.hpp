/**
 *  \file mapObject.hpp
 *
 * \date 13/03/2010
 * \author jsola
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


		/**
		 * Class for generic mappable objects.
		 * \author jsola
		 * \ingroup rtslam
		 */
		class MapObject: public ObjectAbstract {

				friend std::ostream& operator <<(std::ostream & s, MapObject const & obj);

			public:
				/**
				 * Use this enum in constructors to indicate if the object's state vector should be part of the filter.
				 */
				typedef enum {
					FILTERED, ///<  Object's state vector is part of the SLAM filter.
					UNFILTERED ///< Object's state vector is not part of the SLAM filter.
				} filtered_obj_t;


				Gaussian state;

				/**
				 * Selectable constructor with \a inFilter flag.
				 * \param _mapPtr pointer to map
				 * \param _size size of the state vector
				 * \param inFilter flag selecting filtered or non-filtered state vector.
				 */
				MapObject(const map_ptr_t & _mapPtr, const size_t _size, const filtered_obj_t inFilter = FILTERED);
 		                /*
				 * Contructor by replacement: install the new object in place of the given arguments,
				 * using the same position in the filter. The previous object has to be FILTERED, so is the new
				 * object.
				 * \param _mapPtr pointer to map
				 * \parem _previousObj the object to be replaced in the filter. Should by FILTERED
				 * \param _size the new size of the state vector.
				 * \param _icomp the complementary of the new state wrt the previous state, ie the memory to be release when relaxing the previous object.
				 */
		    MapObject(const map_ptr_t & _mapPtr, const MapObject & _previousObj, const size_t _size, jblas::ind_array & _icomp);

				/**
				 * Mandatory virtual destructor
				 */
				virtual ~MapObject() {
				}

				void setup(const vec & _state, const vec & _stateStdDev){
					state.x(_state);
					state.std(_stateStdDev);
				}
				void setup(const vec & _state, const sym_mat & _stateCov){
					state.x(_state);
					state.P(_stateCov);
				}

				inline static size_t size() {
					return 0;
				}

				virtual std::string categoryName() const {
					return "MAP OBJECT";
				}

				/**
				 * A Map have a Reference Frame, all the objects must implements the reframe Function
				 */
//				virtual void reframe(vec7 localFrame, mat & X_xold) = 0 ;


		};

	}
}

#endif /* MAPOBJECT_HPP_ */
