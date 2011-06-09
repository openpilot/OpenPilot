/**
 * \file simulatorObjects.hpp
 *
 * All the objects to simulate a real physical object (robot, sensor...)
 * They contain the true state.
 * This is part of the ad-hoc simulator.
 *
 * \date 24/07/2010
 * \author croussil
 *
 * \ingroup rtslam
 */

#ifndef SIMUOBJECTS_HPP_
#define SIMUOBJECTS_HPP_

#include "kernel/dataLog.hpp"
#include "jmath/jblas.hpp"

namespace jafar {
namespace rtslam {
namespace simu {

	class AdhocSimulator;
	
	class Object
	{
		public:
			size_t id; ///< associated real slam object id
	};
	
	class MapObject: public simu::Object
	{
		private:
			size_t size;
		public:
			MapObject(size_t size): size(size) {}
			virtual jblas::vec getPose(double t) const = 0;
			virtual bool hasEnded(double t) const { return true; }
//			jblas::vec pose;
	};
	
	typedef ublas::bounded_vector<double,12> vec12;
	struct Waypoint
	{
		double t;
		vec12 pose; ///< x,y,z,yaw,pitch,roll vx,vy,vz,vyaw,vpitch,vroll
	};
	typedef std::vector<Waypoint> Trajectory;
	
	class MobileObject: public simu::MapObject, public kernel::DataLoggable
	{
		private:
			mutable double _t;
			Trajectory traj;
			void getWaypointsIndexes(double t, int &i_before, int &i_after) const
			{
				int n = traj.size();
				if (n == 0) JFR_ERROR(RtslamException, RtslamException::SIMU_ERROR, "simu trajectory is empty");
				if (t <= traj[0].t) { i_before = i_after = 0; return; }
				if (t >= traj[n-1].t) { i_before = i_after = n-1; return; }
				
				// dichotomy
				int a,b;
				for (a = 0, b = n-1; b-a > 1; )
				{
					int i = (a+b)/2;
					if (traj[i].t < t) a = i; else b = i;
				}
				i_before = a;
				i_after = b;
			}
			
		public:
			MobileObject(size_t size): MapObject(size) {}
			void clear() { traj.clear(); }
			/**
			@warning a waypoint must be consistent, meaning that the time necessary to change state for each component must be the same
			@return whether the point was coherent and was accepted
			*/
			bool hasEnded(double t) const { if (traj.size() == 0) return true; return (t > traj.back().t); }
			void addWaypoint(double x, double y, double z, double yaw, double pitch, double roll,
			                 double vx, double vy, double vz, double vyaw, double vpitch, double vroll)
			{
				double pose_[12] = {x, y, z, yaw, pitch, roll, vx, vy, vz, vyaw, vpitch, vroll};
				jblas::vec pose = createVector<12>(pose_);
				addWaypoint(pose);
			}

			bool addWaypoint(jblas::vec pose)
			{
				JFR_ASSERT(pose.size() == 12, "pose must be size 12 (pos,ori,vpos,vori)");
				Waypoint p; p.pose.clear();
				p.pose = pose;
				if (traj.size() == 0)
				{
					p.t = 0.;
				} else
				{
					Waypoint &lastp = traj.back();
					p.t = 0.;
					// find max time
					for(int i = 0; i < 6; ++i)
					{
						double avg_vel = (p.pose(i+6) + lastp.pose(i+6)) / 2;
						double t = (p.pose(i) - lastp.pose(i)) / avg_vel;
						if (t > p.t) p.t = t;
					}
					// correct all speeds wrt max time
					for(int i = 0; i < 6; ++i)
					{
						double avg_vel = (p.pose(i) - lastp.pose(i)) / p.t;
						p.pose(i+6) = 2*avg_vel - lastp.pose(i+6);
					}
					
					p.t += lastp.t;
				}
				traj.push_back(p);
				return true;
			}
			
			jblas::vec getPose(double t) const
			{
				_t = t;
				int a, b;
				getWaypointsIndexes(t,a,b);
				if (a == b) return ublas::subrange(traj[b].pose,0,6);
				
				jblas::vec p1 = ublas::subrange(traj[a].pose,0,6 );
				jblas::vec v1 = ublas::subrange(traj[a].pose,6,12);
				jblas::vec v2 = ublas::subrange(traj[b].pose,6,12);
				double t1 = traj[a].t, t2 = traj[b].t;
// JFR_DEBUG("robot " << id << " getPose: a/b " << a << "/" << b << ", t1/t2 " << t1 << "/" << t2 << ", p1 " << p1 << ", v1 " << v1 << ", v2 " << v2);
				return p1 + (t2*v1-t1*v2)*((t-t1)/(t2-t1)) + 0.5*(v2-v1)*((t*t-t1*t1)/(t2-t1));
			}
			jblas::vec getSpeed(double t) const
			{
				_t = t;
				int a, b;
				getWaypointsIndexes(t,a,b);
				if (a == b) return ublas::subrange(traj[b].pose,6,12);
				
				double tp = (t - traj[a].t) / (traj[b].t - traj[a].t);
				return (1-tp) * ublas::subrange(traj[a].pose,6,12) + tp * ublas::subrange(traj[b].pose,6,12);
			}
			jblas::vec getAcc(double t) const
			{
				_t = t;
				int a, b;
				getWaypointsIndexes(t,a,b);
				if (a == b) return jblas::zero_vec(6);
				
				double dt = traj[b].t - traj[a].t;
				return (ublas::subrange(traj[b].pose,6,12) - ublas::subrange(traj[a].pose,6,12)) / dt;
			}
		
		
			virtual void writeLogHeader(kernel::DataLogger& log) const
			{
				log.writeLegendTokens("simu_x simu_y simu_z");
				log.writeLegendTokens("simu_yaw simu_pitch simu_roll");
			}
			virtual void writeLogData(kernel::DataLogger& log) const
			{
				jblas::vec pose = getPose(_t);
				for(int i = 0 ; i < 6 ; ++i) log.writeData(pose(i));
			}

	};
	
	
	class Sensor: public simu::MapObject
	{
		private:
			jblas::vec pose;
			// the obsModels contain a reference to a slam sensor, but which can be a different object
			// than the one used by slam in order to introduce noise in sensor calibration
			std::map<LandmarkAbstract::geometry_t, ObservationModelAbstract*> obsModels;
			
		public:
			Sensor(size_t id, jblas::vec pose, sensor_ptr_t slamSensor): MapObject(pose.size()), pose(pose) { this->id = id; }
			~Sensor()
			{
				for(std::map<LandmarkAbstract::geometry_t, ObservationModelAbstract*>::iterator it = obsModels.begin(); it != obsModels.end(); ++it) delete it->second;
			}
			
			void addObservationModel(LandmarkAbstract::geometry_t lmkType, ObservationModelAbstract *obsModel)
				{ obsModels[lmkType] = obsModel; }
			jblas::vec getPose(double t) const { return pose; }
			
			friend class AdhocSimulator;
	};
	
	class Robot: public simu::MobileObject
	{
		private:
			std::map<size_t,simu::Sensor*> sensors;
		public:
			Robot(size_t id, size_t size): MobileObject(size) { this->id = id; }
			~Robot()
			{
				for(std::map<size_t,simu::Sensor*>::iterator it = sensors.begin(); it != sensors.end(); ++it) delete it->second;
			}
			void addSensor(simu::Sensor *sensor) { sensors[sensor->id] = sensor; }
			jblas::vec getSensorPose(size_t senId, double t) const
			{
				std::map<size_t,simu::Sensor*>::const_iterator itSen = sensors.find(senId);
				if (itSen == sensors.end()) return jblas::vec();
				return itSen->second->getPose(t);
			}
			
			friend class AdhocSimulator;
	};
	
	class Landmark: public simu::MapObject
	{
		private:
			jblas::vec pose;
		public:
			LandmarkAbstract::geometry_t type;
		public:
			Landmark(LandmarkAbstract::geometry_t type, jblas::vec pose): MapObject(pose.size()), pose(pose),type(type) {}
			jblas::vec getPose(double t) const { return pose; }
			
	};
	
	
	
/*
	class Observation: public simu::Object
	{
		public:
			geometry_t type;
			jblas::vec meas;
	};
*/


}}}


#endif

