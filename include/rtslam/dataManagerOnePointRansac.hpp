/*
 * \file dataManagerOnePointRansac.hpp
 *
 *  \date: Jun 30, 2010
 *      \author: jsola
 *      \ingroup rtslam
 */

#ifndef DATAMANAGERONEPOINTRANSAC_HPP_
#define DATAMANAGERONEPOINTRANSAC_HPP_

#include "rtslam/rtSlam.hpp"
#include "rtslam/parents.hpp"

namespace jafar {
	namespace rtslam {

		template<class RawSpec, class SensorSpec, class Detector, class Matcher>
		class DataManagerOnePointRansac: public DataManagerAbstract,
		    public SpecificChildOf<SensorSpec> {

			public:
				// Define the function linkToParentSensorSpec.
			ENABLE_LINK_TO_SPECIFIC_PARENT(SensorAbstract,SensorSpec,
					SensorSpec,DataManagerAbstract)
				;
				// Define the functions sensorSpec() and sensorSpecPtr().
			ENABLE_ACCESS_TO_SPECIFIC_PARENT(SensorSpec,sensorSpec)
				;

			public:
				virtual ~DataManagerOnePointRansac() {
				}
				void process(boost::shared_ptr<RawAbstract> data) {
				}

			protected:
				boost::shared_ptr<Detector> detector;
				boost::shared_ptr<Matcher> matcher;
				boost::shared_ptr<ActiveSearchGrid> asGrid;
				// the list of visible observations to handle
				typedef vector<observation_ptr_t> ObservationListVisible;
				ObservationListVisible obsListVisible;
				struct RansacSet {
						observation_ptr_t obsBasePtr;
						list<observation_ptr_t> inlierObs;
						list<observation_ptr_t> pendingObs;
				};
				list<RansacSet> ransacSetList;

			protected:
				jblas::veci tries;

			protected:
				struct detector_params_t {
						int patchSize; ///<       descriptor patch size
						double measStd; ///<       measurement noise std deviation
						double measVar; ///<       measurement noise variance
				} detectorParams_;
				struct matcher_params_t {
						int patchSize;
						double regionPix; ///<     search region radius for first RANSAC consensus
						double regionSigma; ///<   search region n_sigma for expectation-based search
						double threshold; ///<     matching threshold
						double mahalanobisTh; ///< Mahalanobis distance for outlier rejection <-- TODO: remove?
						double measStd; ///<       measurement noise std deviation
						double measVar; ///<       measurement noise variance
				} matcherParams_;
				struct alg_params_t {
						int n_updates; ///<        maximum number of updates
						int n_tries; ///<          number of RANSAC consensus tries
				} algorithmParams_;
			public:
				void setDetector(const boost::shared_ptr<Detector> & _detector,
				    int patchSize, double _measStd) {
					detector = _detector;
					detectorParams_.patchSize = patchSize;
					detectorParams_.measStd = _measStd;
					detectorParams_.measVar = _measStd * _measStd;
				}
				detector_params_t detectorParams() {
					return detectorParams_;
				}
				void setMatcher(boost::shared_ptr<Matcher> & _matcher, int patchSize,
				    double nPix, double nSigma, double threshold, double mahalanobisTh,
				    double _measStd) {
					matcher = _matcher;
					matcherParams_.patchSize = patchSize;
					matcherParams_.regionPix = nPix;
					matcherParams_.regionSigma = nSigma;
					matcherParams_.threshold = threshold;
					matcherParams_.mahalanobisTh = mahalanobisTh;
					matcherParams_.measStd = _measStd;
					matcherParams_.measVar = _measStd * _measStd;
				}
				matcher_params_t matcherParams() {
					return matcherParams_;
				}
				void setActiveSearchGrid(boost::shared_ptr<ActiveSearchGrid> arg) {
					asGrid = arg;
				}
				boost::shared_ptr<ActiveSearchGrid> activeSearchGrid(void) {
					return asGrid;
				}
				void setAlgorithmParams(int n_updates, int n_tries) {
					algorithmParams_.n_updates = n_updates;
					algorithmParams_.n_tries = n_tries;
				}
				alg_params_t algorithmParams() {
					return algorithmParams_;
				}

			protected:
				void processKnownObs();
				void detectNewObs();

		};

	}
}

#include "rtslam/dataManagerOnePointRansac.impl.hpp"

#endif /* DATAMANAGERONEPOINTRANSAC_HPP_ */
