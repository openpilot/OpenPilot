/* $Id$ */

/** tcl tools for module jmath.
 *
 * \file jmathTools.i
 * \ingroup jmath
 */   

%{

#include <string>
#include <sstream>

#include "boost/version.hpp"

#include "kernel/jafarException.hpp"
#include "jmath/jblas.hpp"

%}

%inline %{

namespace jafar {
  namespace jmath {

    template<class A>
    std::string print(const A& a_) {
      std::ostringstream os;
      os << a_ << std::endl;
      return os.str();
    };

    /** Set the value (and the size) of a vector or a matrix using the operator>> in ublas.
     * @param t a vector or a matrix
     * @param value the value to be set, using ublas
     * format. 
     * - vector: [n](v1,v2,...,vn)
     * - matrix: [n1,n2]((m_11,m_12,...,m_1n2),(m_21,m_22,...,m_2n2),...,(m_n11,m_n12,...,m_n1n2))
     */	
    template<class T>
    void setSizeValue(T& t, const std::string& value) {
      std::stringstream s;
      s << value;
      JFR_IO_STREAM(s >> t, "reading matrix or vector from string");
    };

#if BOOST_VERSION < 103300
    
    template<>
    void setSizeValue<jblas::sym_mat>(jblas::sym_mat& mat, const std::string& value) {
      std::stringstream s;
      jblas::mat matTmp(mat.size1(), mat.size2());
      JFR_IO_STREAM(s >> matTmp, "reading matrix from string");
      mat.assign(matTmp);
    };

#endif

    /** Same as setSizeValue() for a vector except that size of \a vec
     * is used.
     *  format: (v1,v2,...,vn)
     */
    template<class Vec>
    void setValueVec(Vec& vec, const std::string& value) {
      std::stringstream s;
      s << "[" << vec.size() << "]" << value;
      JFR_IO_STREAM(s >> vec,"reading vector from string");
    };


    /** Same as setSizeValue() for a matrix except that size of \a mat
     * is used.
     *  format: ((m_11,m_12,...,m_1n2),(m_21,m_22,...,m_2n2),...,(m_n11,m_n12,...,m_n1n2)) 
     */
    template<class Mat>
    void setValueMat(Mat& mat, const std::string& value) {
      std::stringstream s;
      s << "[" << mat.size1() << "," << mat.size2() << "]" << value;
      JFR_IO_STREAM(s >> mat, "reading matrix from string");
    };

#if BOOST_VERSION < 103300
    
    template<>
    void setValueMat<jblas::sym_mat>(jblas::sym_mat& mat, const std::string& value) {
      std::stringstream s;
      jblas::mat matTmp(mat.size1(), mat.size2());
      s << "[" << mat.size1() << "," << mat.size2() << "]" << value;
      JFR_IO_STREAM(s >> matTmp, "reading matrix from string");
      mat.assign(matTmp);
    };

#endif

  }
}

%}
