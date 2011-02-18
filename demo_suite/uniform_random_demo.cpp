#include "jmath/uniform_random.hpp"
#include "jmath/jblas.hpp"

using namespace jafar;
using namespace std;
int main(int argc, char** argv) {
	jmath::uniform_generator rng(0.0,1.0, 42L);
	jblas::vec v(4);
	rng.fill_vector(v);
	for(int counter = 0; counter < v.size(); counter++)
		cout << v[counter] << endl;
}
