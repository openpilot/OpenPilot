/**
 * Matlab.hpp
 *
 *  Created on: 27/02/2010
 *      Author: nmansard, jsola
 *
 *  \file Matlab.hpp
 *
 *  ## Add a description here ##
 *
 * \ingroup jmath
 */

#ifndef MATLAB_HPP_
#define MATLAB_HPP_

#include <iostream>
#include <boost/numeric/ublas/blas.hpp>
#include <boost/numeric/ublas/symmetric.hpp>
#include "jmath/indirectArray.hpp"

namespace ublas = boost::numeric::ublas;

namespace jafar {
	namespace jmath {


		/**
		 * Class for serializing boost arrays into Matlab format.
		 * \author nmansard, jsola
		 *
		 * This class is used to pretty-print boost arrays in Matlab format using the \c << serialized operator.
		 * It can be used in two equivalent ways:\n
		 * - <c> cout << "M = " << MATLAB (M) << endl;  // <== this is a regular call to the constructor.</c>\n
		 * - <c> cout << "M = " << (MATLAB) M << endl; // <== this is a cast which also calls to the constructor if it exists.</c>\n
		 *
		 * Recognized objects are vectors, vector proxies, indirect arrays, ranges, slices, all sorts of matrices and matrix proxies.
		 *
		 * For indirect arrays, ranges and slices, one unit is added to each index to comply with Matlab indexing (which states the first element to be number 1, not 0).
		 *
		 * The result can be copied from the console and pasted in Matlab. The matrix M will be initialized in Matlab.
		 *
		 * Here is an example. The code:
		 * \code
		 * 	jblas::ind_array i(2);      // first define the indices
		 * 	i(0) = 2;                   //         {boost}  --> [MATLAB]
		 * 	i(1) = 0;                   // this is {2, 0}   --> [3, 1]
		 * 	ublas::range r(1, 3);       // this is {1, 2}   --> [2, 3]
		 * 	ublas::slice s(0, 2, 3);    // this is {0, 2, 4} -> [1, 3, 5]
		 * 	jblas::vec v(8);
		 * 	jblas::vec8 vb;             // then define some vectors
		 * 	randVector(v);
		 * 	randVector(vb);
		 * 	jblas::vec_range vr(v, r);
		 * 	jblas::vec_slice vs(v, s);
		 * 	jblas::vec_indirect vi(v, i);
		 *
		 * 	cout << "\n% Indirect arrays, ranges and slices \n%====================================" << endl;
		 * 	cout << "i   = " << (MATLAB) i << endl;
		 * 	cout << "r   = " << (MATLAB) r << endl;
		 * 	cout << "s   = " << (MATLAB) s << endl;
		 * 	cout << "v   = " << (MATLAB) v << endl;
		 *
		 * 	cout << "vi  = " << (MATLAB) vi << endl;
		 * 	cout << "vr  = " << (MATLAB) vr << endl;
		 * 	cout << "vs  = " << (MATLAB) vs << endl;
		 * 	cout << "vb  = " << (MATLAB) vb << endl;
		 * 	cout << "vbi = " << (MATLAB) ublas::project(vb, i) << endl;
		 * 	cout << "vbr = " << (MATLAB) ublas::project(vb, r) << endl;
		 * 	cout << "vbs = " << (MATLAB) ublas::project(vb, s) << endl;
		 *
		 * 	jblas::mat M(5, 5);          // check also on matrices
		 * 	randMatrix(M);
		 * 	jblas::mat_indirect Mi(M, i, i);
		 * 	jblas::mat_range Mr(M, r, r);
		 * 	jblas::mat_slice Ms(M, s, s);
		 * 	cout << "M   = " << (MATLAB) M << endl;
		 * 	cout << "Mi  = " << (MATLAB) Mi << endl;
		 * 	cout << "Mr  = " << (MATLAB) Mr << endl;
		 * 	cout << "Ms  = " << (MATLAB) Ms << endl;
		 * \endcode
		 *
		 * produces:
		 *
		 * \code
		 * % Indirect arrays, ranges and slices
		 * %====================================
		 * i   = [ 3, 1 ];
		 * r   = [ 2, 3 ];
		 * s   = [ 1, 3, 5 ];
		 * v   = [  -0.999984,  -0.736924,  0.511211,  -0.0826997,  0.0655345,  -0.562082,  -0.905911,  0.357729 ]';
		 * vi  = [  0.511211,  -0.999984 ]';
		 * vr  = [  -0.736924,  0.511211 ]';
		 * vs  = [  -0.999984,  0.511211,  0.0655345 ]';
		 * vb  = [  0.358593,  0.869386,  -0.232996,  0.0388327,  0.661931,  -0.930856,  -0.893077,  0.0594004 ]';
		 * vbi = [  -0.232996,  0.358593 ]';
		 * vbr = [  0.869386,  -0.232996 ]';
		 * vbs = [  0.358593,  -0.232996,  0.661931 ]';
		 * M   = ...
		 * [ 	 0.342299,	-0.984604,	-0.233169,	-0.866316,	-0.165028 ;
		 *   	 0.373545,	 0.177953,	 0.860873,	 0.692334,	 0.0538576 ;
		 *   	-0.81607,	 0.307838,	-0.168001,	 0.402381,	 0.820642 ;
		 *   	 0.524396,	-0.475094,	-0.905071,	 0.472164,	-0.343532 ;
		 *   	 0.265277,	 0.512821,	 0.982075,	-0.269323,	-0.505922 ];
		 * Mi  = ...
		 * [ 	-0.168001,	-0.81607 ;
		 *   	-0.233169,	 0.342299 ];
		 * Mr  = ...
		 * [ 	 0.177953,	 0.860873 ;
		 *   	 0.307838,	-0.168001 ];
		 * Ms  = ...
		 * [ 	 0.342299,	-0.233169,	-0.165028 ;
		 *   	-0.81607,	-0.168001,	 0.820642 ;
		 *   	 0.265277,	 0.982075,	-0.505922 ];
		 * \endcode
		 *
		 * which can be copied and directly pasted in Matlab.
		 *
		 * \ingroup jmath
		 */
		class MATLAB {

			public:
				static bool fullPrec;
				std::string str;
				friend std::ostream & operator <<(std::ostream & os, const MATLAB & m) {
					return os << m.str;
				}

				template<typename bubTemplateVector>
				void initFromBubVector(const bubTemplateVector& v1) {
					std::ostringstream os;
					os << "[ ";
					for (size_t i = 0; i < v1.size(); ++i) {
						{
							os << " ";
							double a = v1(i);
							os << a;
						}
						if (v1.size() != i + 1) {
							os << ", ";
						}
					}
					os << " ]';";
					str = os.str();
				}

				template<typename bubTemplateMatrix>
				void initFromBubMatrix(const bubTemplateMatrix& m1) {
					fullPrec = false;
					std::ostringstream os;
					os << "...\n[ ";
					std::ostringstream ostmp;
					for (size_t i = 0; i < m1.size1(); ++i) {
						ostmp << "\t";
						for (size_t j = 0; j < m1.size2(); ++j) {
							if (m1(i, j) < 0)
								ostmp << "-";
							else
								ostmp << " ";
							if (fullPrec || fabs(m1(i, j)) > 1e-6)
								ostmp << fabs(m1(i, j));
							else {
								ostmp << "0";
							}
							if (m1.size2() != j + 1) {
								ostmp << ",";
								const int size = ostmp.str().length();
								for (size_t i = size; i < 10; ++i)
									ostmp << " ";
								ostmp << "\t";
							}
							os << ostmp.str();
							ostmp.str("");
						}
						if (m1.size1() != i + 1) {
							os << " ;" << std::endl << "  ";
						}
						else {
							os << " ];";
						}
					}
					str = os.str();
				}

				template<typename bubTemplateIndex>
				void initFromBubIndex(const bubTemplateIndex & i1) {
					std::ostringstream os;
					os << "[ ";
					for (size_t i = 0; i < i1.size(); ++i) {
						os << (i1(i) + 1);
						if (i1.size() != i + 1) {
							os << ", ";
						}
					}
					os << " ];";
					str = os.str();
				}


				// Specialized templates for vectors

				template<typename T>
				MATLAB(const ublas::vector<T>& v1) {
					initFromBubVector(v1);
				}

				template<typename T, std::size_t N>
				MATLAB(const ublas::bounded_vector<T, N>& v1) {
					initFromBubVector(v1);
				}


				// Specialized templates for indirect arrays, ranges and slices

				MATLAB(const ublas::indirect_array<>& i1) {
					initFromBubIndex(i1);
				}

				MATLAB(const ublas::range& r1) {
					initFromBubIndex(jafar::jmath::ublasExtra::ia_range(r1));
				}

				MATLAB(const ublas::slice& s1) {
					initFromBubIndex(jafar::jmath::ublasExtra::ia_slice(s1));
				}


				// Specialized templates for vector proxies

				template<typename T>
				MATLAB(const ublas::vector_indirect<T>& v1) {
					initFromBubVector(v1);
				}

				template<typename T>
				MATLAB(const ublas::vector_range<T>& v1) {
					initFromBubVector(v1);
				}

				template<typename T>
				MATLAB(const ublas::vector_slice<T>& v1) {
					initFromBubVector(v1);
				}


				// Non-specialized templates will be interpreted as matrices.
				// These include all kinds of 2-dimensional arrays:

				template<class M>
				MATLAB(const M& m1) {
					initFromBubMatrix(m1);
				}

		};

	}
}

#endif /* MATLAB_HPP_ */
