#include "jafarConfig.h"

#include "jmath/mean_shift_clustering.hpp"
#include "jmath/jblas.hpp"
#include "jmath/uniform_random.hpp"

using namespace jafar::jmath;
using namespace jafar;
int main(int argc, char** argv) {
	jblas::vec3 sphere_center_0;
	sphere_center_0[0] = 0;
	sphere_center_0[1] = 0;
	sphere_center_0[2] = 0;
	jblas::vec3 sphere_center_1;
	sphere_center_1[0] = 5;
	sphere_center_1[1] = 5;
	sphere_center_1[2] = 5;
	jblas::vec3 sphere_center_2;
	sphere_center_2[0] = 10;
	sphere_center_2[1] = 10;
	sphere_center_2[2] = 10;
	jmath::uniform_generator rng(-1.0,1.0, 42L);
	jblas::mat data(120,3);
	for(int i = 0; i < 40; i++) {
		jblas::vec3 pt_0, pt_1, pt_2;
		pt_0[0] = sphere_center_0[0] + rng.run();
		pt_0[1] = sphere_center_0[1] + rng.run();
		pt_0[2] = sphere_center_0[2] + rng.run();
		pt_1[0] = sphere_center_1[0] + rng.run();
		pt_1[1] = sphere_center_1[1] + rng.run();
		pt_1[2] = sphere_center_1[2] + rng.run();
		pt_2[0] = sphere_center_2[0] + rng.run();
		pt_2[1] = sphere_center_2[1] + rng.run();
		pt_2[2] = sphere_center_2[2] + rng.run();
		row(data,i) = pt_0;
		row(data,i+1) = pt_1;
		row(data,i+2) = pt_2;
	}
	jmath::mean_shift_clustering filter(2.0,0.1,0.1,100,mean_shift_clustering::NORMAL, 1.0);
	size_t nb_clusters = filter.run(data);
	std::cout << "found " << nb_clusters << " clusters"<< std::endl;
}
