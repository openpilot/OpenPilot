#include <boost/test/auto_unit_test.hpp>

#include "kernel/jafarDebug.hpp"
#include "kernel/jafarTestMacro.hpp"
#include "jafarConfig.h"
#include "jmath/pca.hpp"

using namespace jafar;

BOOST_AUTO_TEST_CASE( test_pca )
{
	std::cout << "== test_pca ===========================" << std::endl 
	          << "=======================================" << std::endl;
#if defined(HAVE_BOOST_SANDBOX) && defined(HAVE_LAPACK)
  ublas::matrix<float> data(3,20);
  std::cout << data.size1()<<"x"<<data.size2()<< std::endl;
  data(0,0) = 100;   data(1,0) = 8;    data(2,0) = 5;
  data(0,1) = 228;   data(1,1) = 21;   data(2,1) = 2;
  data(0,2) = 341;   data(1,2) = 31;   data(2,2) = 10;
  data(0,3) = 472;   data(1,3) = 40;   data(2,3) = 15;
  data(0,4) = 578;   data(1,4) = 48;   data(2,4) = 3;
  data(0,5) = 699;   data(1,5) = 60;   data(2,5) = 12;
  data(0,6) = 807;   data(1,6) = 71;   data(2,6) = 14;
  data(0,7) = 929;   data(1,7) = 79;   data(2,7) = 16;
  data(0,8) = 1040;  data(1,8) = 92;   data(2,8) = 18;
  data(0,9) = 1160;  data(1,9) = 101;  data(2,9) = 38;
  data(0,10) = 1262; data(1,10) = 109; data(2,10) = 28;
  data(0,11) = 1376; data(1,11) = 121; data(2,11) = 32;
  data(0,12) = 1499; data(1,12) = 128; data(2,12) = 35;
  data(0,13) = 1620; data(1,13) = 143; data(2,13) = 28;
  data(0,14) = 1722; data(1,14) = 150; data(2,14) = 30;
  data(0,15) = 1833; data(1,15) = 159; data(2,15) = 15;
  data(0,16) = 1948; data(1,16) = 172; data(2,16) = 12;
  data(0,17) = 2077; data(1,17) = 181; data(2,17) = 33;
  data(0,18) = 2282; data(1,18) = 190; data(2,18) = 23;
  data(0,19) = 2999; data(1,19) = 202; data(2,19) = 29;
  PCA_T<float> pca(data);
  std::cout << "initial\t\tprojected\t\treconstructed"<<std::endl;
  for(size_t i = 0; i < 20; i++)
  {
    ublas::matrix_column<ublas::matrix<float> > p(data,i);
    ublas::vector< float > pp(3);
    pp = pca.project(p);
    std::cout << "<"<<p[0]<<","<<p[1]<<","<<p[2]<<">";
    std::cout << "\t\t<"<<pp[0]<<","<<pp[1]<<","<<pp[2]<<">";
    ublas::vector< float > pr(3);
    pr = pca.reconstruct(pp);
    std::cout << "\t\t"<<pr[0]<<","<<pr[1]<<","<<pr[2]<<">"<<std::endl;
    JFR_CHECK_VEC_EQUAL(p,pr);
  }
#else
  std::cout << "pca_demo requires lapack and boost_sandbox" << std::endl;
#endif
}
