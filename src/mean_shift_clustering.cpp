#include "jmath/mean_shift_clustering.hpp"

using namespace jafar::jmath;
using namespace std;

void mean_shift_clustering::normal_center(const std::vector<jblas::vec3> &points, 
																					const jblas::vec3& local_center, 
																					jblas::vec3& center)
{
	center = jblas::zero_vec(3);
	for(std::vector<jblas::vec3>::const_iterator pt = points.begin();
			pt != points.end();
			++pt) {
		jblas::vec3 difference = *pt - local_center;
		double n = exp(-sum_2<double>(difference)/(2.0 * gaussian_variance));
		difference*= n;
		center+= difference;
	}
	center/=double(points.size());
	center+=local_center;
}


void mean_shift_clustering::uniform_center(const std::vector<jblas::vec3> &points, 
																					 jblas::vec3& center)
{
	center = jblas::zero_vec(3);
	for(std::vector<jblas::vec3>::const_iterator pt = points.begin();
			pt != points.end();
			++pt) {
		center+= *pt;
	}
	center/=double(points.size());
}

size_t mean_shift_clustering::run(const jblas::mat& data) {
	JFR_ASSERT(data.size1() > 1, "need at least two points")
		JFR_ASSERT(data.size2() == 3, "points must be of dimension 3")
		//	clusters.reserve(data.size1());
		jann::KD_tree_index< flann::L2<double> > points_tree(data, 4);
	points_tree.build();
	jblas::vec3 current_center;
	jblas::vec3 new_center;
	ublas::vector<int> this_indices;
	ublas::vector<double> this_distances;
	for(size_t i = 0; i < data.size1(); i++) {
		jblas::vec3 this_point = row(data, i);
		unsigned int iter = 0;
		jblas::vec3 this_difference;
		do {
			int nb_neighbours = points_tree.radius_search(this_point, this_indices, this_distances, window_radius, jann::search_params(32, 0, true));
			//found some neighbours
			JFR_DEBUG("neighbours found: " << nb_neighbours)
			if(nb_neighbours > 0) {
				std::vector<jblas::vec3> this_neighbours;
				this_neighbours.resize(nb_neighbours);
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
					JFR_RUN_TIME("don't know about distance function for this kernel")
						break;
				}
				this_difference = new_center - current_center;
				current_center = new_center;
				iter++;
			}
		}while((iter < max_iterations) && (ublas::norm_2(this_difference) > convergence_threshold));
		//if the clusters are empty add the found center as a cluster center
		if(clusters.size() == 0) {
			JFR_DEBUG("added first cluster")
			clusters[0] = cluster(current_center);
			continue;
		}
		//check validity of found cluster center
		bool found = false;
		for(std::map<size_t, cluster>::const_iterator cit = clusters.begin(); 
				cit != clusters.end(); ++cit) {
			if(distance_2<double>(current_center, cit->second.center) < min_distance_between_clusters){
				found = true;
				break;
			}
		}
		if(!found)
			clusters[clusters.size()] = cluster(current_center);
	}//end of find clusters
	//prune the data
	jblas::mat clusters_centers(clusters.size(),3);
	for(std::map<size_t, cluster>::const_iterator cit = clusters.begin();
			cit != clusters.end();
			++cit) {
		row(clusters_centers, cit->first) = cit->second.center;
	}
	jann::KD_tree_index< flann::L2<double> > clusters_tree(clusters_centers, 1);
	clusters_tree.build();
	for(size_t i = 0; i < data.size1(); i++) {
		ublas::vector<int> index; index.resize(1);
		ublas::vector<double> dist; dist.resize(1);
		clusters_tree.knn_search(row(data,i), index, dist, 1, jann::search_params(64));
		if(index[0] != -1) {
			JFR_DEBUG("Assigned pt "<<i<<" to cluster "<<index[0])
			clusters[index[0]].add(i);
		}
	}
	return clusters.size();
}
