/**
 * \file sensorAbstract.cpp
 * \date 10/03/2010
 * \author jsola
 * \ingroup rtslam
 */

#include "boost/assign/std/vector.hpp"
#include "boost/shared_ptr.hpp"
#include "jmath/indirectArray.hpp"
#include "rtslam/sensorAbstract.hpp"
#include "rtslam/robotAbstract.hpp"
#include "rtslam/observationAbstract.hpp"
#include "rtslam/quatTools.hpp"

#include "jmath/angle.hpp"
#include <vector>


namespace jafar {
	namespace rtslam {
		using namespace std;
		using namespace boost;
		using namespace ublas;
		using namespace ublasExtra;

		IdFactory SensorAbstract::sensorIds = IdFactory();

		/*
		 * Operator << for class SensorAbstract.
		 * It shows different information of the sensor.
		 */
		std::ostream& operator <<(std::ostream & s,
		    SensorAbstract const & sen) {
			s << sen.categoryName() << " " << sen.id() << ": ";
			if (sen.name().size() > 0) s << sen.name() << ", ";
			s << "of type " << sen.typeName() << std::endl;
			s << ".pose :  " << sen.pose << endl;
			if (sen.state.storage() == Gaussian::REMOTE) s << endl << ".ia_rs: "
			    << sen.ia_globalPose;
			s << ".in robot: " << sen.robot().id();
			return s;
		}

		SensorAbstract::SensorAbstract(const robot_ptr_t & _robPtr, const filtered_obj_t inFilter):
			MapObject(_robPtr->mapPtr(), 7, inFilter), integrate_all(false), pose(state, Gaussian::REMOTE),
			ia_globalPose(inFilter == FILTERED ? ia_union(_robPtr->pose.ia(), pose.ia()) : _robPtr->pose.ia())
		{
			category = SENSOR;
			isInFilter = (inFilter == FILTERED);
			id(sensorIds.getId());
		}

		void SensorAbstract::setPose(double x, double y, double z, double rollDeg,
		    double pitchDeg, double yawDeg) {
			const double pos_[3] = { x, y, z };
			const double euler_[3] = { jmath::degToRad(rollDeg),
			    jmath::degToRad(pitchDeg), jmath::degToRad(yawDeg) };
			ublas::subrange(pose.x(), 0, 3) = createVector<3> (pos_);
			ublas::subrange(pose.x(), 3, 7)
			    = quaternion::e2q(createVector<3> (euler_));
		}

		void SensorAbstract::setPoseStd(double x, double y, double z, double rollDeg,
		    double pitchDeg, double yawDeg, double xStd, double yStd, double zStd,
		    double rollDegStd, double pitchDegStd, double yawDegStd) {

			const double pos_[3] = { x, y, z };
			const double euler_[3] = { jmath::degToRad(rollDeg), jmath::degToRad(pitchDeg), jmath::degToRad(yawDeg) };

			// convert euler pose to quat pose
			ublas::subrange(pose.x(), 0, 3) = createVector<3> (pos_);

			vec3 euler = createVector<3> (euler_);
			ublas::subrange(pose.x(), 3, 7) = quaternion::e2q(euler);

			// convert euler uncertainty to quaternion uncertainty
			const double posStd_[3] = { xStd, yStd, zStd };
			const double eulerStd_[3] = { jmath::degToRad(rollDegStd),
			    jmath::degToRad(pitchDegStd), jmath::degToRad(yawDegStd) };

			vec3 eulerStd = createVector<3> (eulerStd_);

			Gaussian E(3);	E.std(eulerStd);
			vec4 q;
			mat Q_e(4, 3);

			quaternion::e2q(euler, q, Q_e);

			// write pose
			subrange(pose.P(), 0,3, 0,3) = createSymMat<3>(posStd_);
			subrange(pose.P(), 3,7, 3,7) = prod(Q_e, prod<mat>(E.P(), trans(Q_e)));

		}


		/*
		 * Get sensor pose in global frame.
		 */
		vec7 SensorAbstract::globalPose() {
			vec7 globPose;
			jblas::vec7 robotPose = robotPtr()->pose.x();
			jblas::vec7 sensorPose = pose.x();
			globPose = quaternion::composeFrames(robotPose, sensorPose);
			return globPose;
		}

		/*
		 * Get sensor pose in global frame.
		 */
		void SensorAbstract::globalPose(jblas::vec7 & senGlobalPos,
		    jblas::mat & SG_rs) {
			using boost::numeric::ublas::range;
			jblas::vec7 robotPose = robotPtr()->pose.x();
			jblas::vec7 sensorPose = pose.x();

			if (state.storage() == Gaussian::LOCAL) {
				// Sensor is not in the map. Jacobian only wrt robot.
				senGlobalPos = quaternion::composeFrames(robotPose, sensorPose);
				quaternion::composeFrames_by_dglobal(robotPose, sensorPose, SG_rs);
			} else {
				// Sensor is in the map. Give composed Jacobian.
				jblas::mat PG_r(7, 7), PG_s(7, 7);
				quaternion::composeFrames(robotPose, sensorPose, senGlobalPos, PG_r,
				                          PG_s);
				project(SG_rs, range(0, 7), range(0, 7)) = PG_r;
				project(SG_rs, range(0, 7), range(7, 14)) = PG_s;
			}
		}
		
		void SensorExteroAbstract::process(unsigned id)
		{
			// get data
			hardwareSensorPtr->getRaw(id, rawPtr);
			rawCounter++;
			
			// observe
			for (DataManagerList::iterator dmaIter = dataManagerList().begin(); dmaIter != dataManagerList().end(); ++dmaIter)
			{
				data_manager_ptr_t dmaPtr = *dmaIter;
				dmaPtr->processKnown(rawPtr);
				dmaPtr->mapManagerPtr()->manage();
				dmaPtr->detectNew(rawPtr);
			}
			
			//hardwareSensorPtr->release();
		}


	}
}
