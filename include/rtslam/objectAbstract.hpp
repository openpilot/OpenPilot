/**
 * \file objectAbstract.hpp
 *
 * Header file for abstract objects.
 *
 * \date 17/03/2010
 * \author jsola@laas.fr
 *
 *
 * \ingroup rtslam
 */

#ifndef OBJECTABSTRACT_HPP_
#define OBJECTABSTRACT_HPP_

#include <string>
#include <vector>

#include "rtslam/rtSlam.hpp"

namespace jafar {
	namespace rtslam {

		namespace display {
			class DisplayDataAbstract;
		}

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

				friend std::ostream& operator <<(std::ostream & s, jafar::rtslam::ObjectAbstract & obj);

			public:
				/**
				 * Use this enum in constructors to indicate that the object is devoted to simulation.
				 */
				typedef enum {
					FOR_SIMULATION ///< Force simulation object
				} simulation_t;

			private:
				std::size_t id_;
				std::string categoryName_;
				std::string type_;
				std::string name_;

			public:
				ObjectAbstract();
				virtual ~ObjectAbstract() {
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
				inline void setup(const std::string & _categoryName, const size_t _id, const std::string & _type, const std::string & _name) {
					categoryName(_categoryName);
					id(_id);
					type(_type);
					name(_name);
				}
				inline void setup(const size_t _id, const std::string & _name) {
					id(_id);
					name(_name);
				}
				std::vector<display::DisplayDataAbstract*> displayData;
		};
	}
}

#endif /* OBJECTABSTRACT_HPP_ */
