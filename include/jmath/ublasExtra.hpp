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
#include "boost/numeric/bindings/lapack/driver/gesdd.hpp"
#include "boost/numeric/bindings/ublas/matrix.hpp"
#include "boost/numeric/bindings/ublas/vector.hpp"
#endif
#endif

#include "boost/numeric/ublas/lu.hpp"
#include "boost/numeric/ublas/io.hpp"

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
				for (std::size_t i = 0; i < v.size(); i++) {
					v(i) = val_[i];
				}
			}

			/** Fills vector with a certain value
			 */
			template<class V>
			void fillVector(V& v, const double& value = 0.0) {
				for (std::size_t i = 0; i < v.size(); ++i)
					v(i) = value;
			}

			/** Fills vector with the contents of a c-array
			 */
			template<class V, typename W>
			void fillVector(V& out, const W* in) {
				for (std::size_t i = 0; i < out.size(); ++i)
					out(i) = in[i];
			}

			/** Creates a vector with the contents of a c-array
			 */
			template<int size, typename W>
			jblas::vec createVector(const W in[size]) {
				jblas::vec out(size);
				for (std::size_t i = 0; i < size; ++i)
					out(i) = in[i];
				return out;
			}

			/**
			 general subrange from jblas::vec or c array to jblas::vec
			 @param v the whole vector
			 @param pos block position
			 @param size block size
			*/
			template <class T>
			jblas::vec createVector(T const & v, size_t pos, size_t size)
			{
				jblas::vec res(size);

				for(size_t i = 0; i < size; ++i)
					res(i) = v[pos+i];

				return res;
			}

			/** Creates a vector with the contents of a c-array
			 */
			template<int size, typename W>
			jblas::sym_mat createSymMat(const W std[size]) {
				jblas::sym_mat out(size);
				out.clear();
				for (std::size_t i = 0; i < size; ++i)
					out(i,i) = std[i] * std[i];
				return out;
			}

			/**
			 Convert an aligned full symmetric matrix (jblas::vec or c array) to a sym_mat
			 @param v the vector that contains the upper triangle aligned
			 @param csize container size
			 @param pos block position
			 @param size block size
			*/
			template <class T>
			jblas::sym_mat createSymMat(T const & v, size_t shift, size_t csize, size_t pos, size_t size)
			{
				jblas::sym_mat m(size);

				for(size_t i = 0; i < size; ++i)
				{
					size_t k = shift + (csize)*(csize+1)/2 - (csize-(i+pos))*(csize-(i+pos)+1)/2;
					for(size_t j = i; j < size; ++j) m(i,j) = v[k+(j-i)];
				}

				return m;
			}

			/**
			 Convert an aligned full symmetric matrix (jblas::vec or c array) to a sym_mat
			 @param m the matrix to convert
			 @param v the vector that will contains the upper triangle aligned (needs to be allocated to the right size)
			 @param csize container size
			 @param pos block position
			 @param size block size
			*/
			template <class T>
			void explodeSymMat(jblas::sym_mat const & m, T & v, size_t shift, size_t csize, size_t pos, size_t size)
			{
				for(size_t i = 0; i < size; ++i)
				{
					size_t k = shift + (csize)*(csize+1)/2 - (csize-(i+pos))*(csize-(i+pos)+1)/2;
					for(size_t j = i; j < size; ++j) v[k+(j-i)] = m(i,j);
				}
			}


#define DECL_VEC(name,size,values) jblas::vec name(size); { const double tmp[size] = {values}; fillVector(name, tmp); }
#define INIT_VEC(size,values) fillVector2<size>((double[]){ values })


			template<class V>
			bool isZeroVector(const V& v) {
				bool isZero = true;
				for (std::size_t i = 0; i < v.size(); ++i) {
					if (v(i) != 0.0) {
						isZero = false;
						break;
					}
				}
				return isZero;
			}

			/**
			 * create diagonal matrix from scalar.
			 * The matrix is:
			 * - \a M(i,i) = \a val <-- diagonal
			 * - \a M(i,j) = 0  <-- off-diagonal
			 *
			 * \param size the size of the (square) matrix
			 * \param val the value to set to the diagonal elements
			 */
			template<typename T>
			ublas::diagonal_matrix<T> scalar_diag_mat(const size_t size, const T val) {
				jblas::diag_mat D(size, size);
				D.clear();
				for (size_t i = 0; i < size; i++)
					D(i, i) = val;
				return D;
			}

			/**
			 * Create diagonal matrix from vector.
			 * The matrix is:
			 * - \a M(i,i) = \a v(i) <-- diagonal
			 * - \a M(i,j) = 0  <-- off-diagonal
			 *
			 * \param v the vector with the values to set to the diagonal
			 */
			template<class V>
			jblas::diag_mat vec_diag_mat(const V & v) {
				jblas::diag_mat D(v.size(), v.size());
				D.clear();
				for (size_t i = 0; i < v.size(); i++)
					D(i, i) = v(i);
				return D;
			}



			/** normalize a vector.
			 */
			template<class V>
			void normalize(V& v) {
				double n = ublas::norm_2(v);
				JFR_NUMERIC(n > details::EPSILON,
						"ublasExtra::normalize: vector too small");
				v /= n;
			}

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
				lu_substitute<jblas::mat const, jblas::mat> (mLu, mLuInv);
				JFR_TRACE_END("ublasExtra::lu_inv");
				inv.assign(mLuInv);
			}
			/**
			 * Covariance transformation using jacobians (J*P*Jt)
			 * \param P the covariances matrix
			 * \param J the Jacobian matrix
			 * \return the transformed covariances matrix
			 */
			template<class SymMat>
			jblas::sym_mat prod_JPJt(const SymMat & P, const jblas::mat & J) {
				JFR_PRECOND((J.size2()==P.size1()) && (P.size2()==P.size1()),
						"ublasExtra::prod_JPJt: size mismatch.");
				return ublas::prod<jblas::sym_mat>(J, ublas::prod<jblas::mat>(P, ublas::trans(J)));
			}

			/**
			 * Normalized estimation error squared (x^T*P^-1*x)
			 * \param P the covariances matrix
			 * \param x the vector
			 * \return the NEES = x' * P^-1 * x
			 */
			template<class SymMat, class V>
			double prod_xt_P_x(const SymMat & P, const V & x) {
					JFR_PRECOND(x.size() == P.size1(),
							"ublasExtra::prod_xt_P_x: size mismatch.");
					jblas::sym_mat iP(P.size1());
					lu_inv(P, iP);
					return ublas::inner_prod(x, ublas::prod<jblas::vec>(iP, x));
			}

			/**
			 * Normalized estimation error squared (x^T*P^-1*x)
			 * \param iP the inverse of the covariances matrix
			 * \param x the vector
			 * \return the NEES = x' * iP * x
			 */
			template<class SymMat, class V>
			double prod_xt_iP_x(SymMat & iP, V & x) {
					JFR_PRECOND(x.size()==iP.size1(),
							"ublasExtra::prod_xt_iP_x: size mismatch.");
					return ublas::inner_prod(x, ublas::prod(iP, x));
			}

			/**
			 * Jacobian of normalize().
			 */
			template<class V, class M>
			void normalizeJac(V& v, M& J) {
				JFR_NUMERIC(ublas::norm_2(v) > details::EPSILON,
						"ublasExtra::normalizeJac: vector too small");
				JFR_PRECOND(J.size1() == v.size() && J.size2() == v.size(),
						"ublasExtra::normalizeJac: size of J is invalid");
				switch (v.size()) {
					case 2: {
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
						J(0, 0) = -t6 * t1 + t8;
						J(0, 1) = -t11;
						J(1, 0) = -t11;
						J(1, 1) = -t6 * t2 + t8;
						/* end maple copy/paste */
					}
						return;
					case 3: {
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
						J(0, 0) = -t7 * t1 + t9;
						J(0, 1) = -t12;
						J(0, 2) = -t13;
						J(1, 0) = -t12;
						J(1, 1) = -t7 * t2 + t9;
						J(1, 2) = -t17;
						J(2, 0) = -t13;
						J(2, 1) = -t17;
						J(2, 2) = -t7 * t3 + t9;
						/* end maple copy/paste */

					}
						return;
					default: {
						// We use the general formula:
						// VN_v = d(vn)/d(v) = (I*norm(v)^2 - v*v') / norm(v)^3
						double n2 = ublas::inner_prod(v, v); // norm square
						double n = sqrt(n2); // norm
						double n3 = n * n2; // norm cube
						jblas::identity_mat I(v.size());
						jblas::mat vvt(v.size(), v.size());
						vvt = ublas::outer_prod(v, v);
						J = (n2 * I - vvt) / n3;
						//						J = (n2 * ublas::identity_matrix(v.size()) - ublas::outer_prod(v, v)) / n3;
						return;
						//	  JFR_RUN_TIME("ublasExtra::normalizeJac: not implemented yet");
					}
				};
			}

			/**
			 * Compute the jacobian of inner product.
			 */
			template<std::size_t N, class Vec1, class Vec2, class Mat>
			void inner_prodJac(Vec1 const& u1, Vec2 const& u2, Mat& J) {
				JFR_PRECOND(u1.size() == N && u2.size() == N,
						"ublasExtra::inner_prodJac:" << N << "-vectors");
				JFR_PRECOND(J.size1() == 1 && J.size2() == 2*N,
						"ublasExtra::inner_prodJac:");
				for (std::size_t i = 0; i < N; ++i) {
					J(0, i) = u2(i);
					J(0, N + i) = u1(i);
				}
			}

			/** Compute the jacobian of norm_2.
			 */
			template<std::size_t N, class Vec, class Mat>
			void norm_2Jac(Vec const& u, Mat& J) {
				JFR_PRECOND(u.size() == N,
						"ublasExtra::norm_2Jac: " << N << "-vectors");
				JFR_PRECOND(J.size1() == 1 && J.size2() == N,
						"ublasExtra::norm_2Jac:");
				double d = 0;
				for (std::size_t i = 0; i < N; ++i) {
					d += pow(u(i), 2);
				}
				d = sqrt(d);
				for (std::size_t i = 0; i < N; ++i) {
					J(0, i) = u(i) / d;
				}
			}

			/** Compute the cross product of \a v1 and \a v2, result is
			 *  stored in \a vRes.
			 */
			template<class Vec1, class Vec2, class VecRes>
			void crossProd(Vec1 const& v1, Vec2 const& v2, VecRes& vRes) {
				JFR_PRECOND(v1.size()==3 && v2.size()==3 && vRes.size()==3,
						"ublasExtra::crossProd: 3D vector");

				vRes(0) = v1(1) * v2(2) - v1(2) * v2(1);
				vRes(1) = v1(2) * v2(0) - v1(0) * v2(2);
				vRes(2) = v1(0) * v2(1) - v1(1) * v2(0);
			}

			/** Compute the cross product of \a v1 and \a v2, the result is
			 *  returned.
			 */
			template<class Vec1, class Vec2>
			jblas::vec3 crossProd(Vec1 const& v1, Vec2 const& v2) {
				jblas::vec3 vRes;
				crossProd(v1, v2, vRes);
				return vRes;
			}

			/*
			 * Matrix
			 */

			/** Format a matrix output it to a string. Each matrix row
			 * corresponds to a line.
			 */
			template<class Mat>
			std::string prettyFormat(Mat const& m_) {
				std::stringstream s;
				for (std::size_t i = 0; i < m_.size1(); ++i) {
					for (std::size_t j = 0; j < m_.size2(); ++j) {
						s << m_(i, j) << "\t";
					}
					s << std::endl;
				}
				return s.str();
			}

			/*!
			 * Format a matrix output to a string in matlab syntax
			 */
			template<class Mat>
			std::string matlabFormat(Mat const& m_) {
				std::stringstream s;
				s << "[";
				for (std::size_t i = 0; i < m_.size1(); ++i) {
					for (std::size_t j = 0; j < m_.size2(); ++j) {
						if (j != (m_.size2() - 1))
							s << m_(i, j) << ",";
						else
							s << m_(i, j);
					}
					if (i != (m_.size1() - 1))
						s << ";";
					else
						s << "]";
				};

				return s.str();
			}

			template<class M>
			void setMatrixValue(M& m, const double* val_, std::size_t size1_, std::size_t size2_) {
				JFR_PRECOND(m.size1()==size1_ && m.size2()==size2_,
						"ublasExtra::setValue: size of m does not match");
				unsigned int k = 0;
				for (std::size_t i = 0; i < m.size1(); i++) {
					for (std::size_t j = 0; j < m.size2(); j++) {
						m(i, j) = val_[k];
						k++;
					}
				}
			}

			namespace details {

				template<class M>
				double det2(const M& m_) {
					return m_(0, 0) * m_(1, 1) - m_(1, 0) * m_(0, 1);
				}

				template<class M>
				double det3(const M& m_) {
					return m_(0, 0) * m_(1, 1) * m_(2, 2) + m_(0, 1) * m_(1, 2) * m_(2, 0) + m_(0, 2) * m_(1, 0) * m_(2, 1) - m_(
					    2, 0) * m_(1, 1) * m_(0, 2) - m_(2, 1) * m_(1, 2) * m_(0, 0) - m_(2, 2) * m_(1, 0) * m_(0, 1);
				}

				template<class M>
				void inv2(const M& m_, M& m_inv) {
					m_inv(0, 0) = m_(1, 1);
					m_inv(0, 1) = -1.0 * m_(0, 1);
					m_inv(1, 0) = -1.0 * m_(1, 0);
					m_inv(1, 1) = m_(0, 0);
					m_inv /= det2(m_);
				}

				template<class M>
				void inv3(const M& m_, M& m_inv) {
					m_inv(0, 0) = m_(1, 1) * m_(2, 2) - m_(1, 2) * m_(2, 1);
					m_inv(0, 1) = m_(2, 1) * m_(0, 2) - m_(2, 2) * m_(0, 1);
					m_inv(0, 2) = m_(0, 1) * m_(1, 2) - m_(0, 2) * m_(1, 1);
					m_inv(1, 0) = m_(2, 0) * m_(1, 2) - m_(1, 0) * m_(2, 2);
					m_inv(1, 1) = m_(0, 0) * m_(2, 2) - m_(2, 0) * m_(0, 2);
					m_inv(1, 2) = m_(1, 0) * m_(0, 2) - m_(0, 0) * m_(1, 2);
					m_inv(2, 0) = m_(1, 0) * m_(2, 1) - m_(2, 0) * m_(1, 1);
					m_inv(2, 1) = m_(2, 0) * m_(0, 1) - m_(0, 0) * m_(2, 1);
					m_inv(2, 2) = m_(0, 0) * m_(1, 1) - m_(1, 0) * m_(0, 1);

					m_inv /= det3(m_);
				}

			} // namespace details

#ifdef THALES_TAROT
#undef max
#endif

			/** Find maximum value of a matrix.
			 * \warning it returns a double whatever matrix type...
			 */
			template<class M>
			double max(const M& m_) {
				double max = m_(0, 0);
				for (std::size_t i = 0; i < m_.size1(); i++) {
					for (std::size_t j = 0; j < m_.size2(); j++) {
						if (m_(i, j) > max) {
							max = m_(i, j);
						}
					}
				}
				return max;
			}

			/** Find maximum value of a matrix.
			 * \warning it returns a double whatever matrix type...
			 */
			template<class V>
			double maxV(const V& v_) {
				double max = v_(0);
				for (std::size_t i = 1; i < v_.size(); i++) {
					if (v_(i) > max) {
						max = v_(i);
					}
				}
				return max;
			}
			/** Computes the trace of a matrix.
			 * \warning it returns a double whatever matrix type...
			 */
			template<class M>
			double trace(const M& m_) {
				JFR_PRECOND(m_.size1() == m_.size2(),
						"ublasExtra::trace: m_ must be square");
				double t = 0;
				for (std::size_t i = 0; i < m_.size1(); i++) {
					t += m_(i, i);
				}
				return t;
			}

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

				for (std::size_t i = 0; i < pivots.size(); ++i) {
					if (pivots(i) != i)
						det *= -1.0;
					det *= mLu(i, i);
				}
				return det;
			}

			template<class M>
			double det(const M& m_) {
				JFR_PRECOND(m_.size1() == m_.size2(),
						"ublasExtra::det: m_ must be a square matrix");
				switch (m_.size1()) {
					case 1:
						return m_(0, 0);
					case 2:
						return details::det2(m_);
					case 3:
						return details::det3(m_);
					default:
						return lu_det(m_);
				}
			}

			template<class M>
			void inv(const M& m_, M& m_inv) {
				JFR_PRECOND(m_.size1() == m_.size2(),
						"ublasExtra::inv: m_ must be a square matrix");
				switch (m_.size1()) {
					case 1:
						m_inv(0, 0) = 1 / m_(0, 0);
						break;
					case 2:
						details::inv2(m_, m_inv);
						break;
					case 3:
						details::inv3(m_, m_inv);
						break;
					default:
						lu_inv(m_, m_inv);
				}
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
				jblas::mat_column_major U(m.size1(), m.size2());
				jblas::mat_column_major VT(m.size1(), m.size2());
				int ierr = boost::numeric::bindings::lapack::gesdd('A',working_m, s, U, VT);
				if (ierr != 0) {
					throw(jmath::LapackException(ierr, "LinearLeastSquares::solve: error in lapack::gesdd() routine", __FILE__,
					    __LINE__));
				}
				jblas::mat S(jblas::zero_mat(s.size(), s.size()));
				for (unsigned int i = 0; i < s.size(); i++) {
					JFR_ASSERT(s(i)!=0, "ublasExtra::svd_inv: singular matrix");
					S(i, i) = 1 / s(i);
				}
				inv = ublas::prod(S, ublas::trans(U));
				inv = ublas::prod(ublas::trans(VT), inv);
			}

#endif
#endif 


			/**
			 * @return the eigen values of a 2x2 matrix
			 */
			inline jblas::vec2 eigenValues(const jblas::mat22& m) {
				double tr = m(0, 0) + m(1, 1);
				double diff = m(0, 0) - m(1, 1);
				double sq = sqrt(4 * m(0, 1) * m(1, 0) + diff * diff);
				jblas::vec2 v;
				v(0) = 0.5 * (tr + sq);
				v(1) = 0.5 * (tr - sq);
				return v;
			}

			/** computes per row M minus v and store it in M_v
			 * @return M_v
			 * @TODO: move it into a matrix vector expression like ublas::prod
			 */
			template<typename T>
			void minus_vector(const ublas::matrix<T> &M, const ublas::vector<T> &v, 
												ublas::matrix<T> &M_v)
			{
				size_t n_cols;
				size_t n_rows;
				JFR_ASSERT(((n_rows = M.size1()) == M_v.size1()) && 
									 ((n_cols = M.size2()) == M_v.size2()),
									 "M and M_v must be same size");
				JFR_ASSERT(n_cols == v.size(),
									 "vector size and matrix coulmns size must be equal");
				for(size_t r = 0; r < n_rows; r++) 
					for(size_t c = 0; c < n_cols; c++) 
						M_v(r, c) = M(r,c) - v[c];
			}
			
			/** computes per row M minus v and store it in M
			 * @TODO: move it into a matrix vector expression like ublas::prod
			 */
			template<typename T>
			void minus_vector(ublas::matrix<T> &M, const ublas::vector<T> &v)
			{
				size_t n_cols;
				JFR_ASSERT((n_cols = M.size2()) == v.size(),
									 "vector size and matrix coulmns size must be equal");
				size_t n_rows = M.size1();
				for(size_t r = 0; r < n_rows; r++) 
					for(size_t c = 0; c < n_cols; c++) 
						M(r, c)-= v[c];
			}

			/** computes per row M plus v and store it in M
			 * @TODO: move it into a matrix vector expression like ublas::prod
			 */
			template<typename T>
			void plus_vector(ublas::matrix<T> &M, const ublas::vector<T> &v)
			{
				JFR_ASSERT(M.size2() == v.size(),
									 "vector size and matrix coulmns size must be equal");
				for(size_t r = 0; r < M.size1(); r++) 
					for(size_t c = 0; c < M.size2(); c++) 
						M(r, c)+= v[c];
			}

			/** deletes a matrix M(nxm) row in a memory efficient way i.e. no extra memory
			 *  is allocated.
			 * @return M with size n-1xm.
			 */
			template<typename T>
			void delete_row(ublas::matrix<T> &M, size_t index) 
			{
				JFR_ASSERT((index < M.size1() && (index>=0)),
									 "index must be in [0.."<<M.size1()<<"[ range");
				if (index == M.size1() -1)
					M.resize(index, M.size2(), true);
				else {
					for(size_t row_counter = index+1; row_counter < M.size1();row_counter++)
						row(M,row_counter-1) = row(M,row_counter);
					M.resize(M.size1() - 1, M.size2(), true);
				}
			}

			/** deletes a matrix M(nxm) column in a memory efficient way i.e. no extra memory
			 *  is allocated.
			 * @return M with size nxm-1.
			 */
			template<typename T>
			void delete_column(ublas::matrix<T> &M, size_t index) 
			{
				JFR_ASSERT((index < M.size2() && (index>=0)),
									 "index must be in [0.."<<M.size2()<<"[ range");
				if (index == M.size2() - 1)
					M.resize(M.size1(), index, true);
				else {
					for(size_t col_counter = index+1; col_counter < M.size1();col_counter++)
						column(M,col_counter-1) = column(M,col_counter);
					M.resize(M.size1() , M.size2() - 1, true);
				}
			}
			/** deletes a list of column indices stored in @ref indices from @ref M
			 * @param indices: vector of indices considered
			 * @param is_sorted: are the indices sorted into an acending order?
			 * @return M with size nxm-(indices.size()).
			 */
			template<typename T>
			void delete_columns(ublas::matrix<T> &M, 
													std::vector<size_t> indices, 
													bool is_sorted = false) 
			{
				JFR_ASSERT(((indices.size() <= M.size2()) && (indices.size() > 0)),
									 "indices size is "<<indices.size()<<
									 " whereas M columns size is "<<M.size2());
				if(!is_sorted)
					std::sort(indices.begin(), indices.end());
				for(std::vector<size_t>::reverse_iterator rit = indices.rbegin();
						rit != indices.rend();
						++rit)
					delete_column(M, *rit);
			}
			/** deletes a list of row indices stored in @ref indices from @ref M
			 * @param indices: vector of indices considered
			 * @param is_sorted: are the indices sorted into an acending order?
			 * @return M with size nxm-(indices.size()).
			 */
			template<typename T>
			void delete_rows(ublas::matrix<T> &M, 
											 std::vector<size_t> indices, 
											 bool is_sorted = false) 
			{
				JFR_ASSERT(((indices.size() <= M.size2()) && (indices.size() > 0)),
									 "indices size is "<<indices.size()<<
									 " whereas M rows size is "<<M.size2());
				if(!is_sorted)
					std::sort(indices.begin(), indices.end());
				for(std::vector<size_t>::reverse_iterator rit = indices.rbegin();
						rit != indices.rend();
						++rit)
					delete_row(M, *rit);
			}
		} // namespace ublasExtra
	/*\@}*/

	} // namespace jmath
} // namespace jafar

#endif // JMATH_UBLAS_EXTRA_HPP
