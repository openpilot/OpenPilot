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
#include "jmath/ublasCompatibility.hpp"

%}

%inline %{

namespace jafar {
  namespace jmath {

    template<class A>
    std::string print(const A& a_) {
      std::ostringstream os;
      os << a_ << std::endl;
      return os.str();
    }
    template<class T>
    T add(const T& t1, const T& t2)
    {
      return t1 + t2;
    }
    template<class T>
    T sub(const T& t1, const T& t2)
    {
      return t1 - t2;
    }

    template<class T, class T2>
    T div(const T& t1, const T2& t2)
    {
      return t1 / t2;
    }
    template<class T, class T2>
    T scalmul(const T& t1, const T2& t2)
    {
      return t1 * t2;
    }
    template<class T, class T2>
    T2 vecmatmul(const T& t1, const T2& t2)
    {
      return ublas::prod(t1, t2);
    }
    jblas::mat mul(const jblas::vec& t1, const jblas::vec& t2)
    {
      jblas::mat M(t1.size(), t2.size());
      for(unsigned int i = 0; i < t1.size(); i++)
      {
        for(unsigned int j = 0; j < t2.size(); j++)
        {
          M(i,j) = t1(i) * t2(j);
        }
      }
      return M;
    }
    template<class T, class T2, class T3>
    T3 matmatmul(const T& t1, const T2& t2)
    {
      return ublas::prod(t1, t2);
    }
    
     template<class T, class T2>
    T& assignVec3(T& t1, const T2& t2)
    {
      return t1.assign(t2);
    }
    

    template<class T, class T2>
    T& assignMat(T& t1, const T2& t2)
    {
      return t1.assign(t2);
    }
    
    template<class T>
    T inv(const T& mat)
    {
      T invmat;
      jmath::ublasExtra::inv(mat, invmat);
      return invmat;
    }
    
    template<class T>
        T trans(const T& mat)
    {
      return ublas::trans(mat);
    }
    
    jblas::vec rangeToVec(const jblas::vec_range& r)
    {
      return r;
    }
    jblas::vec_range* vecToRange( jblas::vec& v, int i1, int i2)
    {
      return new jblas::vec_range( v, ublas::range(i1,i2));
    }


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
    }

    /** Same as setSizeValue() for a vector except that size of \a vec
     * is used.
     *  format: (v1,v2,...,vn)
     */
    template<class Vec>
    void setValueVec(Vec& vec, const std::string& value) {
      std::stringstream s;
      s << "[" << vec.size() << "]" << value;
      JFR_IO_STREAM(s >> vec,"reading vector from string");
    }


    /** Same as setSizeValue() for a matrix except that size of \a mat
     * is used.
     *  format: ((m_11,m_12,...,m_1n2),(m_21,m_22,...,m_2n2),...,(m_n11,m_n12,...,m_n1n2)) 
     */
    template<class Mat>
    void setValueMat(Mat& mat, const std::string& value) {
      std::stringstream s;
      s << "[" << mat.size1() << "," << mat.size2() << "]" << value;
      JFR_IO_STREAM(s >> mat, "reading matrix from string");
    }
   
    /** Dummy function that returns the ith element of a vec
     * 
     */
		template<class V>
		double getElementAt(const V& v, const unsigned int& rank) {
			if(rank < v.size()) return v(rank); else return 0.;
		}
		template<class V>
		double setElementAt(V& v, const unsigned int& rank, double val) {
			if(rank < v.size()) return v(rank) = val;
		}
		
		/** Dummy function that returns the (i,j)th element of a mat
		* 
		*/
		template<class Mat>
		double getMatElementAt(const Mat& m, const unsigned int& i, const unsigned int& j) {
			if( (i < m.size1()) and (j < m.size2()) ) return m(i,j); else return 0.;
		}
		template<class Mat>
		void setMatElementAt(Mat& m, const unsigned int& i, const unsigned int& j, double val) {
			if( (i < m.size1()) and (j < m.size2()) ) m(i,j)= val;
		}
 
  }
}

%}
