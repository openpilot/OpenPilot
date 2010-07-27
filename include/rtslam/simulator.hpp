/**
 * \file simulator.hpp
 *
 * Simulator
 * This is part of the ad-hoc simulator.
 *
 * \author croussil@laas.fr
 * \date 24/07/2010
 *
 * \ingroup rtslam
 */

#ifndef SIMULATOR_HPP_
#define SIMULATOR_HPP_

#include "kernel/IdFactory.hpp"
#include "jmath/jblas.hpp"

#include "rtslam/quatTools.hpp"
#include "rtslam/simulatorObjects.hpp"

namespace jafar {
namespace rtslam {
namespace simu {


/**
The simulated environment and slam config
*/
class AdhocSimulator
{
	private:
		std::map<size_t,simu::Robot*> robots;
		std::map<size_t,simu::Landmark*> landmarks;
		IdFactory lmkIdFactory;
		
	public:
		AdhocSimulator(const std::string & configFile);
		AdhocSimulator() {}
		~AdhocSimulator()
		{
			for(std::map<size_t,simu::Robot*>::iterator it = robots.begin(); it != robots.end(); ++it) delete it->second;
			for(std::map<size_t,simu::Landmark*>::iterator it = landmarks.begin(); it != landmarks.end(); ++it) delete it->second;
		}
		
		void addRobot(simu::Robot *robot) { robots[robot->id] = robot; }
		void addSensor(size_t robId, simu::Sensor *sensor)
		{
			std::map<size_t,simu::Robot*>::iterator it = robots.find(robId);
			if (it != robots.end()) return it->second->addSensor(sensor);
		}
		void addLandmark(simu::Landmark *landmark) { landmark->id = lmkIdFactory.getId(); landmarks[landmark->id] = landmark; }
		bool addObservationModel(size_t robId, size_t senId, LandmarkAbstract::geometry_t lmkType, ObservationModelAbstract *obsModel)
		{
			std::map<size_t,simu::Robot*>::iterator itRob = robots.find(robId);
			if (itRob == robots.end()) return false;
			std::map<size_t,simu::Sensor*>::const_iterator itSen = itRob->second->sensors.find(senId);
			if (itSen == itRob->second->sensors.end()) return false;
			itSen->second->addObservationModel(lmkType, obsModel);
			return true;
		}
		
		bool hasEnded(size_t robId, size_t senId, double t) const
		{
			std::map<size_t,simu::Robot*>::const_iterator itRob = robots.find(robId);
			if (itRob == robots.end()) return true;
			std::map<size_t,simu::Sensor*>::const_iterator itSen = itRob->second->sensors.find(senId);
			if (itSen == itRob->second->sensors.end()) return true;
			return (itRob->second->hasEnded(t) && itSen->second->hasEnded(t));
		}
		
		jblas::vec getRobotPose(size_t id, double t) const
		{
			std::map<size_t,simu::Robot*>::const_iterator it = robots.find(id);
			if (it != robots.end()) return it->second->getPose(t); else return jblas::vec();
		}
		
		jblas::vec getSensorPose(size_t robId, size_t senId, double t) const
		{
			std::map<size_t,simu::Robot*>::const_iterator itRob = robots.find(robId);
			if (itRob == robots.end()) return jblas::vec();
			return itRob->second->getSensorPose(senId, t);
		}
		
		jblas::vec getLandmarkPose(size_t id, double t) const
		{
			std::map<size_t,simu::Landmark*>::const_iterator it = landmarks.find(id);
			if (it != landmarks.end()) return it->second->getPose(t); else return jblas::vec();
		}
		LandmarkAbstract::geometry_t getLandmarkType(size_t id) const
		{
			std::map<size_t,simu::Landmark*>::const_iterator it = landmarks.find(id);
			if (it != landmarks.end()) return it->second->type; else return (LandmarkAbstract::geometry_t)-1;
		}
		
		bool getObservationPose(jblas::vec &obsPose, size_t robId, size_t senId, size_t lmkId, double t) const
		{
			std::map<size_t,simu::Robot*>::const_iterator itRob = robots.find(robId);
			if (itRob == robots.end()) return false;
			std::map<size_t,simu::Sensor*>::const_iterator itSen = itRob->second->sensors.find(senId);
			if (itSen == itRob->second->sensors.end()) return false;
			std::map<size_t,simu::Landmark*>::const_iterator itLmk = landmarks.find(lmkId);
			if (itLmk == landmarks.end()) return false;
			std::map<LandmarkAbstract::geometry_t, ObservationModelAbstract*>::const_iterator itMod = itSen->second->obsModels.find(itLmk->second->type);
			if (itMod == itSen->second->obsModels.end()) return false;
			
			jblas::vec6 robpose_e = itRob->second->getPose(t); std::swap(robpose_e(3), robpose_e(5)); // FIXME-EULER-CONVENTION
			jblas::vec7 robpose_q = quaternion::e2q_frame(robpose_e);
			jblas::vec6 senpose_e = itSen->second->getPose(t); std::swap(senpose_e(3), senpose_e(5)); // FIXME-EULER-CONVENTION
			jblas::vec7 senpose_q = quaternion::e2q_frame(senpose_e);
			jblas::vec7 senGlobPose = quaternion::composeFrames(robpose_q, senpose_q);
			jblas::vec lmkPose = itLmk->second->getPose(t);
			jblas::vec nobs;
			itMod->second->project_func(senGlobPose, lmkPose, obsPose, nobs);
			bool r = itMod->second->predictVisibility_func(obsPose, nobs);
//JFR_DEBUG("robot pose " << robpose_e << ", sensor pose " << senpose_e << ", sensor global pose q " << senGlobPose << ", landmark " << lmkId << "/" << itLmk->second->id << " pose " << lmkPose << ", observation pose " << obsPose << "(nobs " << nobs << "), visibility " << r);
			return r;
		}
		
		
		raw_ptr_t getRaw(size_t robId, size_t senId, double t) //const
		{
			// TODO optimize avoiding to repeat computation of sensor pose
			boost::shared_ptr<simu::RawSimu> raw(new simu::RawSimu());
			raw->timestamp = t;
			jblas::vec pose;
			for(std::map<size_t,simu::Landmark*>::const_iterator it = landmarks.begin(); it != landmarks.end(); ++it)
			{
				size_t lmkId = it->second->id;
				if (getObservationPose(pose, robId, senId, lmkId, t))
					raw->obs[lmkId] = featuresimu_ptr_t(new FeatureSimu(pose, it->second->type, lmkId));
			}
std::cout << "simulation has generated a raw at time " << t << " with " << raw->obs.size() << " obs ; robot pose " << robots[1]->getPose(t) << std::endl;
			return raw;
		}
};


}}}

#endif

