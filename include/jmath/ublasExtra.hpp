/* $Id$ */

#ifndef JMATH_UBLAS_EXTRA_HPP
#define JMATH_UBLAS_EXTRA_HPP

#include <cmath>

#include "kernel/jafarException.hpp"
#include "kernel/jafarDebug.hpp"

// #include "jmath/lapack_bindings.hpp"

namespace jafar {
  namespace jmath {

    template<class Mat>
    std::string prettyFormat(Mat const& m_) {
      std::stringstream s;
      for (std::size_t i = 0 ; i < m_.size1() ; ++i) {
	for (std::size_t j = 0 ; j < m_.size2() ; ++j) {
	   s << m_(i,j) << " ";
	}
	s << std::endl;
      }
      return s.str();
    };

    /** This class provides some extra tools for ublas vector.
     *
     * \ingroup jmath
     */
    class VectorTools {

    protected:

      /** implementation of setValue function.
       */
      template<class V>
      static inline void _setValue(V& v, const double* val_) {
        for (std::size_t i = 0 ; i < v.size() ; i++) {
          v(i) = val_[i];
        }
      };

    public:

     template<class V>
      static inline void setValue(V& v, const double* val_) {
        VectorTools::_setValue(v, val_);
      };

      template<class V>
      static inline void setValue(V& v, const double* val_, std::size_t size_) {
        JFR_PRECOND(v.size()==size_,
                    "VectorTools::setValue: size of v does not match");
        VectorTools::_setValue(v, val_);
      };
      
    }; // class VectorTools


    /** This class provides some extra tools for ublas matrix.
     *
     * \ingroup jmath
     */
    class MatrixTools {

    protected:

      /** implementation of setValue function.
       */
      template<class M>
      static inline void _setValue(M& m, const double* val_) {
        unsigned int k = 0;
        for (std::size_t i = 0 ; i < m.size1() ; i++) {
          for (std::size_t j = 0 ; j < m.size2() ; j++) {
            m(i,j) = val_[k];
            k++;
          }
        }
      };

      template<class M>
      inline static double det2(const M& m_) {
        return m_(0,0)*m_(1,1) - m_(1,0)*m_(0,1);
      }

      template<class M>
      inline static double det3(const M& m_) {
        return m_(0,0)*m_(1,1)*m_(2,2) 
          + m_(0,1)*m_(1,2)*m_(2,0) 
          + m_(0,2)*m_(1,0)*m_(2,1) 
          - m_(2,0)*m_(1,1)*m_(0,2) 
          - m_(2,1)*m_(1,2)*m_(0,0) 
          - m_(2,2)*m_(1,0)*m_(0,1);
      }

      template<class M>
      inline static void inv2(const M& m_, M& m_inv) {
        m_inv(0,0) = m_(1,1);
        m_inv(0,1) = -1.0*m_(0,1);
        m_inv(1,0) = -1.0*m_(1,0);
        m_inv(1,1) = m_(0,0);
        m_inv /= det2(m_);
      };

      template<class M>
      inline static void inv3(const M& m_, M& m_inv) {
        m_inv(0,0) = m_(1,1)*m_(2,2)-m_(2,1)*m_(1,2);
        m_inv(0,1) = m_(2,1)*m_(0,2)-m_(0,1)*m_(2,2);
        m_inv(0,2) = m_(0,1)*m_(1,2)-m_(1,1)*m_(0,2);
        m_inv(1,0) = m_(2,0)*m_(1,2)-m_(1,0)*m_(2,2);
        m_inv(1,1) = m_(0,0)*m_(2,2)-m_(2,0)*m_(0,2);
        m_inv(1,2) = m_(1,0)*m_(0,2)-m_(0,0)*m_(1,2);
        m_inv(2,0) = m_(1,0)*m_(2,1)-m_(2,0)*m_(1,1);
        m_inv(2,1) = m_(2,0)*m_(0,1)-m_(0,0)*m_(2,1);
        m_inv(2,2) = m_(0,0)*m_(1,1)-m_(1,0)*m_(0,1);

        m_inv /= det3(m_);

      };
      

    public:

      template<class M>
      static inline void setValue(M& m, const double* val_) {
        MatrixTools::_setValue(m, val_);
      };

      template<class M>
      static inline void setValue(M& m, const double* val_, std::size_t size1_, std::size_t size2_) {
        JFR_PRECOND(m.size1()==size1_ && m.size2()==size2_,
                    "MatrixTools::setValue: size of m does not match");
        MatrixTools::_setValue(m, val_);
      };

      template<class M>
      static double max(const M& m_) {
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
      static double trace(const M& m_) {
        JFR_PRECOND(m_.size1() == m_.size2(),
                    "MatrixTools::trace: m_ must be square");
        double t=0;
        for (std::size_t i =0 ; i <m_.size1() ; i++) {
          t+=m_(i,i);
        }
        return t;
      };

      template<class M>
      static double det(const M& m_) {
        JFR_PRECOND(m_.size1() == m_.size2(),
                    "MatrixTools::det: m_ must be a square matrix");
        switch(m_.size1()) {
        case 1:
          return m_(0,0);
        case 2:
          return det2(m_);
        case 3:
          return det3(m_);
        default:
          JFR_RUN_TIME("MatrixTools::det: not implemented yet");
        }  
      };

      template<class M>
      static void inv(const M& m_, M& m_inv) {
        JFR_PRECOND(m_.size1() == m_.size2(),
                    "MatrixTools::inv: m_ must be a square matrix");
        switch(m_.size1()) {
        case 1:
          m_inv(0,0) = 1/m_(0,0);
          break;
        case 2:
          inv2(m_, m_inv);
          break;
        case 3:
          inv3(m_, m_inv);
          break;
        default:
          JFR_RUN_TIME("MatrixTools::inv: not implemented yet");
        }
      };

//       static int testLapack(jblas::mat& A, jblas::mat& B) {
//         int err = lapack::gesv(A,B);
//         return err;
//       };

    }; // class MatrixTools

  } // namespace jmath
} // namespace jafar

#endif // JMATH_UBLAS_EXTRA_HPP
