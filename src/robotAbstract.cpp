/**
 * robotAbstract.cpp
 *
 * \date 08/03/2010
 * \author jsola@laas.fr
 *
 *  \file robotAbstract.cpp
 *
 *  ## Add a description here ##
 *
 * \ingroup rtslam
 */

#include "rtslam/robotAbstract.hpp"
#include "rtslam/sensorAbstract.hpp"
#include "rtslam/mapAbstract.hpp"

#include <boost/shared_ptr.hpp>

namespace jafar {
	namespace rtslam {
		using namespace std;


		/*
		 * Operator << for class RobotAbstract.
		 * It shows different information of the robot.
		 */
		ostream& operator <<(ostream & s, RobotAbstract & rob) {
			s << rob.categoryName() << " " << rob.id() << ": ";
			if (rob.name().size() > 0)
				s << rob.name() << ", ";
			s << "of type " << rob.type() << endl;
			s << ".state:  " << rob.state << endl;
			s << ".pose :  " << rob.pose << endl;
			s << ".sens : [";
			sensors_ptr_set_t::iterator senIter;
			for (senIter = rob.sensorsPtrSet.begin(); senIter != rob.sensorsPtrSet.end(); senIter++)
				s << " " << senIter->first << " ";
			s << "]";
			return s;
		}


		/*
		 * Remote constructor from remote map and size of control vector.
		 */
		RobotAbstract::RobotAbstract(const map_ptr_t & _mapPtr, const size_t _size_state, const size_t _size_control, const size_t _size_pert) :
			MapObject(_mapPtr, _size_state),
			mapPtr(_mapPtr),
			pose(state, jmath::ublasExtra::ia_set(0, 7)),
			control(_size_control),
			perturbation(_size_pert),
			XNEW_x(_size_state, _size_state),
			XNEW_pert(_size_state, _size_pert),
			Q(_size_state, _size_state)
		{
			constantPerturbation = false;
			categoryName("ROBOT"); // robot is categorized
		}


		/*
		 * Add a sensor to this robot
		 */
		void RobotAbstract::linkToSensor(const sensor_ptr_t & _senPtr) {
			sensorsPtrSet[_senPtr->id()] = _senPtr;
		}

		void RobotAbstract::linkToMap(const map_ptr_t & _mapPtr) {
			mapPtr = _mapPtr;
		}

				void RobotAbstract::move() {
					//move_func(); // x = F(x, u); Update Jacobians dxnew/dx and dxnew/du
					vec x = state.x();
					vec n = perturbation.x();
					vec xnew(x.size());

					move_func(x, control, n, dt_or_dx, xnew, XNEW_x, XNEW_pert);
					state.x() = xnew;

					if (!constantPerturbation)
						computeStatePerturbation();

					mapPtr->filter.predict(mapPtr->ia_used_states(), XNEW_x, state.ia(), Q); // P = F*P*F' + Q

				}

//		void RobotAbstract::move() {
//			//move_func(); // x = F(x, u); Update Jacobians dxnew/dx and dxnew/du
//			vec x = state.x();
//			vec n = perturbation.x();
//			move_func(x, control, n, dt_or_dx, x, XNEW_x, XNEW_pert);
//			state.x() = x;
//			if (!constantPerturbation)
//				computeStatePerturbation();
//			mapPtr->filter.predict(mapPtr->ia_used_states(), XNEW_x, state.ia(), Q); // P = F*P*F' + Q
//		}

		void RobotAbstract::computeStatePerturbation() {
			Q = jmath::ublasExtra::prod_JPJt(perturbation.P(), XNEW_pert);
		}

		void RobotAbstract::exploreSensors() {
			for (sensors_ptr_set_t::iterator senIter = sensorsPtrSet.begin(); senIter != sensorsPtrSet.end(); senIter++) {
				sensor_ptr_t senPtr = senIter->second;
				cout << "exploring sen: " << senPtr->id() << endl;

				senPtr->acquireRaw();
				senPtr->processRaw();

			}
		}

	}
}
