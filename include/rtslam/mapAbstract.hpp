/*
 * \file mapAbstract.hpp
 *
 * \date Feb 9, 2010
 *     \author jsola@laas.fr
 *
 * Header file for abstract maps
 *
 * \ingroup rtslam
 */

#ifndef MAPABSTRACT_HPP_
#define MAPABSTRACT_HPP_

#include "jmath/jblas.hpp"
#include "rtslam/rtSlam.hpp"

#include "rtslam/gaussian.hpp"
#include "rtslam/kalmanFilter.hpp"
#include "rtslam/parents.hpp"
#include "rtslam/worldAbstract.hpp"

namespace jafar {
	/**
	 * Namespace rtslam for real-time slam module.
	 * \ingroup rtslam
	 */
	namespace rtslam {
		using namespace std;


		// some forward declarations.
		class RobotAbstract;
		class LandmarkAbstract;
		class MapManagerAbstract;
	  class ObservationFactory;

		/** Base class for all map types defined in the module rtslam.
		 *
		 * \author jsola@laas.fr
		 * \ingroup rtslam
		 */
		class MapAbstract: public ObjectAbstract, public ChildOf<WorldAbstract>, 
			public boost::enable_shared_from_this<MapAbstract>,
			public ParentOf<RobotAbstract> , public ParentOf<MapManagerAbstract> {

			ENABLE_LINK_TO_PARENT(WorldAbstract,World,MapAbstract);
			ENABLE_ACCESS_TO_PARENT(WorldAbstract,world);
			
			ENABLE_ACCESS_TO_CHILDREN(RobotAbstract,Robot,robot);
		  ENABLE_ACCESS_TO_CHILDREN(MapManagerAbstract,MapManager,mapManager);

		public:
				/**
				 * Print all MAP data.
				 *
				 * It traverses the map tree in the following way:
				 * - robots
				 *   - sensors in robot
				 * - landmarks
				 *   - observations of landmark from each sensor
				 */
				friend std::ostream& operator <<(std::ostream & s, const jafar::rtslam::MapAbstract & map);

			public:


				/**
				 * Constructor
				 */
				MapAbstract(size_t _max_size);
				MapAbstract(const ekfInd_ptr_t & ekfPtr);

				/**
				 * Mandatory virtual destructor - Map is used as-is, non-abstract by now
				 */
				virtual ~MapAbstract() {
				}

				virtual std::string categoryName() {
					return "MAP";
				}

				Gaussian state;

				/**
				 * EKF engine
				 */
				ekfInd_ptr_t filterPtr;
//				ExtendedKalmanFilterIndirect filter;

				/**
				 * Size things and map usage management
				 */
				size_t max_size;
				size_t current_size;
				jblas::vecb used_states;

				/**
				 * Map's indirect array is a function by now.
				 * \return the indirect array of all used states in the map.
				 * \todo: see how to avoid constructing this each time
				 */
				inline jblas::ind_array ia_used_states() {
					jblas::ind_array res;
					res = jmath::ublasExtra::ia_set(used_states);
					return res;
				}


				jblas::vec & x();
				jblas::sym_mat & P();
				double & x(size_t i);
				double & P(size_t i, size_t j);

				/**
				 * Query about available free space.
				 * \return the number of unused states
				 */
				inline std::size_t unusedStates() const {
					return max_size - current_size;
				}


				/**
				 * Query about available free space
				 * \return true if at least N non-used states
				 */
				inline bool unusedStates(const size_t N) const {
					return (unusedStates() >= N);
				}

				/**
				 * Obtain free Map space of a given size.
				 * The free space in \a used_states and the current size \a current_size are modified accordingly.
				 * Ig not enough space is available, the returned indirect array is of null size.
				 * \param _size the requested free space size.
				 * \return the resulting free space.
				 */
				jblas::ind_array reserveStates(const std::size_t _size);

 		                /**
				 * From the already-reserved space _ia, keep the first N states in a new index that is returned,
				 * and stored all the other states in _icomp for future liberation.
				 */
		                jblas::ind_array convertStates(const jblas::ind_array & _ia,const std::size_t N,jblas::ind_array & _icomp);

				/**
				 * Liberate the space indicated.
				 * The free space in \a used_states and the current size \a current_size are modified accordingly.
				 * \param _ia the space to liberate.
				 */
				void liberateStates(const jblas::ind_array & _ia);

				/**
				 * Add observations to landmark.
				 * This requires traversing all the map tree and this is why this function goes here.
				 * \param lmkPtr the pointer to the landmark to associate observations to.
				 */
				void completeObservationsInGraph(const sensor_ptr_t & senPtr, const landmark_ptr_t & lmkPtr);

				void clear();
				void fillSeq();
				void fillDiag();
				void fillDiagSeq();
				void fillRndm();

			private:


		};

	}
}

#endif /* MAPABSTRACT_HPP_ */

