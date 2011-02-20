#include "jafarConfig.h"

#include "jmath/mean_shift_clustering.hpp"
#include "jmath/jblas.hpp"
#include "jmath/random.hpp"
#include "jmath/mat_file.hpp"

using namespace jafar::jmath;
using namespace jafar;
int main(int argc, char** argv) {
	jmath::matrix_file<double> mat;
	// mat.data.resize(120,3);
	// jblas::vec3 sphere_center_0;
	// sphere_center_0[0] = 0;
	// sphere_center_0[1] = 0;
	// sphere_center_0[2] = 0;
	// jblas::vec3 sphere_center_1;
	// sphere_center_1[0] = 5;
	// sphere_center_1[1] = 5;
	// sphere_center_1[2] = 5;
	// jblas::vec3 sphere_center_2;
	// sphere_center_2[0] = 10;
	// sphere_center_2[1] = 10;
	// sphere_center_2[2] = 10;
	// MultiDimUniformDistribution rng(3, -1.0, 1.0);


	// for(int i = 0; i < 40; i++) {
	// 	jblas::vec3 pt_0, pt_1, pt_2;
	// 	pt_0 = sphere_center_0 + rng.get();
	// 	pt_1 = sphere_center_1 + rng.get();
	// 	pt_2 = sphere_center_2 + rng.get();

	// 	// row(mat.data,3*i) = pt_0;
	// 	// row(mat.data,3*i+1) = pt_1;
	// 	// row(mat.data,3*i+2) = pt_2;
	// 	row(mat.data,i) = pt_0;
	// 	row(mat.data,i+40) = pt_1;
	// 	row(mat.data,i+80) = pt_2;
	// }
	// //	mat.save("pts.xyz");
	// for(int i = 0; i < 120; i++) {
	// 	if(i == 40 || i == 80)
	// 		std::cout << std::endl;
	// 	std::cout << row(mat.data,i) << std::endl;
	// }
	mat.load("data.ascii");
	jmath::mean_shift_clustering<double,3> filter(2.0,0.1,0.1,100,mean_shift_clustering<double,3>::NORMAL, 1.0);
	size_t nb_clusters = filter.run(mat.data);
	std::cout << "found " << nb_clusters << " clusters"<< std::endl;
	// std::map<size_t, jmath::mean_shift_clustering::cluster> clusters = filter.found_clusters();
	// jblas::mat M(clusters.size(), 3);
	// for(size_t s = 0; s < clusters.size();s++){
	// 	ublas::row(M, s) = clusters[s].center;
	// }
	// nb_clusters = filter.run(M);
}
