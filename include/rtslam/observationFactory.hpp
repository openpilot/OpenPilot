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

#if 0
class ObservationFactory
{
	private:
		// data : create one struct per type of observation to create (but you can factorize)
		struct
		{
			int match_width;
			int match_height;
		} ph_pnt_params;
		
		
	private:
		// creators : create one function with same signature but different name for each type of observation to create
		observation_ptr_t create_ph_euc(sensor_ptr_t senPtr, landmark_ptr_t lmkPtr);
		observation_ptr_t create_ph_ahp(sensor_ptr_t senPtr, landmark_ptr_t lmkPtr);

	private:
		typedef observation_ptr_t (*create_fun)(sensor_ptr_t,landmark_ptr_t);
		typedef std::pair<SensorAbstract::type_enum, LandmarkAbstract::type_enum> sen_lmk;
		typedef std::map<sen_lmk, create_fun> creators_map;
		creators_map creators;
		
	public:
		// constructor : push all creators in the map
		ObservationFactory()
		{
			creators[std::pair<SensorAbstract::PINHOLE, LandmarkAbstract::PNT_EUC>] = &create_ph_euc;
			creators[std::pair<SensorAbstract::PINHOLE, LandmarkAbstract::PNT_AHP>] = &create_ph_ahp;
		}
		
		observation_ptr_t create(sensor_ptr_t senPtr, landmark_ptr_t lmkPtr)
		{
			creators_map::iterator it = creators.find(sen_lmk(senPtr->type,lmkPtr->type));
			if (it == creators.end())
				JFR_ERROR(RtslamException, RtslamException::UNKNOWN_TYPES_FOR_FACTORY, "ObservationFactory: do not know how to create an observation from sensor type " << senPtr->type << " and landmark type " << lmkPtr->type);
			return (**it)(senPtr, lmkPtr);
		}
		
		// setup data : create one struct per type of observation to create (but you can factorize)
		void setup_ph_pnt(int match_width, int match_height);




};

#endif


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



class ObservationFactory
{
	private:
		typedef std::pair<SensorAbstract::type_enum, LandmarkAbstract::type_enum> SenLmk;
		typedef std::map<SenLmk, ObservationMakerAbstract*> CreatorsMap;
		CreatorsMap creators;
		
	public:
		void addMaker(ObservationMakerAbstract *maker)
		{
			creators[maker->type] = maker;
		}
		
		observation_ptr_t create(sensor_ptr_t senPtr, landmark_ptr_t lmkPtr)
		{
			CreatorsMap::iterator it = creators.find(SenLmk(senPtr->type,lmkPtr->type));
			if (it == creators.end())
				JFR_ERROR(RtslamException, RtslamException::UNKNOWN_TYPES_FOR_FACTORY, "ObservationFactory: do not know how to create an observation from sensor type " << senPtr->type << " and landmark type " << lmkPtr->type);
			return (*it)->second->create(senPtr, lmkPtr);
		}
};



//////////////////////////////////////////////////////


class PhEuc_ObservationMaker: public ObservationMakerAbstract
{
	private:
		int match_width;
		int match_height;
		
	public:
		PhEuc_ObservationMaker(int match_width, int match_height):
			ObservationMakerAbstract(SensorAbstract::PINHOLE, LandmarkAbstract::PNT_EUC), 
			match_width(match_width), match_height(match_height) {}
			
		observation_ptr_t create(sensor_ptr_t senPtr, landmark_ptr_t lmkPtr)
		{
			return observation_ptr_t(new ObservationPinHoleEuclideanPoint(senPtr, lmkPtr, match_width, match_height));
		}
};


class PhAhp_ObservationMaker: public ObservationMakerAbstract
{
	private:
		int match_width;
		int match_height;
		
	public:
		PhAhp_ObservationMaker(int match_width, int match_height):
			ObservationMakerAbstract(SensorAbstract::PINHOLE, LandmarkAbstract::PNT_AHP), 
			match_width(match_width), match_height(match_height) {}
			
		observation_ptr_t create(sensor_ptr_t senPtr, landmark_ptr_t lmkPtr)
		{
			return observation_ptr_t(new ObservationPinHoleEuclideanPoint(senPtr, lmkPtr, match_width, match_height));
		}
};


}}

#endif

