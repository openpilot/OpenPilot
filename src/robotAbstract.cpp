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

		IdFactory RobotAbstract::robotIds = IdFactory();

		/*
		 * Operator << for class RobotAbstract.
		 * It shows different information of the robot.
		 */
		ostream& operator <<(ostream & s, RobotAbstract & rob) {
			s << rob.categoryName() << " " << rob.id() << ": ";
			if (rob.name().size() > 0)
				s << rob.name() << ", ";
			s << "of type " << rob.typeName() << endl;
			s << ".state:  " << rob.state << endl;
			s << ".pose :  " << rob.pose << endl;
			s << ".sens : [";
			for (RobotAbstract::SensorList::const_iterator senIter = rob.sensorList().begin();
						senIter != rob.sensorList().end(); senIter++)
				s << " " << (*senIter)->id() << " "; // Print the address of the sensor.
			s << "]";
			return s;
		}


		/*
		 * Remote constructor from remote map and size of control vector.
		 */
		RobotAbstract::RobotAbstract(const map_ptr_t & _mapPtr, const size_t _size_state, const size_t _size_control, const size_t _size_pert) :
			MapObject(_mapPtr, _size_state),
			pose(state, jmath::ublasExtra::ia_set(0, 7)),
			control(_size_control),
			perturbation(_size_pert),
			XNEW_x(_size_state, _size_state),
			XNEW_pert(_size_state, _size_pert),
			Q(_size_state, _size_state)
		{
			constantPerturbation = false;
			category = ROBOT;
		}

		RobotAbstract::RobotAbstract(const simulation_t dummy, const map_ptr_t & _mapPtr, const size_t _size_state, const size_t _size_control, const size_t _size_pert) :
			MapObject(_mapPtr, _size_state, UNFILTERED),
			pose(state, jmath::ublasExtra::ia_set(0, 7)),
			control(_size_control),
			perturbation(_size_pert),
			XNEW_x(_size_state, _size_state),
			XNEW_pert(_size_state, _size_pert),
			Q(_size_state, _size_state)
		{
			constantPerturbation = true;
			category = ROBOT;
		}


				void RobotAbstract::move() {
					vec x = state.x();
					vec n = perturbation.x();
					vec xnew(x.size());

					move_func(x, control, n, dt_or_dx, xnew, XNEW_x, XNEW_pert);
					state.x() = xnew;

					if (mapPtr()->filterPtr){

						if (!constantPerturbation)
							computeStatePerturbation();

						mapPtr()->filterPtr->predict(mapPtr()->ia_used_states(), XNEW_x, state.ia(), Q); // P = F*P*F' + Q
					}
				}


		void RobotAbstract::computeStatePerturbation() {
			Q = jmath::ublasExtra::prod_JPJt(perturbation.P(), XNEW_pert);
		}

		void RobotAbstract::exploreSensors() const {
			for (SensorList::const_iterator senIter = sensorList().begin(); senIter != sensorList().end(); senIter++) {
				sensor_ptr_t senPtr = *senIter;

				senPtr->acquireRaw();
				senPtr->processRaw();

			}
		}

	}
}
