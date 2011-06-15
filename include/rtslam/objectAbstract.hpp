/**
 * \file objectAbstract.hpp
 *
 * Header file for abstract objects.
 *
 * \date 17/03/2010
 * \author jsola
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

				friend std::ostream& operator <<(std::ostream & s, ObjectAbstract const & obj);

			public:
				/**
				 * Use this enum in constructors to indicate that the object is devoted to simulation.
				 */
				typedef enum {
					FOR_SIMULATION ///< Force simulation object
				} simulation_t;

				enum category_enum { WORLD, MAP, OBJECT, MAPPABLE_OBJECT, ROBOT, SENSOR, LANDMARK, OBSERVATION, FEATURE, APPEARANCE, RAW, DATA_MANAGER};

			private:
				std::size_t id_;

				std::string name_;

			protected:
				category_enum category;

			public:
				ObjectAbstract();
				virtual ~ObjectAbstract();
				inline void id(std::size_t _id) {
					id_ = _id;
				}
				inline void name(std::string _name) {
					name_ = _name;
				}
				inline const std::size_t & id() const {
					return id_;
				}
				inline std::size_t & id() {
					return id_;
				}
				inline const std::string & name() const {
					return name_;
				}
				inline std::string & name() {
					return name_;
				}
				virtual std::string categoryName() const {
					return "OBJECT";
				}
				virtual std::string typeName() const {
					return "Undefined";
				}

				inline void setup(const category_enum & _category, const size_t _id, const std::string & _name) {
					category = _category;
					id(_id);
					name(_name);
				}
				inline void setup(const size_t _id, const std::string & _name) {
					id(_id);
					name(_name);
				}
				virtual void destroyDisplay();
				std::vector<display::DisplayDataAbstract*> displayData;
		};
	}
}

#endif /* OBJECTABSTRACT_HPP_ */
