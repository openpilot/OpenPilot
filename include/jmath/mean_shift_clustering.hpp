#ifndef _MEAN_SHIFT_CLUSTERING_
#define _MEAN_SHIFT_CLUSTERING_

#include "jafarConfig.h"
#ifdef HAVE_FLANN

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
		template<typename NUMTYPE, size_t D>
		class mean_shift_clustering {
			///inner structure to hold a cluster center along with inliers
		public:
			typedef ublas::bounded_vector<NUMTYPE, D> vector_type;
			struct cluster {
				///cluster center
				vector_type center;
				///cluster members
				std::vector<int> inliers;
				cluster() : inliers(0) {}
				///constructor
				cluster(const vector_type& _center) : center(_center), inliers(0) {}
				cluster(const vector_type& _center, const std::vector<int> &_inliers) : 
					center(_center), inliers(_inliers) {}
				cluster(const vector_type& _center, int* _inliers, size_t nb_inliers) :
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
			void normal_center(const std::vector<vector_type> &points, 
												 const vector_type& mean, 
												 vector_type& center)
			{
				JFR_DEBUG("nb of points "<<points.size());
					center = ublas::zero_vector<NUMTYPE>(D);
				for(typename std::vector<vector_type>::const_iterator pt = points.begin();
						pt != points.end();
						++pt) {
					vector_type difference = *pt - mean;
					NUMTYPE n = exp(-sum_2<NUMTYPE>(difference)/(2.0 * gaussian_variance));
					// NUMTYPE n = exp(-sum_2(difference)/(2.0 * gaussian_variance));
					//		NUMTYPE n = exp(-distance_2(*pt,mean)/(2.0 * gaussian_variance));		
					difference*= n;
					center+= difference;
				}
				center/=NUMTYPE(points.size());
				center+=mean;
			}
			///computes the center of uniform distribution of n points
			void uniform_center(const std::vector<vector_type> &points, 
													vector_type& center)
			{
				center = ublas::zero_vector<NUMTYPE>(D);
				for(typename std::vector<vector_type>::const_iterator pt = points.begin();
						pt != points.end();
						++pt) {
					center+= *pt;
				}
				center/=NUMTYPE(points.size());
			}

		public:
			///supported kernel types
			enum KERNEL_TYPE {NORMAL, UNIFORM};
			///constructor
			mean_shift_clustering(NUMTYPE _radius = 1.0,
														NUMTYPE _threshold = 0.1,
														NUMTYPE _min_distance = 0.1,
														unsigned int _max = 100,
														mean_shift_clustering::KERNEL_TYPE _kernel = mean_shift_clustering::NORMAL,
														NUMTYPE _variance = 1.0) :
				window_radius(_radius), convergence_threshold(_threshold),
				min_distance_between_clusters(_min_distance), max_iterations(_max),
				kernel(_kernel), gaussian_variance(_variance) {}
			/**run mean shift clustering algorithm on the data set
			 * @return number of found clusters
			 */
			size_t run(const ublas::matrix<NUMTYPE>& data) 
			{
				using namespace jafar::jmath;
				using namespace std;
 
				JFR_ASSERT(data.size1() > 1, "need at least two points");
					JFR_ASSERT(data.size2() == D, "points must be of dimension D");
					jann::KD_tree_index< flann::L2<NUMTYPE> > points_tree(data, 4);
				points_tree.build();
				vector_type current_center;
				vector_type new_center;
				for(size_t i = 0; i < data.size1(); i++) {
					current_center = row(data, i);
					unsigned int iter = 0;
					vector_type this_difference;
					do {
						ublas::vector<int> this_indices;
						ublas::vector<NUMTYPE> this_distances;
						int nb_neighbours = points_tree.radius_search(current_center, this_indices, this_distances, window_radius, jann::search_params(128, 0, true));
						//found some neighbours
						JFR_DEBUG("neighbours found: " << nb_neighbours);
							if(nb_neighbours > 0) {
								this_indices.resize(nb_neighbours, true);
								std::vector<vector_type> this_neighbours;
								this_neighbours.reserve(nb_neighbours);
								//fill neighbours
								for(ublas::vector<int>::const_iterator index = this_indices.begin();
										index != this_indices.end();
										++index) {
									this_neighbours.push_back(row(data, *index));
								}
								//compute new center
								switch(kernel) {
								case NORMAL : 
									normal_center(this_neighbours, current_center, new_center);
									break;
								case UNIFORM :
									uniform_center(this_neighbours, new_center);
									break;
								default :
									JFR_RUN_TIME("don't know about this kernel")
										break;
								}
								this_difference = new_center - current_center;
								current_center = new_center;
								iter++;
							}
					}while((iter < max_iterations) && (ublas::norm_2(this_difference) > convergence_threshold));
					//if the clusters are empty add the found center as a cluster center
					if(clusters.size() == 0) {
						JFR_DEBUG("added first cluster");
							clusters[0] = cluster(current_center);
						//				clusters[0] = cluster(current_center, &this_indices[0], this_indices.size());
						continue;
					}  else {
						//check validity of found cluster center
						bool found = false;
						typename std::map<size_t, cluster>::iterator cit;
						for(cit = clusters.begin(); cit != clusters.end(); ++cit) {
							//							JFR_DEBUG("dist_2 " << distance_2<NUMTYPE>(current_center, cit->second.center))
							if(distance_2<NUMTYPE>(current_center, cit->second.center) < min_distance_between_clusters){
								found = true;
								break;
							}
						}
						if(!found)
							//				clusters[clusters.size()] = cluster(current_center, &this_indices[0], this_indices.size());
							clusters[clusters.size()] = cluster(current_center);
						// else 
						// 	cit->second.add(&this_indices[0], this_indices.size());
					}
				}//end of find clusters
				//prune the data
				ublas::matrix<NUMTYPE> clusters_centers(clusters.size(),D);
				for(typename std::map<size_t, cluster>::const_iterator cit = clusters.begin();
						cit != clusters.end();
						++cit) {
					row(clusters_centers, cit->first) = cit->second.center;
				}
				jann::KD_tree_index< flann::L2<NUMTYPE> > clusters_tree(clusters_centers, 1);
				clusters_tree.build();
				for(size_t i = 0; i < data.size1(); i++) {
					ublas::vector<int> index; index.resize(1);
					ublas::vector<NUMTYPE> dist; dist.resize(1);
					clusters_tree.knn_search(row(data,i), index, dist, 1, jann::search_params(-1));
					if(index[0] != -1) {
						//						JFR_DEBUG("Assigned pt "<<i<<" to cluster "<<index[0])
							clusters[index[0]].add(i);
					}
				}
				return clusters.size();
			}

			///@return the found clusters
			std::map<size_t, cluster> found_clusters() const {
				return clusters;
			}
		private:
			///clusters map
			std::map<size_t, cluster> clusters;
			///search window radius
			NUMTYPE window_radius;
			///convergence threshold for a candidate cluster center
			NUMTYPE convergence_threshold;
			///minimum distance between clusters
			NUMTYPE min_distance_between_clusters;
			///maximum number of iterations
			unsigned int max_iterations;
			///used kernel
			KERNEL_TYPE kernel;
			///gaussian variance
			NUMTYPE gaussian_variance;

			/**  
			 * @param vector of NUMTYPE v
			 * @return the sum of the squared elements sum_2 = sum v[i]²
			 */			
			template <typename T>
			T sum_2(const ublas::vector<T>& v) {
				T sum = 0;
				for(size_t i = 0; i < v.size(); i++)
					sum+= v[i] * v[i];
				return sum;
			}
			
			/**  
			 * @param 2 vectors of NUMTYPE with same size v1 and v2
			 * @return the squared distance of v1 and v2 distance_2 = sum_2 (v1 - v2)
			 */			
			template <typename T>
			T distance_2(const ublas::vector<T>& v1, const ublas::vector<T>& v2) {
				JFR_ASSERT(v1.size() == v2.size(), 
									 "mean_shift_clustering::distance_2: v1 and v2 sizes differ");
					ublas::vector<T> dif = v1 -v2;
				return sum_2(dif);
			}
		};
	}
}
#endif // HAVE_FLANN
#endif
