////////////////////////////////////////////////////////////////////
// The car files are courtesy of Alex Segal found in gicp package //
////////////////////////////////////////////////////////////////////
#include "jmath/mat_file.hpp"
#include "jmath/jblas.hpp"
using namespace jafar;
using namespace std;

int main(int argc, char** argv) {
	jmath::matrix_file<double> mat;
	mat.load("car0.ascii");
	for (unsigned int r = 0; r < mat.data.size1(); r++) {
		for (unsigned int c = 0; c < mat.data.size2(); c++)
			cout << mat.data(r,c)<< " ";
		cout << endl;
	}
	
	mat.save("mat.data");

	jblas::mat M(3,3);
	for (unsigned int r = 0; r < 3; r++)
		for (unsigned int c = 0; c < 3; c++)
			M(r,c) = r*10 + c;
	jmath::matrix_file<double> saved_mat(M);
	saved_mat.save("M.data");
}
