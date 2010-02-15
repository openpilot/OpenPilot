/*
 * test.cpp
 *
 *  Created on: Jan 28, 2010
 *      Author: jsola
 */

#include <iostream>
#include "jmath/jblas.hpp"
#include "jmath/ublasExtra.hpp"
using namespace std;

int main()
{
    const unsigned N = 7;
    ublas::matrix<int> M(N,N);
    ublas::vector<int> V(N);

    // initialize M and V
    std::cout << "M=" << std::endl;
    for (unsigned i = 0; i < M.size1(); ++i)
    {
        V(i) = 111*(i+1);
        for (unsigned j = 0; j < M.size2(); ++j)
        {
            M(i,j) = 10*i+j;
            std::cout.width(2);
            std::cout.fill('0');
            std::cout << M(i,j) << "  ";
        }
        std::cout << std::endl;
    }
    std::cout << "V=\n" << V << std::endl;

    // define an indirect_array
    const unsigned n = 3;
    ublas::indirect_array<> ia(n);
    ia[0] = 1;
    ia[1] = 2;
    ia[2] = 4;

    //hint: use ublas::indirect_array<>::all() to access all indices
    // use ia to extract some indices
    ublas::matrix_indirect<ublas::matrix<int> > Mindirect(M, ia,ia);
    std::cout << "Mindirect=\n" << Mindirect << std::endl;

    // or use ublas::project
    std::cout << "Mprojected=\n" << ublas::project(M, ia, ia) << std::endl;

    // similar for vectors
    ublas::vector_indirect<ublas::vector<int> > Vindirect(V,ia);
    std::cout << "Vindirect=\n" << Vindirect << std::endl;

    // and
    std::cout << "Vprojected=\n" << ublas::project(V, ia) << std::endl;
}
