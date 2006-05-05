/* $Id$ */

#ifndef JMATH_UBLAS_COMPATIBILITY_HPP
#define JMATH_UBLAS_COMPATIBILITY_HPP

#include "boost/version.hpp"

#if BOOST_VERSION < 103300

namespace boost { namespace numeric { namespace ublas {

  // Special input operator for symmetrix_matrix
  template<class E, class T, class MT, class MF1, class MF2, class MA>
  // BOOST_UBLAS_INLINE This function seems to be big. So we do not let the compiler inline it.
  std::basic_istream<E, T> &operator >> (std::basic_istream<E, T> &is,
                                         symmetric_matrix<MT, MF1, MF2, MA> &m) {
    typedef typename symmetric_matrix<MT, MF1, MF2, MA>::size_type size_type;
    E ch;
    size_type size1, size2;
    MT value;
    if (is >> ch && ch != '[') {
      is.putback (ch);
      is.setstate (std::ios_base::failbit);
    } else if (is >> size1 >> ch && ch != ',') {
      is.putback (ch);
      is.setstate (std::ios_base::failbit);
    } else if (is >> size2 >> ch && (size2 != size1 || ch != ']')) { // symmetric matrix must be square
      is.putback (ch);
      is.setstate (std::ios_base::failbit);
    } else if (! is.fail ()) {
      symmetric_matrix<MT, MF1, MF2, MA> s (size1, size2);
      if (is >> ch && ch != '(') {
        is.putback (ch);
        is.setstate (std::ios_base::failbit);
      } else if (! is.fail ()) {
        for (size_type i = 0; i < size1; i ++) {
          if (is >> ch && ch != '(') {
            is.putback (ch);
            is.setstate (std::ios_base::failbit);
            break;
          }
          for (size_type j = 0; j < size2; j ++) {
            if (is >> value >> ch && ch != ',') {
              is.putback (ch);
              if (j < size2 - 1) {
                is.setstate (std::ios_base::failbit);
                break;
              }
            }
            if (i <= j) { 
              // this is the first time we read this element - set the value
              s(i,j) = value;
            }
            else if ( s(i,j) != value ) {
              // matrix is not symmetric
              is.setstate (std::ios_base::failbit);
              break;
            }
          }
          if (is >> ch && ch != ')') {
            is.putback (ch);
            is.setstate (std::ios_base::failbit);
            break;
          }
          if (is >> ch && ch != ',') {
            is.putback (ch);
            if (i < size1 - 1) {
              is.setstate (std::ios_base::failbit);
              break;
            }
          }
        }
        if (is >> ch && ch != ')') {
          is.putback (ch);
          is.setstate (std::ios_base::failbit);
        }
      }
      if (! is.fail ())
        m.swap (s);
    }
    return is;
  }

}}}

#endif

#endif // JMATH_UBLAS_COMPATIBILITY_HPP
