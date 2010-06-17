/**
 * \file observationFactory.hpp
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
#include "rtslam/featurePoint.hpp"
#include "rtslam/descriptorImagePoint.hpp"

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
		
		virtual observation_ptr_t create(const sensor_ptr_t &senPtr, const landmark_ptr_t &lmkPtr) = 0;
		virtual feature_ptr_t createFeat(const sensor_ptr_t &senPtr, const landmark_ptr_t &lmkPtr) = 0;
		virtual descriptor_ptr_t createDesc(const sensor_ptr_t &senPtr, const landmark_ptr_t &lmkPtr, const feature_ptr_t &featPtr, const jblas::vec7 &senPoseInit, const observation_ptr_t &obsInitPtr) = 0;
};

typedef boost::shared_ptr<ObservationMakerAbstract> observation_maker_ptr_t;



class ObservationFactory
{
	private:
		typedef std::pair<SensorAbstract::type_enum, LandmarkAbstract::type_enum> SenLmk;
		typedef std::map<SenLmk, observation_maker_ptr_t> CreatorsMap;
		CreatorsMap creators;
		
		observation_maker_ptr_t getMaker(SenLmk senLmk)
		{
			CreatorsMap::iterator it = creators.find(senLmk);
			if (it == creators.end())
				JFR_ERROR(RtslamException, RtslamException::UNKNOWN_TYPES_FOR_FACTORY,
				          "ObservationFactory: do not know how to create an observation from sensor type "
				          << senLmk.first << " and landmark type " << senLmk.second);
			return it->second;
		}
		
	public:
		void addMaker(const observation_maker_ptr_t &maker)
		{
			creators[maker->type] = maker;
		}
		
		observation_ptr_t create(const sensor_ptr_t &senPtr, const landmark_ptr_t &lmkPtr)
		{
			observation_maker_ptr_t maker = getMaker(SenLmk(senPtr->type,lmkPtr->type));
			return maker->create(senPtr, lmkPtr);
		}
		
		feature_ptr_t createFeat(const sensor_ptr_t &senPtr, const landmark_ptr_t &lmkPtr)
		{
			observation_maker_ptr_t maker = getMaker(SenLmk(senPtr->type,lmkPtr->type));
			return maker->createFeat(senPtr, lmkPtr);
		}
		
		descriptor_ptr_t createDesc(const sensor_ptr_t &senPtr, const landmark_ptr_t &lmkPtr, const feature_ptr_t &featPtr, const jblas::vec7 &senPoseInit, const observation_ptr_t &obsInitPtr)
		{
			observation_maker_ptr_t maker = getMaker(SenLmk(senPtr->type,lmkPtr->type));
			return maker->createDesc(senPtr, lmkPtr, featPtr, senPoseInit, obsInitPtr);
		}
};



//////////////////////////////////////////////////////


template<class ObsType, class SenType, class LmkType, SensorAbstract::type_enum SenTypeId, LandmarkAbstract::type_enum LmkTypeId>
class ImagePointObservationMaker: public ObservationMakerAbstract
{
	private:
		
	public:
		ImagePointObservationMaker():
			ObservationMakerAbstract(SenTypeId, LmkTypeId) {}
			
		observation_ptr_t create(const sensor_ptr_t &senPtr, const landmark_ptr_t &lmkPtr)
		{
			SenType *sen = PTR_CAST<SenType*>(senPtr.get());
			boost::shared_ptr<ObsType> res(new ObsType(senPtr, lmkPtr));
			res->setup(senPtr, lmkPtr, sen->params.pixNoise, sen->params.patchSize);
			return res;
		}
		
		feature_ptr_t createFeat(const sensor_ptr_t &senPtr, const landmark_ptr_t &lmkPtr)
		{
			SenType *sen = PTR_CAST<SenType*>(senPtr.get());
			feature_ptr_t res(new FeatureImagePoint(sen->params.patchSize, sen->params.patchSize, CV_8U));
			return res;
		}
		
		descriptor_ptr_t createDesc(const sensor_ptr_t &senPtr, const landmark_ptr_t &lmkPtr, const feature_ptr_t &featPtr, const jblas::vec7 &senPoseInit, const observation_ptr_t &obsInitPtr)
		{
			//SenType *sen = PTR_CAST<SenType*>(senPtr.get());
			feat_img_pnt_ptr_t featSpecPtr = SPTR_CAST<FeatureImagePoint>(featPtr);
			descriptor_ptr_t res(new DescriptorImagePoint(featSpecPtr, senPoseInit, obsInitPtr));
			return res;
		}
};

typedef ImagePointObservationMaker<ObservationPinHoleEuclideanPoint, SensorPinHole, LandmarkEuclideanPoint, 
	SensorAbstract::PINHOLE, LandmarkAbstract::PNT_EUC> 
	PinholeEucpObservationMaker;
typedef ImagePointObservationMaker<ObservationPinHoleAnchoredHomogeneousPoint, SensorPinHole, LandmarkAnchoredHomogeneousPoint,
	SensorAbstract::PINHOLE, LandmarkAbstract::PNT_AH>
	PinholeAhpObservationMaker;



}}

#endif

