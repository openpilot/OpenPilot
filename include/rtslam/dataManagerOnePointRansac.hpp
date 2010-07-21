/*
 * \file dataManagerOnePointRansac.hpp
 *
 *  \date: Jun 30, 2010
 *      \author: jsola
 *      \ingroup rtslam
 */

#ifndef DATAMANAGERONEPOINTRANSAC_HPP_
#define DATAMANAGERONEPOINTRANSAC_HPP_

#include <vector>
#include <list>
#include "boost/shared_ptr.hpp"

#include "rtslam/dataManagerAbstract.hpp"
#include "rtslam/quatTools.hpp"

namespace jafar {
	namespace rtslam {

		typedef std::vector<observation_ptr_t> ObsList;
		struct RansacSet {
				observation_ptr_t obsBasePtr;
				ObsList inlierObs;
				ObsList pendingObs;
				size_t size() {
					return inlierObs.size();
				}
		};
		typedef boost::shared_ptr<RansacSet> ransac_set_ptr_t;
		typedef std::list<ransac_set_ptr_t> RansacSetList;


		// TODO extend to n-point ransac ?
		/**
		This class implements the one-point-Ransac ActiveSearch strategy
		
		@ingroup rtslam
		*/
		template<class RawSpec,class SensorSpec, class FeatureSpec, class RoiSpec, class FeatureManagerSpec, class DetectorSpec, class MatcherSpec>
		class DataManagerOnePointRansac: public DataManagerAbstract, public SpecificChildOf<SensorSpec> {

			public:
				// Define the function linkToParentSensorSpec.
				ENABLE_LINK_TO_SPECIFIC_PARENT(SensorAbstract, SensorSpec, SensorSpec, DataManagerAbstract);
				// Define the functions sensorSpec() and sensorSpecPtr().
				ENABLE_ACCESS_TO_SPECIFIC_PARENT(SensorSpec, sensorSpec);

			public: // public interface
				DataManagerOnePointRansac(const boost::shared_ptr<DetectorSpec> & _detector, const boost::shared_ptr<MatcherSpec> & _matcher, const boost::shared_ptr<FeatureManagerSpec> _featMan, int n_updates, int n_tries, int n_init):
					detector(_detector), matcher(_matcher), featMan(_featMan)
				{
					algorithmParams.n_updates = n_updates;
					algorithmParams.n_tries = n_tries;
					algorithmParams.n_init = n_init;
				}
				virtual ~DataManagerOnePointRansac() {
				}
				void process(boost::shared_ptr<RawAbstract> data);

			protected: // main data members
				boost::shared_ptr<DetectorSpec> detector;
				boost::shared_ptr<MatcherSpec> matcher;
				boost::shared_ptr<FeatureManagerSpec> featMan;
				// the list of observations sorted by information gain
				typedef map<double, observation_ptr_t> ObservationListSorted;
				ObservationListSorted obsListSorted;
				// the list of visible observations to handle
				ObsList obsVisibleList;
				unsigned remainingObsCount;
				ObsList obsBaseList;
				ObsList obsFailedList;
				RansacSetList ransacSetList;

			protected: // parameters
				struct alg_params_t {
						int n_updates; ///< maximum number of updates
						int n_tries;   ///< number of RANSAC consensus tries
						int n_init;    ///< number of feature initialization
				} algorithmParams;

			public: // getters ans setters
/*				boost::shared_ptr<FeatureManagerSpec> featureManager(void) {
					return featMan;
				}*/
// 				void setAlgorithmParams(int n_updates, int n_tries) {
// 					algorithmParams_.n_updates = n_updates;
// 					algorithmParams_.n_tries = n_tries;
// 				}
// 				alg_params_t algorithmParams() {
// 					return algorithmParams_;
// 				}

			protected: // particular processing
				void processKnownObs(boost::shared_ptr<RawSpec> rawData);
				void detectNewObs(boost::shared_ptr<RawSpec> rawData);

			protected: // helper functions
				void projectAndCollectVisibleObs();
				void getOneMatchedBaseObs(observation_ptr_t & obsBasePtr, boost::shared_ptr<RawSpec> rawData);
				observation_ptr_t selectOneRandomObs();
				vec updateMean(const observation_ptr_t & obsPtr);
				void projectFromMean(vec & exp, const observation_ptr_t & obsPtr, const vec & x);
				bool isLowInnovationInlier(const observation_ptr_t & obsPtr, const vec & exp, double lowInnTh);
				bool isExpectedInnovationInlier( observation_ptr_t & obsPtr, double highInnTh);
// 				bool match(const boost::shared_ptr<RawImage> & rawPtr, const appearance_ptr_t & targetApp, image::ConvexRoi &roi, Measurement & measure, const appearance_ptr_t & app);
				bool matchWithLowInnovation(const observation_ptr_t obsPtr, double lowInnTh);
				bool matchWithExpectedInnovation(boost::shared_ptr<RawSpec> rawData,  observation_ptr_t obsPtr);

		};

	}
}


#include "rtslam/dataManagerOnePointRansac.impl.hpp"

#endif /* DATAMANAGERONEPOINTRANSAC_HPP_ */
