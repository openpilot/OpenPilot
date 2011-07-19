/**
 * \file simuData.hpp
 *
 * All the data objects that will be used by real slam when doing simulation.
 * They must respect the abstract slam interface.
 * This is NOT part of the ad-hoc simulator, but required for any estimator without realistic raw simulation.
 *
 * \date 24/07/2010
 * \author croussil
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
			jblas::vec4 exp;
			jblas::vec4 obs;
			size_t id;
		public:
			AppearanceSimu() {}
			AppearanceSimu(LandmarkAbstract::geometry_t type, size_t id): type(type), id(id) {}
			AppearanceAbstract* clone() { return new AppearanceSimu(*this); }
			
			jblas::vec4 realObs()
			{
				return obs;
			}
	};
	
	class FeatureSimu: public rtslam::FeatureAbstract
	{
		public:
			FeatureSimu() {}
			FeatureSimu(jblas::vec meas, LandmarkAbstract::geometry_t type, size_t id):
				FeatureAbstract(meas.size(), appearance_ptr_t(new AppearanceSimu(type, id))) { measurement.x() = meas; }
			FeatureSimu(int size): FeatureAbstract(size, appearance_ptr_t(new AppearanceSimu())) {}
	};
	
	typedef boost::shared_ptr<FeatureSimu> featuresimu_ptr_t;
	
	class DescriptorSimu: public rtslam::DescriptorAbstract
	{
		private:
			boost::shared_ptr<AppearanceSimu> appPtr;
			
		public:
			bool addObservation(const observation_ptr_t & obsPtr)
			{
				if (obsPtr->events.updated)
				{
					appPtr = SPTR_CAST<AppearanceSimu>(obsPtr->observedAppearance);
					return true;
				} else
					return false;
			}
			bool predictAppearance(const observation_ptr_t & obsPtr)
			{
				boost::shared_ptr<AppearanceSimu> app_dst = SPTR_CAST<AppearanceSimu>(obsPtr->predictedAppearance);
				boost::shared_ptr<AppearanceSimu> app_src = appPtr;
				*app_dst = *app_src;

				if(app_dst->type == LandmarkAbstract::LINE)
				{
					app_dst->exp = obsPtr->expectation.x();
				}

				return true;
			}
			bool isPredictionValid(const observation_ptr_t & obsPtr) { return true; }
			std::ostream& print(std::ostream& os) const { os << appPtr->id; return os; }
	};
	
	
	class RawSimu: public rtslam::RawAbstract {
		public:
			typedef std::map<size_t,featuresimu_ptr_t> ObsList;
			ObsList obs;
			virtual RawAbstract* clone() { RawSimu *raw = new RawSimu(); *raw = *this; return raw; }
	};
	
	
}}}



#endif

