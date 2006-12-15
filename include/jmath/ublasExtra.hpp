/* $Id$ */

#ifndef JMATH_UBLAS_EXTRA_HPP
#define JMATH_UBLAS_EXTRA_HPP

#include <cmath>

#include "jafarConfig.h"

#if BOOST_VERSION < 103301 // missing includes in lu.hpp
#include "boost/numeric/ublas/vector.hpp"
#include "boost/numeric/ublas/vector_proxy.hpp"
#include "boost/numeric/ublas/matrix.hpp"
#include "boost/numeric/ublas/matrix_proxy.hpp"
#include "boost/numeric/ublas/triangular.hpp"
#include "boost/numeric/ublas/operation.hpp"
#endif

#if BOOST_VERSION == 103301 // missing includes in lu.hpp
#include "boost/numeric/ublas/triangular.hpp"
#endif

#ifdef HAVE_BOOST_SANDBOX
#ifdef HAVE_LAPACK
#include "boost/numeric/bindings/lapack/gesdd.hpp"
#include "boost/numeric/bindings/traits/ublas_matrix.hpp"
#include "boost/numeric/bindings/traits/ublas_vector.hpp"
#endif
#endif

#include "boost/numeric/ublas/lu.hpp"

#include "kernel/jafarException.hpp"
#include "kernel/jafarDebug.hpp"

#include "jmath/jblas.hpp"
#include "jmath/jmathException.hpp"



namespace jafar {
  namespace jmath {

    /** \ingroup jmath */
    /*\@{*/

    /** This namespace contains some extra tools for ublas vector and
     * matrix.
     */
    namespace ublasExtra {

      namespace details {
	// a small value
	const double EPSILON = 1e-8;
      }
      
      /*
       * Vector
       */

      template<class V>
      void setVectorValue(V& v, const double* val_, std::size_t size_) {
	JFR_PRECOND(v.size()==size_,
                    "ublasExtra::setValue: size of v does not match");
        for (std::size_t i = 0 ; i < v.size() ; i++) {
          v(i) = val_[i];
        }
      };


      /** normalize a vector.
       */
      template<class V>
      void normalize(V& v) {
	double n = ublas::norm_2(v);
	JFR_NUMERIC(n > details::EPSILON,
		    "ublasExtra::normalize: vector too small");
	v /= n;
      };

      /** jacobian of normalize().
       */
      template<class V>
      void normalizeJac(V& v, jblas::mat& J) {
	JFR_NUMERIC(ublas::norm_2(v) > details::EPSILON,
		    "ublasExtra::normalizeJac: vector too small");
	JFR_PRECOND(J.size1() == v.size() && J.size2() == v.size(),
		    "ublasExtra::normalizeJac: size of J is invalid");
	switch(v.size()) {
	case 2:
	  {
	    double x = v(0);
	    double y = v(1);
	    /* begin maple copy/paste */
	    double t1 = x * x;
	    double t2 = y * y;
	    double t3 = t1 + t2;
	    double t4 = sqrt(t3);
	    double t6 = 0.1e1 / t4 / t3;
	    double t8 = 0.1e1 / t4;
	    double t11 = t6 * x * y;
	    J(0,0) = -t6 * t1 + t8;
	    J(0,1) = -t11;
	    J(1,0) = -t11;
	    J(1,1) = -t6 * t2 + t8;
	    /* end maple copy/paste */
	  }
	  return;
	case 3:
	  {
	    double x = v(0);
	    double y = v(1);
	    double z = v(2);
	    /* begin maple copy/paste */
	    double t1 = x * x;
	    double t2 = y * y;
	    double t3 = z * z;
	    double t4 = t1 + t2 + t3;
	    double t5 = sqrt(t4);
	    double t7 = 0.1e1 / t5 / t4;
	    double t9 = 0.1e1 / t5;
	    double t11 = t7 * x;
	    double t12 = t11 * y;
	    double t13 = t11 * z;
	    double t17 = t7 * y * z;
	    J(0,0) = -t7 * t1 + t9;
	    J(0,1) = -t12;
	    J(0,2) = -t13;
	    J(1,0) = -t12;
	    J(1,1) = -t7 * t2 + t9;
	    J(1,2) = -t17;
	    J(2,0) = -t13;
	    J(2,1) = -t17;
	    J(2,2) = -t7 * t3 + t9;
	    /* end maple copy/paste */

	  }
	  return;
	default:
	  JFR_RUN_TIME("ublasExtra::normalizeJac: not implemented yet");
	}
      };

      /** Compute the jacobian of inner product.
       */
      template<std::size_t N, class Vec1, class Vec2, class Mat>
      void inner_prodJac(Vec1 const& u1, Vec2 const& u2, Mat& J)
      {
	JFR_PRECOND(u1.size() == N && u2.size() == N,
		    "ublasExtra::inner_prodJac:" << N << "-vectors");
	JFR_PRECOND(J.size1() == 1 && J.size2() == 2*N,
		    "ublasExtra::inner_prodJac:");
	for (std::size_t i = 0 ; i < N ; ++i) {
	  J(0,i) = u2(i);
	  J(0,N+i) = u1(i);
	}
      };

      /** Compute the jacobian of norm_2.
       */
      template<std::size_t N, class Vec, class Mat>
      void norm_2Jac(Vec const& u, Mat& J)
      {
	JFR_PRECOND(u.size() == N,
		    "ublasExtra::norm_2Jac: " << N << "-vectors");
	JFR_PRECOND(J.size1() == 1 && J.size2() == N,
		    "ublasExtra::norm_2Jac:");
	double d=0;
	for (std::size_t i = 0 ; i < N ; ++i) {
	  d+=pow(u(i),2);
	}
	d = sqrt(d);
	for (std::size_t i = 0 ; i < N ; ++i) {
	  J(0,i) = u(i)/d;
	}
      };

      /** Compute the cross product of \a v1 and \a v2, results is
       *  stored in \a vRes.
       */
      template<class Vec1, class Vec2, class VecRes>
      void crossProd(Vec1 const& v1, Vec2 const& v2, VecRes& vRes) {
	JFR_PRECOND(v1.size()==3 && v2.size()==3 && vRes.size()==3,
		    "ublasExtra::crossProd: 3D vector");

	vRes(0) = v1(1) * v2(2) - v1(2) * v2(1);
	vRes(1) = v1(2) * v2(0) - v1(0) * v2(2);
	vRes(2) = v1(0) * v2(1) - v1(1) * v2(0);
      };


      /** Compute the cross product of \a v1 and \a v2, the result is
       *  returned.
       */
      template<class Vec1, class Vec2>
      jblas::vec3 crossProd(Vec1 const& v1, Vec2 const& v2) {
	jblas::vec3 vRes;
	crossProd(v1,v2,vRes);
	return vRes;
      };
      
      /*
       * Matrix
       */

      /** Format a matrix output it to a string. Each matrix row
       * corresponds to a line.
       */
      template<class Mat>
      std::string prettyFormat(Mat const& m_) {
	std::stringstream s;
	for (std::size_t i = 0 ; i < m_.size1() ; ++i) {
	  for (std::size_t j = 0 ; j < m_.size2() ; ++j) {
	    s << m_(i,j) << "\t";
	}
	  s << std::endl;
	}
	return s.str();
      };

			/*!
			 * Format a matrix output to a string in matlab syntax
			 */
			template<class Mat>
			std::string matlabFormat(Mat const& m_) {
				std::stringstream s;
				s << "[";
				for (std::size_t i = 0 ; i < m_.size1() ; ++i) {
					for (std::size_t j = 0 ; j < m_.size2() ; ++j) {
						if ( j != (m_.size2()-1) )
							s << m_(i,j) << ",";
						else
							s << m_(i,j);
					}
					if ( i != (m_.size1()-1) )
						s << ";";
					else
					 s << "]";
				};
				
				return s.str();
			};
      template<class M>
      void setMatrixValue(M& m, const double* val_, std::size_t size1_, std::size_t size2_) {
        JFR_PRECOND(m.size1()==size1_ && m.size2()==size2_,
                    "ublasExtra::setValue: size of m does not match");
	unsigned int k = 0;
        for (std::size_t i = 0 ; i < m.size1() ; i++) {
          for (std::size_t j = 0 ; j < m.size2() ; j++) {
            m(i,j) = val_[k];
            k++;
          }
        }
      };

      namespace details {

	template<class M>
	double det2(const M& m_) {
	  return m_(0,0)*m_(1,1) - m_(1,0)*m_(0,1);
	}

	template<class M>
	double det3(const M& m_) {
	  return m_(0,0)*m_(1,1)*m_(2,2) 
	    + m_(0,1)*m_(1,2)*m_(2,0) 
	    + m_(0,2)*m_(1,0)*m_(2,1) 
	    - m_(2,0)*m_(1,1)*m_(0,2) 
	    - m_(2,1)*m_(1,2)*m_(0,0) 
	    - m_(2,2)*m_(1,0)*m_(0,1);
	}

	template<class M>
	void inv2(const M& m_, M& m_inv) {
	  m_inv(0,0) = m_(1,1);
	  m_inv(0,1) = -1.0*m_(0,1);
	  m_inv(1,0) = -1.0*m_(1,0);
	  m_inv(1,1) = m_(0,0);
	  m_inv /= det2(m_);
	};

	template<class M>
	void inv3(const M& m_, M& m_inv) {
	  m_inv(0,0) = m_(1,1)*m_(2,2)-m_(1,2)*m_(2,1);
	  m_inv(0,1) = m_(2,1)*m_(0,2)-m_(2,2)*m_(0,1);
	  m_inv(0,2) = m_(0,1)*m_(1,2)-m_(0,2)*m_(1,1);
	  m_inv(1,0) = m_(2,0)*m_(1,2)-m_(1,0)*m_(2,2);
	  m_inv(1,1) = m_(0,0)*m_(2,2)-m_(2,0)*m_(0,2);
	  m_inv(1,2) = m_(1,0)*m_(0,2)-m_(0,0)*m_(1,2);
	  m_inv(2,0) = m_(1,0)*m_(2,1)-m_(2,0)*m_(1,1);
	  m_inv(2,1) = m_(2,0)*m_(0,1)-m_(0,0)*m_(2,1);
	  m_inv(2,2) = m_(0,0)*m_(1,1)-m_(1,0)*m_(0,1);

	  m_inv /= det3(m_);
	};

      } // namespace details

      /** Find maximum value of a matrix.
       * \warning it returns a double whatever matrix type...
       */
      template<class M>
      double max(const M& m_) {
        double max = m_(0,0);
        for (std::size_t i=0 ; i < m_.size1() ; i++) {
          for (std::size_t j=0 ; j < m_.size2() ; j++) {
            if (m_(i,j) > max) {
              max = m_(i,j);
            }
          }
        }
        return max;
      };

      /** Computes the trace of a matrix.
       *
       * \warning it returns a double whatever matrix type...
       */
      template<class M>
      double trace(const M& m_) {
        JFR_PRECOND(m_.size1() == m_.size2(),
                    "ublasExtra::trace: m_ must be square");
        double t=0;
        for (std::size_t i =0 ; i <m_.size1() ; i++) {
          t+=m_(i,i);
        }
        return t;
      };

      template<class M>
      double det(const M& m_) {
        JFR_PRECOND(m_.size1() == m_.size2(),
                    "ublasExtra::det: m_ must be a square matrix");
        switch(m_.size1()) {
        case 1:
          return m_(0,0);
        case 2:
          return details::det2(m_);
        case 3:
          return details::det3(m_);
        default:
          lu_det(m_);
        }  
      };

      template<class M>
      void inv(const M& m_, M& m_inv) {
        JFR_PRECOND(m_.size1() == m_.size2(),
                    "ublasExtra::inv: m_ must be a square matrix");
        switch(m_.size1()) {
        case 1:
          m_inv(0,0) = 1/m_(0,0);
          break;
        case 2:
	  details::inv2(m_, m_inv);
          break;
        case 3:
	  details::inv3(m_, m_inv);
          break;
        default:
          lu_inv(m_,m_inv);
        }
      };

      /** General matrix inversion routine.
       *  It uses lu_factorize and lu_substitute in uBLAS to invert a matrix 
       */
      template<class M1, class M2>
      void lu_inv(M1 const& m, M2& inv) {
	JFR_PRECOND(m.size1() == m.size2(),
		    "ublasExtra::lu_inv(): input matrix must be squared");
	JFR_PRECOND(inv.size1() == m.size1() && inv.size2() == m.size2(),
		    "ublasExtra::lu_inv(): invalid size for inverse matrix");

        using namespace boost::numeric::ublas;
        // create a working copy of the input
	jblas::mat mLu(m);

        // perform LU-factorization
	JFR_TRACE_BEGIN;
        lu_factorize(mLu);
	JFR_TRACE_END("ublasExtra::lu_inv");

        // create identity matrix of "inverse"
	jblas::mat mLuInv(jblas::identity_mat(m.size1()));

        // backsubstitute to get the inverse
	JFR_TRACE_BEGIN;
        lu_substitute<jblas::mat const, jblas::mat>(mLu, mLuInv);
	JFR_TRACE_END("ublasExtra::lu_inv");
	inv.assign(mLuInv);
      }

#ifdef HAVE_BOOST_SANDBOX
#ifdef HAVE_LAPACK

			/*!
			 * General matrix inversion routine.
			 * It use singular value decomposition to invert a matrix
			 */
			template<class M1, class M2>
		void svd_inv(M1 const& m, M2& inv) {
			JFR_PRECOND(m.size1() >= m.size2(),"ublasExtra::svd_inv: wrong matrix size");
			JFR_PRECOND(inv.size1() == m.size1() && inv.size2() == m.size2(),
				"ublasExtra::svd_inv(): invalid size for inverse matrix");
			jblas::mat_column_major working_m(m);
			jblas::vec s(m.size2());
			jblas::mat_column_major U(m.size1(),m.size2());
			jblas::mat_column_major VT(m.size1(),m.size2());  
			int ierr = boost::numeric::bindings::lapack::gesdd(working_m,s,U,VT);
			if (ierr!=0) {
				throw(jmath::LapackException(ierr, 
					"LinearLeastSquares::solve: error in lapack::gesdd() routine",
					__FILE__,
					__LINE__));
			}
			jblas::mat S(jblas::zero_mat(s.size(),s.size()));   
			for(unsigned int i=0;i<s.size();i++) {       
				JFR_ASSERT(s(i)!=0, "ublasExtra::svd_inv: singular matrix");        
			  S(i,i)=1/s(i);                     
			}               
			inv = ublas::prod(S,ublas::trans(U));                      
			inv = ublas::prod(ublas::trans(VT),inv);
		};
			
#endif
#endif 

      /** General matrix determinant. 
       *  It uses lu_factorize in uBLAS.
       */
      template<class M>
      double lu_det(M const& m) {
	JFR_PRECOND(m.size1() == m.size2(),
		    "ublasExtra::lu_det: matrix must be square");

        // create a working copy of the input
	jblas::mat mLu(m);
	ublas::permutation_matrix<std::size_t> pivots(m.size1());

	JFR_TRACE_BEGIN;
	lu_factorize(mLu, pivots);
	JFR_TRACE_END("ublasExtra::lu_det");

	double det = 1.0;

	for (std::size_t i=0; i < pivots.size(); ++i) {
	  if (pivots(i) != i)
	    det *= -1.0;
	  det *= mLu(i,i);
	}
	return det;
      }

    } // namespace ublasExtra

    /*\@}*/


  } // namespace jmath
} // namespace jafar

#endif // JMATH_UBLAS_EXTRA_HPP
