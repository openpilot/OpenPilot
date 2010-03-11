/**
 * Matlab.hpp
 *
 *  Created on: 27/02/2010
 *      Author: jsola
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
						//DEBUGos <<  v1(i);
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
					os << "...\n [ ";
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

				template<typename T>
				MATLAB(const ublas::vector<T>& v1) {
					initFromBubVector(v1);
				}

				template<typename T, std::size_t N>
				MATLAB(const ublas::bounded_vector<T, N>& v1) {
					initFromBubVector(v1);
				}

				MATLAB(const ublas::indirect_array<>& i1) {
					initFromBubIndex(i1);
				}

				MATLAB(const ublas::range& r1) {
					initFromBubIndex(jafar::jmath::ublasExtra::ia_range(r1));
				}

				MATLAB(const ublas::slice& s1) {
					initFromBubIndex(jafar::jmath::ublasExtra::ia_slice(s1));
				}

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

				template<class M>
				MATLAB(const M& m1) {
					initFromBubMatrix(m1);
				}

		};

	}
}

#endif /* MATLAB_HPP_ */
