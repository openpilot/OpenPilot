/**
 * \file objectAbstract.hpp
 *
 *  Created on: 17/03/2010
 *     \author: jsola@laas.fr
 *
 *
 *  ## Add a description here ##
 *
 * \ingroup rtslam
 */

#ifndef OBJECTABSTRACT_HPP_
#define OBJECTABSTRACT_HPP_

#include <iostream>

namespace jafar {
	namespace rtslam {

		/**
		 * Class for generic objects in rtslam.
		 * This class defines standard members:
		 * - identifier (eg. 1)
		 * - category type name (eg. SENSOR)
		 * - type (eg. Pin-hole-camera)
		 * - name (eg. Marlin)
		 * \ingroup rtslam
		 */
		class ObjectAbstract {
			private:
				std::size_t id_;
				std::string categoryName_;
				std::string type_;
				std::string name_;

			public:
				virtual ~ObjectAbstract(){}
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
				inline void setup(std::string _categoryName, size_t _id, std::string & _type, std::string & _name) {
					categoryName(_categoryName);
					id(_id);
					type(_type);
					name(_name);
				}
				/**
				 * Operator << for class ObjectAbstract.
				 * It shows different information of the object.
				 */
				friend std::ostream& operator <<(std::ostream & s, jafar::rtslam::ObjectAbstract & obj) {
					s << obj.categoryName() << " " << obj.id() << ": ";
					if (obj.name().size() > 0)
						s << obj.name() << ", ";
					s << "of type " << obj.type();
					return s;
				}



		};

	}
}

#endif /* OBJECTABSTRACT_HPP_ */
