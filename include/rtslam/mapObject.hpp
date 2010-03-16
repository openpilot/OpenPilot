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
				 * With this constructor the object is not linked to any map. Use it for sensors.
				 * \param _size the state size.
				 */
				MapObject(const std::size_t _size);

				/**
				 * Remote constructor from remote map and indirect array
				 * \param _map the remote map
				 * \param _begin the first index pointing to the remote storage
				 */
				MapObject(MapAbstract & _map, const size_t _size);



				/**
				 * Mandatory virtual destructor
				 */
				virtual ~MapObject() {
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
				inline std::size_t & id() {
					return id_;
				}
				inline std::string & type() {
					return type_;
				}
				inline std::string & name() {
					return name_;
				}
				inline std::string & categoryName() {
					return categoryName_;
				}

				inline static std::size_t size() {
					return 0;
				}

				inline void setup(size_t _id, std::string & _type, std::string & _name) {
					id(_id);
					type(_type);
					name(_name);
				}


				/**
				 * Operator << for class MapObject.
				 * It shows different information of the object.
				 */
				friend std::ostream& operator <<(std::ostream & s, jafar::rtslam::MapObject & obj) {
					s << obj.categoryName() << " " << obj.id() << ": ";
					if (obj.name().size() > 0)
						s << obj.name() << ", ";
					s << "of type " << obj.type() << std::endl;
					s << ".state:  " << obj.state;
					return s;
				}

		};

	}
}

#endif /* MAPOBJECT_HPP_ */
