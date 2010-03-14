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
#include "rtslam/mapAbstract.hpp"
#include "rtslam/gaussian.hpp"

namespace jafar {
	namespace rtslam {

		/**
		 * Class for generic mappable objects.
		 * \author jsola
		 * \ingroup rtslam
		 */
		class MapObject {
			private:
				std::size_t id_;
				std::string type_;
				std::string name_;
				std::string categoryName_;

			public:
				MapAbstract * map;
				Gaussian state;

				/**
				 * Local constructor from size.
				 * \param _size the state size.
				 */
				MapObject(std::size_t _size);

				/**
				 * Remote constructor from remote map and indirect array
				 * \param _map the remote map
				 * \param _iar the indirect array pointing to the remote storage
				 */
				MapObject(MapAbstract & _map, const jblas::ind_array & _iar);

				inline void setup(size_t _id, std::string & _name, std::string & _type) {
					id_ = _id;
					name_ = _name;
					type_ = _type;
				}

				inline void id(std::size_t _id) {
					id_ = _id;
				}
				inline void type(std::string _type) {
					type_ = _type;
				}
				inline void name(std::string _name) {
					name_ = _name;
				}
				inline void categoryName(std::string _categoryName) {
					categoryName_ = _categoryName;
				}
				inline std::size_t id(void) {
					return id_;
				}
				inline std::string type(void) {
					return type_;
				}
				inline std::string name(void) {
					return name_;
				}
				inline std::string categoryName(void) {
					return categoryName_;
				}

				inline static std::size_t size() {
					return 0;
				}

				/**
				 * Operator << for class MapObject.
				 * It shows different information of the object.
				 */
				friend std::ostream& operator <<(std::ostream & s, jafar::rtslam::MapObject & obj) {
					s << obj.categoryName() << " " << obj.id() << " of type " << obj.type() << std::endl;
					s << ".state:  " << obj.state << std::endl;
					return s;
				}

		};

	}
}

#endif /* MAPOBJECT_HPP_ */
