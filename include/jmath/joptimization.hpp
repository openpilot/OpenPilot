/* $Id$ */
/** \file joptimization.hpp
 *
 * This is the standard header defined in jafar to use non linear optimaztion methods
 * Code spoofed from Cyrille Berger (cberger@cberger.net)
 *
 * \ingroup jmath
 */

#ifndef JMATH_JOPTIMIZATION_HPP
#define JMATH_JOPTIMIZATION_HPP

#ifdef USE_JMATH_SERIALIZATION
#include "jmath/serialize_vector.hpp"
#include "jmath/serialize_matrix.hpp"
#include "jmath/serialize_symmetric.hpp"
#include "jmath/serialize_banded.hpp"
#else
#include "boost/numeric/ublas/vector.hpp"
#include "boost/numeric/ublas/matrix.hpp"
#include "boost/numeric/ublas/symmetric.hpp"
#include "boost/numeric/ublas/banded.hpp"
#endif

#include "boost/numeric/ublas/io.hpp"

#include "boost/numeric/ublas/matrix_proxy.hpp"
#include "boost/numeric/ublas/vector_proxy.hpp"

#include "jmath/ublasCompatibility.hpp"

#include "boost/numeric/bindings/lapack/gesv.hpp"
#include "boost/numeric/bindings/traits/ublas_matrix.hpp"
#include "boost/numeric/bindings/traits/ublas_vector.hpp"

/** \addtogroup jmath */
/*@{*/
/// shortcut for ublas namespace
namespace ublas = boost::numeric::ublas;
/// shortcut for lapack namespace
namespace lapack = boost::numeric::bindings::lapack;

// Declaration
namespace joptimization {
  namespace methods {
    /**
     * Use this class as a template parameter for the gaussNewton function if you want to use
     * the Gauss-Newton method which uses only the jacobian.
     */
    template< class _TFunction_, typename _TType_ >
    struct GaussNewton {
      typedef _TType_ Type;
      typedef _TFunction_ Function;
      static void computeM(const ublas::matrix<_TType_>& jacobian, 
                           const _TFunction_* f, 
                           ublas::vector<_TType_>& parameters, 
                           ublas::vector<_TType_>& values, 
                           ublas::matrix<_TType_>& M);
    };

    /**
     * Use this class as a template parameter for the gaussNewton function if you want to use
     * the Newton method which uses the Hessian.
     * You need to define ublas::matrix\<double\> hessian(const ublas::vector\<double\>& x, int n) const in _TFunction if you want to use that method.
     */
    template< class _TFunction_, typename _TType_ >
    struct Newton {
      typedef _TType_ Type;
      typedef _TFunction_ Function;
      static void computeM(const ublas::matrix<_TType_>& jacobian, 
                           const _TFunction_* f, 
                           ublas::vector<_TType_>& parameters, 
                           ublas::vector<_TType_>& values, 
                           ublas::matrix<_TType_>& M);
    };
  }

  /**
   * This namespace contains function to call some of the classical optimization algorithm for
   * estimating the parameter p and minimizing sum( f_i(p)2 ).
   * 
   * The class _TFunction_ must at least contains the following functions:
   *  - ublas::vector\<double\> values(const ublas::vector\<double\>& x) const
   * which return the list of values of each function f_i
   *  - ublas::matrix\<double\> jacobian(const ublas::vector\<double\>& x) const
   * which return the jacobian
   * - int count() const which return the number of functions f_i
   * 
   * And optionnally when using the gaussNewton algorithm witht the Newton method:
   *  - ublas::matrix\<double\> hessian(const ublas::vector\<double\>& x, int n) const
   * which return the hessian
   * 
   * Example:
   * @code
   *   // Optimization of 2 functions depending on (a,b) :
   *  // a*a+ 2*b - 5
   *  // -a + b + 1
   *  class Function {
   *    public:
   *    ublas::vector\<double\> values(const ublas::vector\<double\>& x) const
   *    {
   *      ublas::vector\<double\> v(2);
   *      v[0] = x[0]*x[0] + 2.0 * x[1] - 5.0;
   *      v[1] = -x[0] + x[1] + 1.0;
   *      return v;
   *    }
   *    ublas::matrix\<double\> jacobian(const ublas::vector\<double\>& x) const
   *    {
   *      ublas::matrix_row\< jblas::wsvector\<double\> \> jt(2, 2);
   *      jt(0,0) = 2.0 * x[0];
   *      jt(0,1) = 2.0;
   *      jt(1,0) = -1.0;
   *      jt(1,1) = 1.0;
   *      ublas::matrix\<double\> jr(2,2);
   *      jr = jt;
   *      return jr;
   *    }
   *    ublas::matrix\<double\> hessian(const ublas::vector\<double\>& x, int n) const
   *    {
   *      ublas::matrix\<double\> ht(2, 2);
   *      if(n == 0)
   *      {
   *        ht(0,0) = 2.0;
   *        ht(0,1) = 0.0;
   *        ht(1,0) = 0.0;
   *        ht(1,1) = 0.0;
   *      } else if(n == 1)
   *      {
   *        ht(0,0) = 0.0;
   *        ht(0,1) = 0.0;
   *        ht(1,0) = 0.0;
   *        ht(1,1) = 0.0;
   *      }
   *      ublas::matrix\<double\> hr(2,2);
   *      hr = ht;
   *      return hr;
   *    }
   *    int count() const {
   *      return 2;
   *    }
   *  };
   * 
   *    Function f;
   *    ublas::vector\<double\> v(2);
   *    v[0] = 10.0;
   *    v[1] = 10.0;
   *    std::cout \<\< "Gauss-Newton:" \<\< std::endl;
   *    std::cout \<\< "Remain = " \<\< joptimization::algorithms::gaussNewton\< joptimization::methods::GaussNewton\< Function, double\> \>(&f, v, 100, 1e-12) \<\< std::endl;
   *    std::cout \<\< v \<\< f.values(v) \<\< std::endl;
   *    v[0] = 10.0;
   *    v[1] = 10.0;
   *    std::cout \<\< "Newton:" \<\< std::endl;
   *    std::cout \<\< "Remain = " \<\< joptimization::algorithms::gaussNewton\< joptimization::methods::Newton\< Function, double\> \>(&f, v, 100, 1e-12) \<\< std::endl;
   *    std::cout \<\< v \<\< f.values(v) \<\< std::endl;
   *    v[0] = 1.0;
   *    v[1] = 1.0;
   *    std::cout \<\< "Gradient descent:" \<\< std::endl;
   *    std::cout \<\< "Remain = " \<\< joptimization::algorithms::gradientDescent\< Function, double\>(&f, v, 100, 1e-12, 1e-2) \<\< std::endl;
   *    std::cout \<\< v \<\< f.values(v) \<\< std::endl;
   *    std::cout \<\< "Levenberg-Marquardt:" \<\< std::endl;
   *    v[0] = 10.0;
   *    v[0] = 10.0;
   *    std::cout \<\< "Remain = " \<\< joptimization::algorithms::levenbergMarquardt\< Function, double\>(&f, v, 100, 1e-12, 0.01, 10.0) \<\< std::endl;
   *    std::cout \<\< v \<\< f.values(v) \<\< std::endl;
   * @endcode
   */
  namespace algorithms {
    /**
     * Perform an optimization following Gauss Newton algorithm
     * @param f functions
     * @param parameters intial guess of the parameters value
     * @param iter maximal number of iterations
     * @param epsilon if the remain is below epsilon the function return
     * @param gamma the coeficient apply to the derivative
     * @return the remain
     */
    template< class _TFunction_, typename _TType_  >
    _TType_ gradientDescent(_TFunction_* f, 
                            ublas::vector<_TType_>& parameters, 
                            int iter, 
                            _TType_ epsilon, 
                            _TType_ gamma);
    /**
     * Perform an optimization following Gauss Newton algorithm
     * @param f functions
     * @param parameters intial guess of the parameters value
     * @param iter maximal number of iterations
     * @param epsilon if the remain is below epsilon the function return
     * @return the remain
     */
    template< class _TMethod_ >
    typename _TMethod_::Type gaussNewton(typename _TMethod_::Function* f, 
                                         ublas::vector<typename _TMethod_::Type>& parameters, 
                                         int iter, 
                                         typename _TMethod_::Type epsilon);
    /**
     * Perform an optimization following Levenberg Marquardt algorithm
     * @param f functions
     * @param parameters intial guess of the parameters value
     * @param iter maximal number of iterations
     * @param epsilon if the remain is below epsilon the function return
     * @param lambda0 the intial value of the damping parameter
     * @param nu adjustment coefficient of the damping parameter
     * @return the remain
     */
    template< class _TFunction_, typename _TType_  >
    _TType_ levenbergMarquardt(_TFunction_* f, 
                               ublas::vector<_TType_>& parameters, 
                               int iter, 
                               _TType_ epsilon, 
                               _TType_ lambda0, 
                               _TType_ nu);
  }
}
// Implementation
namespace optimization {
  namespace details { // Private functions used by the algorithms
    // Compute the remain
    template<typename _TType_>
    double computeRemain(const ublas::vector<_TType_>& values) {
      _TType_ remain = 0.0;
      for(unsigned-int j = 0; j < values.size(); j++)
      {
        _TType_ v = values[j];
        remain += v * v;
      }
      return sqrt(remain / values.size() );
    }
    // Solve (common to levenbergMarquardt and gaussNewton
    template<typename _TType_>
    ublas::vector<double> solve(const ublas::matrix<_TType_>& M, 
                                const ublas::matrix<_TType_>& jacobian, 
                                const ublas::vector<_TType_>& values, 
                                int p_count) {
      ublas::matrix<_TType_> MD(M.size1(),M.size2());
      MD = M;
      // Solve the linear system
//       jblas::ilut_precond< ublas::matrix<_TType_> > P(M, 10, 1e-4);
//       jblas::iteration iter(1E-8);  // defines an iteration object, with a max residu of 1E-8
      ublas::vector<_TType_> B(p_count); // Unknown and left hand side.
      B = ublas::prec_prod(ublas::trans(jacobian), values);
      B = B * -1.0;
      //jblas::lu_solve(MD, X, B);
      int ierr = lapack::gesv(MD, B); 
      if (ierr!=0) {
        throw(jmath::LapackException(ierr, 
                                     "joptimization::dtails::solve: error in lapack::gesv() routine",
                                     __FILE__,
                                     __LINE__));
      }
       
//       std::cout << " M = " << M << endl;
//       std::cout << " Values= " << values << endl;
//       std::cout << " B = " << B << endl;
//       iter.set_maxiter(250);
//       jblas::gmres(M, X, B, P, 50, iter);  // execute the GMRES algorithm
//       jblas::qmr(M, X, B, P, iter);  // execute the GMRES algorithm
/*      std::cout << " B = " << B << endl;
      std::cout << " X = " << X << endl;*/
//       ublas::vector<_TType_> tmp(p_count);
//       tmp = ublas::prec_prod(M, X);
//       tmp = -1.0 * tmp;
//       //add(tmp,B);
//       B = tmp + B;
//       std::cout << " M * X - B = " << X << endl;
      // Update
      return B;
    }
  }
  namespace methods {
    template< class _TFunction_, typename _TType_ >
    void GaussNewton< _TFunction_, _TType_ >::computeM(const ublas::matrix<_TType_>& jacobian, 
                                                       const _TFunction_* f, 
                                                       ublas::vector<_TType_>& parameters, 
                                                       ublas::vector<_TType_>& values, 
                                                       ublas::matrix<_TType_> &M) {
      M = ublas::prec_prod( ublas::trans(jacobian), jacobian);
    }
    template< class _TFunction_, typename _TType_ >
    void Newton< _TFunction_, _TType_ >::computeM(const ublas::matrix<_TType_>& jacobian, 
                                                  const _TFunction_* f, 
                                                  ublas::vector<_TType_>& parameters, 
                                                  ublas::vector<_TType_>& values, 
                                                  ublas::matrix<_TType_> &M) {
      M = ublas::prec_prod( ublas::trans(jacobian), jacobian);
      // Compute the sum hessian
      for(int i = 0; i < f->count(); i++) {
        ublas::matrix< _TType_ > hessian = f->hessian( parameters,i);
        hessian = values[0] * hessian;
        //add( hessian, M);
        M = hessian + M;
      }
    }
  }
  namespace algorithms {
    // Gradient descent
    template< class _TFunction_, typename _TType_  >
    _TType_ gradientDescent(_TFunction_* f, 
                            ublas::vector<_TType_>& parameters, 
                            int iter, 
                            _TType_ epsilon, 
                            _TType_ gamma) {
      int p_count = parameters.size();
      int f_count = f->count();
      for(int i = 0; i < iter; i++) {
        // Compute the values
        ublas::vector<_TType_> values = f->values(parameters);
        // Compute the remaining
        _TType_ remain = details::computeRemain(values);
        JFR_DEBUG("Iteration " << i << ", error : " << remain << std::endl);
        if(remain < epsilon) {
          return remain;
        }
        // Compute the jacobian
        ublas::matrix<_TType_> jacobian = f->jacobian(parameters);
  //       std::cout << "jacobian = " << jacobian << " values " << values << std::endl;
  //       std::cout << "parameters = " << parameters << std::endl;
        // update
        for(int j = 0; j < p_count; j++) {
          double update = 0.0;
          for(int i = 0; i < f_count; i++) {
            update += values[i] * jacobian(i,j);
          }
  //         std::cout << " update = " << update << " j = " << j << endl;
          parameters[j] -=  update * gamma;
        }
        JFR_DEBUG("parameters = " << parameters << std::endl);
      }
      return details::computeRemain(f->values(parameters));
    }
    // Gauss-Newton
    template< class _TMethod_ >
    typename _TMethod_::Type gaussNewton(typename _TMethod_::Function* f, 
                                         ublas::vector<typename _TMethod_::Type>& parameters, 
                                         int iter, 
                                         typename _TMethod_::Type epsilon) {
      int p_count = parameters.size();
      int f_count = f->count();
      for(int i = 0; i < iter; i++) {
        // Compute the values
        ublas::vector<typename _TMethod_::Type> values = f->values(parameters);
        // Compute the remaining
        typename _TMethod_::Type remain = details::computeRemain(values);
        JFR_DEBUG("Iteration " << i << ", error : " << remain << std::endl);
        if(remain < epsilon) {
          return remain;
        }
        // Compute the jacobian
        ublas::matrix<typename _TMethod_::Type> jacobian = f->jacobian(parameters);
  //       std::cout << "Jacobian = " << jacobian << endl;
  //       std::cout << "Transposed Jacobian = " << ublas::trans(jacobian) << endl;
        ublas::matrix<typename _TMethod_::Type> M(jacobian.size2(), jacobian.size2());
        _TMethod_::computeM(jacobian, f, parameters, values, M);
        ublas::vector<typename _TMethod_::Type> X = details::solve(M, jacobian, values, p_count);
        //add(X, parameters);
        X = X + parameters;
        JFR_DEBUG(" Parameters " << parameters << endl);
      }
      return details::computeRemain(f->values(parameters));
    }
    template< class _TFunction_, typename _TType_  >
    _TType_ levenbergMarquardt(_TFunction_* f, 
                               ublas::vector<_TType_>& parameters, 
                               int iter, 
                               _TType_ epsilon, 
                               _TType_ lambda0, 
                               _TType_ nu) {
      int p_count = parameters.size();
//       int f_count = f->count();
      _TType_ lambda = lambda0 / nu;
      _TType_ invnu = 1.0 / nu;
      if(nu < invnu) {
        double t = nu;
        nu = invnu;
        invnu = t;
      }
        // Compute the values
      ublas::vector<_TType_> values = f->values(parameters);
        // Compute the remaining
      _TType_ previousremain = details::computeRemain(f->values(parameters));
      // optimization
      for(int i = 0; i < iter; i++) {
        if(previousremain < epsilon) {
          return previousremain;
        }
        // Compute the jacobian
        ublas::matrix<_TType_> jacobian = f->jacobian(parameters);
  //       std::cout << "Jacobian = " << jacobian << endl;
  //       std::cout << "Transposed Jacobian = " << ublas::trans(jacobian) << endl;
        ublas::matrix<_TType_> M(jacobian.size2(), jacobian.size2());
        M = ublas::prec_prod( ublas::trans(jacobian), jacobian);
        
        // Compute lambda * identity
        ublas::identity_matrix<_TType_> M2(jacobian.size2());
        M2 = M2 * -lambda;
        // substract it to M
        //add(M, M2);
        M = M + M2; 
//         std::cout << M2 << std::endl;
/*        std::cout << std::endl << "[[";
        for(unsigned-int m = 0; m < M2.size1(); m++)
        {
          for(unsigned-int n = 0; n < M2.size2(); n++)
          {
            std::cout << M2(n,m) << " ";
          }
          std::cout << "];[";
        }
        std::cout << std::endl;
        std::cout << std::endl;*/
        ublas::vector<_TType_> X = details::solve(M2, jacobian, values, p_count);
        // Compute the update
        ublas::vector<double> newparameters = parameters;
        //add(X, newparameters);        
        X = X + newparameters;
        ublas::vector<_TType_> newvalues = f->values(newparameters);
        double newremain = details::computeRemain(newvalues);
        // Check if the update is an improvement
        JFR_DEBUG( "Iteration " << i << ", error : " << previousremain << ", new error " << newremain << ", lambda : "<< lambda << std::endl );
        if(newremain < previousremain) {
          parameters = newparameters;
          values = newvalues;
          previousremain = newremain;
          lambda *= invnu;
//           lambda = lambda0;
        } else {
          lambda *= nu;
        }
        if(lambda == 0.0 or lambda >= 1.0e200) {
          lambda = lambda0;
//           return previousremain;
        }
        JFR_DEBUG(" Parameters " << parameters << endl);
      }
      return details::computeRemain(f->values(parameters));
    }
  }
}

#endif
