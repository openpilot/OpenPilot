#include "jafarConfig.h"

#include "jmath/mean_shift_clustering.hpp"
#include "jmath/jblas.hpp"
#include "jmath/random.hpp"

using namespace jafar::jmath;
using namespace jafar;
int main(int argc, char** argv) {
#ifdef HAVE_FLANN
  jblas::mat data(120,3);
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
	MultiDimUniformDistribution rng(3, -1.0, 1.0);


	for(int i = 0; i < 40; i++) {
		jblas::vec3 pt_0, pt_1, pt_2;
		pt_0 = sphere_center_0 + rng.get();
		pt_1 = sphere_center_1 + rng.get();
		pt_2 = sphere_center_2 + rng.get();
		row(data,i) = pt_0;
		row(data,i+40) = pt_1;
		row(data,i+80) = pt_2;
	}
	jmath::mean_shift_clustering<double,3> filter(2.0,0.1,0.1,100,mean_shift_clustering<double,3>::NORMAL, 1.0);
	size_t nb_clusters = filter.run(data);
	std::cout << "found " << nb_clusters << " clusters"<< std::endl;
#endif // HAVE_FLANN
}

