/**
 * \file sensorPinHole.hpp
 *
 * Header file for observation factories
 *
 * \author croussil@laas.fr
 * \date 16/06/2010
 *
 * \ingroup rtslam
 */

#ifndef OBSERVATION_FACTORY_HPP_
#define OBSERVATION_FACTORY_HPP_


#include "rtslam/observationPinHoleEuclideanPoint.hpp"
#include "rtslam/observationPinHoleAnchoredHomogeneous.hpp"
#include "rtslam/sensorAbstract.hpp"
#include "rtslam/landmarkAbstract.hpp"

namespace jafar {
namespace rtslam {


/*
A maker is a class that inherit from ObservationMakerAbstract,
that stores in internal data some parameters that it got from the constructor parameters,
and implements the create method
*/

class ObservationMakerAbstract
{
	protected:
		typedef std::pair<SensorAbstract::type_enum, LandmarkAbstract::type_enum> SenLmk;
		SenLmk type;
		friend class ObservationFactory;
	
	public:
		ObservationMakerAbstract(SensorAbstract::type_enum sensor_type, LandmarkAbstract::type_enum landmark_type):
			type(sensor_type, landmark_type) {}
		
		virtual observation_ptr_t create(sensor_ptr_t senPtr, landmark_ptr_t lmkPtr) = 0;
};

typedef boost::shared_ptr<ObservationMakerAbstract> observation_maker_ptr_t;



class ObservationFactory
{
	private:
		typedef std::pair<SensorAbstract::type_enum, LandmarkAbstract::type_enum> SenLmk;
		typedef std::map<SenLmk, observation_maker_ptr_t> CreatorsMap;
		CreatorsMap creators;
		
	public:
		void addMaker(observation_maker_ptr_t maker)
		{
			creators[maker->type] = maker;
		}
		
		observation_ptr_t create(sensor_ptr_t senPtr, landmark_ptr_t lmkPtr)
		{
			CreatorsMap::iterator it = creators.find(SenLmk(senPtr->type,lmkPtr->type));
			if (it == creators.end())
				JFR_ERROR(RtslamException, RtslamException::UNKNOWN_TYPES_FOR_FACTORY, "ObservationFactory: do not know how to create an observation from sensor type " << senPtr->type << " and landmark type " << lmkPtr->type);
			return it->second->create(senPtr, lmkPtr);
		}
};



//////////////////////////////////////////////////////


class PhEuc_ObservationMaker: public ObservationMakerAbstract
{
	private:
		int match_size;
		
	public:
		PhEuc_ObservationMaker(int match_size):
			ObservationMakerAbstract(SensorAbstract::PINHOLE, LandmarkAbstract::PNT_EUC), 
			match_size(match_size) {}
			
		observation_ptr_t create(sensor_ptr_t senPtr, landmark_ptr_t lmkPtr)
		{
			observation_ptr_t res(new ObservationPinHoleEuclideanPoint(senPtr, lmkPtr));
			res->setup(senPtr, lmkPtr, match_size);
			return res;
		}
};


class PhAhp_ObservationMaker: public ObservationMakerAbstract
{
	private:
		int match_size;
		
	public:
		PhAhp_ObservationMaker(int match_size):
			ObservationMakerAbstract(SensorAbstract::PINHOLE, LandmarkAbstract::PNT_AH),
			match_size(match_size) {}
			
		observation_ptr_t create(sensor_ptr_t senPtr, landmark_ptr_t lmkPtr)
		{
			observation_ptr_t res(new ObservationPinHoleEuclideanPoint(senPtr, lmkPtr));
			res->setup(senPtr, lmkPtr, match_size);
			return res;
		}
};


}}

#endif

