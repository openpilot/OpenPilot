/**
 * \file simuData.hpp
 *
 * All the data objects that will be used by real slam when doing simulation.
 * They must respect the abstract slam interface.
 * This is NOT part of the ad-hoc simulator, but required for any estimator without realistic raw simulation.
 *
 * \author croussil@laas.fr
 * \date 24/07/2010
 *
 * \ingroup rtslam
 */

#ifndef SIMUDATA_HPP_
#define SIMUDATA_HPP_

namespace jafar {
namespace rtslam {
namespace simu {
	
	
	
	class AppearanceSimu: public rtslam::AppearanceAbstract
	{
		public:
			LandmarkAbstract::geometry_t type;
			size_t id;
		public:
			AppearanceSimu() {}
			AppearanceSimu(LandmarkAbstract::geometry_t type, size_t id): type(type), id(id) {}
			AppearanceAbstract* clone() { return new AppearanceSimu(*this); }
			
		
	};
	
	class FeatureSimu: public rtslam::FeatureAbstract
	{
		public:
			FeatureSimu() {}
			FeatureSimu(jblas::vec meas, LandmarkAbstract::geometry_t type, size_t id):
				FeatureAbstract(meas.size(), appearance_ptr_t(new AppearanceSimu(type, id))) { measurement.x() = meas; }
			FeatureSimu(int size): FeatureAbstract(size, appearance_ptr_t(new AppearanceSimu())) {}
	};
	
	class DescriptorSimu: public rtslam::DescriptorAbstract
	{
		private:
			boost::shared_ptr<FeatureSimu> featPtr;
			
		public:
			DescriptorSimu(const boost::shared_ptr<FeatureSimu> & featPtr): featPtr(featPtr) {}
			
			bool predictAppearance(observation_ptr_t & obsPtr)
			{
				boost::shared_ptr<AppearanceSimu> app_dst = SPTR_CAST<AppearanceSimu>(obsPtr->predictedAppearance);
				boost::shared_ptr<AppearanceSimu> app_src = SPTR_CAST<AppearanceSimu>(featPtr->appearancePtr);
				*app_dst = *app_src;
				return true;
			}
	};
	
	
	class RawSimu: public rtslam::RawAbstract {
		public:
			typedef std::map<size_t,FeatureSimu> ObsList;
			ObsList obs;
	};
	
	
}}}



#endif

