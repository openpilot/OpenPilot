/* $Id$ */

#ifndef JMATH_UBLAS_EXTRA_HPP
#define JMATH_UBLAS_EXTRA_HPP

#include <cmath>

#include "kernel/jafarException.hpp"
#include "kernel/jafarDebug.hpp"

#include "jmath/jblas.hpp"

// #include "jmath/lapack_bindings.hpp"

namespace jafar {
  namespace jmath {

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

    /** This namespace contains some extra tools for ublas vector and matrix.
     *
     * \ingroup jmath
     */
    namespace ublasExtra {

      /// a small value
      const double EPSILON = 1e-8;

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
	JFR_NUMERIC(n > EPSILON,
		    "VectorTools::normalize: vector too small");
	v /= n;
      };

      /** jacobian of normalize().
       */
      template<class V>
      void normalizeJac(V& v, jblas::mat& J) {
	JFR_NUMERIC(ublas::norm_2(v) > EPSILON,
		    "VectorTools::normalizeJac: vector too small");
	JFR_PRECOND(J.size1() == v.size() && J.size2() == v.size(),
		    "VectorTools::normalizeJac: size of J is invalid");
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
	  JFR_RUN_TIME("VectorTools::normalizeJac: not implemented yet");
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
		    "ublasExtra::norm_2Jac:" << N << "-vectors");
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

      /** Compute the cross product of two vectors.
       */
      template<class Vec1, class Vec2, class VecRes>
      void crossProd(Vec1 const& v1, Vec2 const& v2, VecRes& vRes) {
	JFR_PRECOND(v1.size()==3 && v2.size()==3 && vRes.size()==3,
		    "VectorTools::crossProd: 3D vector");

	vRes(0) = v1(1) * v2(2) - v1(2) * v2(1);
	vRes(1) = v1(2) * v2(0) - v1(0) * v2(2);
	vRes(2) = v1(0) * v2(1) - v1(1) * v2(0);
      };

      template<class Vec1, class Vec2>
      jblas::vec3 crossProd(Vec1 const& v1, Vec2 const& v2) {
	jblas::vec3 vRes;
	crossProd(v1,v2,vRes);
	return vRes;
      };
      
      /*
       * Matrix
       */

      template<class M>
      void setMatrixValue(M& m, const double* val_, std::size_t size1_, std::size_t size2_) {
        JFR_PRECOND(m.size1()==size1_ && m.size2()==size2_,
                    "MatrixTools::setValue: size of m does not match");
	unsigned int k = 0;
        for (std::size_t i = 0 ; i < m.size1() ; i++) {
          for (std::size_t j = 0 ; j < m.size2() ; j++) {
            m(i,j) = val_[k];
            k++;
          }
        }
      };

      namespace detail {

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

      } // namespace detail

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

      template<class M>
      double trace(const M& m_) {
        JFR_PRECOND(m_.size1() == m_.size2(),
                    "MatrixTools::trace: m_ must be square");
        double t=0;
        for (std::size_t i =0 ; i <m_.size1() ; i++) {
          t+=m_(i,i);
        }
        return t;
      };

      template<class M>
      double det(const M& m_) {
        JFR_PRECOND(m_.size1() == m_.size2(),
                    "MatrixTools::det: m_ must be a square matrix");
        switch(m_.size1()) {
        case 1:
          return m_(0,0);
        case 2:
          return detail::det2(m_);
        case 3:
          return detail::det3(m_);
        default:
          JFR_RUN_TIME("MatrixTools::det: not implemented yet");
        }  
      };

      template<class M>
      void inv(const M& m_, M& m_inv) {
        JFR_PRECOND(m_.size1() == m_.size2(),
                    "MatrixTools::inv: m_ must be a square matrix");
        switch(m_.size1()) {
        case 1:
          m_inv(0,0) = 1/m_(0,0);
          break;
        case 2:
	  detail::inv2(m_, m_inv);
          break;
        case 3:
	  detail::inv3(m_, m_inv);
          break;
        default:
          JFR_RUN_TIME("MatrixTools::inv: not implemented yet");
        }
      };


    } // namespace ublasExtra

    // deprecated
    namespace VectorTools=jafar::jmath::ublasExtra;
    namespace MatrixTools=jafar::jmath::ublasExtra;

  } // namespace jmath
} // namespace jafar

#endif // JMATH_UBLAS_EXTRA_HPP
