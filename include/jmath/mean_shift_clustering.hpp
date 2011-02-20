#ifndef _MEAN_SHIFT_CLUSTERING_
#define _MEAN_SHIFT_CLUSTERING_

#include "jmath/jann.hpp"
#include "jmath/jblas.hpp"
#include <set>
namespace jafar {
	namespace jmath {
		/**
		 *  From <a href=http://homepages.inf.ed.ac.uk/rbf/CVonline/LOCAL_COPIES/TUZEL1/MeanShift.pdf/> :
		 * "The mean shift algorithm is a nonparametric clustering technique which does 
		 * not require prior knowledge of the number of clusters, and does not 
		 * constrain the shape of the clusters".
		 * This implementation involves gaussian or normal kernels.
		 * @ingroup jmath
		 * @TODO make it more general i.e. use functors for distance and norm
		 */
		class mean_shift_clustering {
			///inner structure to hold a cluster center along with inliers
		public:
			struct cluster {
				///cluster center
				jblas::vec3 center;
				///cluster members
				std::vector<int> inliers;
				cluster() : inliers(0) {}
				///constructor
				cluster(const jblas::vec3& _center) : center(_center), inliers(0) {}
				cluster(const jblas::vec3& _center, const std::vector<int> &_inliers) : 
					center(_center), inliers(_inliers) {}
				cluster(const jblas::vec3& _center, int* _inliers, size_t nb_inliers) :
				center(_center) 
				{
					this->add(_inliers, nb_inliers);
				}
				///add a data point in a cluster
				void add(size_t pt) {
					inliers.push_back(pt);
				}
				template <class InputIterator>
				void add(InputIterator first, InputIterator last) {
					inliers.insert(inliers.end(), first, last);
				}
				void add(int* _inliers, size_t nb_inliers) {
					for(size_t i = 0; i < nb_inliers; i++, _inliers++)
						inliers.push_back(*_inliers);
				}
			};
		private:
			///computes the center of the gaussian formed by n points at local_center
			void normal_center(const std::vector<jblas::vec3> &points, 
												 const jblas::vec3& local_center, 
												 jblas::vec3& center);
			///computes the center of uniform distribution of n points
			void uniform_center(const std::vector<jblas::vec3> &points, 
													jblas::vec3& center);
		public:
			///supported kernel types
			enum KERNEL_TYPE {NORMAL, UNIFORM};
			///constructor
			mean_shift_clustering(double _radius = 1.0,
														double _threshold = 0.1,
														double _min_distance = 0.1,
														unsigned int _max = 100,
														mean_shift_clustering::KERNEL_TYPE _kernel = mean_shift_clustering::NORMAL,
														double _variance = 1.0) :
				window_radius(_radius), convergence_threshold(_threshold),
				min_distance_between_clusters(_min_distance), max_iterations(_max),
				kernel(_kernel), gaussian_variance(_variance){}
			/**run mean shift clustering algorithm on the data set
			 * @return number of found clusters
			 */
			size_t run(const jblas::mat& data);
			///@return the found clusters
			std::map<size_t, cluster> found_clusters() const {
				return clusters;
			}
		private:
			///clusters map
			std::map<size_t, cluster> clusters;
			///search window radius
			double window_radius;
			///convergence threshold for a candidate cluster center
			double convergence_threshold;
			///minimum distance between clusters
			double min_distance_between_clusters;
			///maximum number of iterations
			unsigned int max_iterations;
			///used kernel
			KERNEL_TYPE kernel;
			///gaussian variance
			double gaussian_variance;
			
			// template <typename T>
			// T sum_2(const ublas::vector<T>& v) {
			// 	T sum = 0;
			// 	for(size_t i = 0; i < v.size(); i++)
			// 		sum+= v[i] * v[i];
			// 	return sum;
			// }
			
			// template <typename T>
			// T distance_2(const ublas::vector<T>& v1, const ublas::vector<T>& v2) {
			// 	JFR_ASSERT(v1.size() == v2.size(), 
			// 						 "mean_shift_clustering::distance_2: v1 and v2 sizes differ")
			// 		ublas::vector<T> dif = v1 -v2;
			// 	return sum_2(dif);
			// }
		};
	}
}
#endif
