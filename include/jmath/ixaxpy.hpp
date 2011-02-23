/**
 * ixaxpy.hpp
 *
 *  Created on: 27/02/2010
 *      Author: jsola
 *
 *  \file ixaxpy.hpp
 *
 *  ## Add a description here ##
 *
 * \ingroup jmath
 */

#ifndef IXAXPY_HPP_
#define IXAXPY_HPP_

#include "jmath/jblas.hpp"
#include "jmath/indirectArray.hpp"

namespace jafar {
	namespace jmath {
		namespace ublasExtra {

			using namespace std;
			using namespace jblas;
			using namespace boost::numeric::ublas;


			////////////////////////////////////
			// SPARSE SYMMETRIC MATRIX PRODUCT
			////////////////////////////////////
			template<class bubMatrix>
			void ixaxpy_offdiag_subprod(sym_mat & S, const ind_array & ia_invariant, const bubMatrix & J_oi,
			    const ind_array & ia_in, const ind_array & ia_out)
			{
				project(S, ia_out, ia_invariant) = prod(J_oi, project(S, ia_in, ia_invariant));
			}

			template<class bubMatrix, class bubMatrixY>
			void ixaxpy_diag_subprod(sym_mat & S, const bubMatrix & J, const ind_array & ia_in, const ind_array & ia_out,
			    const bubMatrixY Y)
			{
				matrix<double> JSii = prod(J, project(S, ia_in, ia_in));
				ublas::noalias(project(S, ia_out, ia_out)) = prod<sym_mat> (JSii, trans(J));
				project(S, ia_out, ia_out) += Y;
			}

			template<class bubMatrix>
			void ixaxpy_diag_subprod(sym_mat & S, const bubMatrix & J, const ind_array & ia_in, const ind_array & ia_out)
			{
				matrix<double> JSii = prod(J, project(S, ia_in, ia_in));
				ublas::noalias(project(S, ia_out, ia_out)) = prod<sym_mat> (JSii, trans(J));
			}


			/**
			 * Symmetric, sparse matrix product with indirect indexing.
			 * This function performs the product X = A*X*A' + Y where:\n
			 * - X is a symmetric matrix
			 * having the role of a covariances matrix of a certain vector \a x, \n
			 * - A is a sparse Jacobian matrix, A=[A_oi 0 0; 0 I 0;0 0 0],
			 * which maps the elements of x indexed by indirect arrays \a ia_in into
			 * the elements indexed by \a ia_out, that is:\n
			 *   - x(ia_out) = A_oi * x(ia_in).\n
			 * - Y_oo is a symmetric matrix that is added to the ia_out covariance. We have Y = [Y_oo 0;0 0]
			 *
			 * Due to the sparsity of the problem and to the existence of identity blocks in the Jacobian A,
			 * a significant part of the symmetric matrix X is unchanged. The indices to these unchanged entries
			 * must be given to the function via \a ia_invariant.
			 *
			 * To explain the operation, one has to consider four parts in the storage state vector, x = [s, n] = [in, out, inv, n], where:
			 *  - \a s is the used part of x.
			 *  - \a in is the input part of \a s (of \a x) affected by \a A_oi.
			 *  - \a out is the output part of \a s (of \a x) affected by \a A_oi.
			 *  - \a inv is the part of \a s (of \a x) not affected by \a A_oi.
			 *  - \a n is the part of \a x that is not used at all, and it only serves as reserved storage.
			 *
			 * We index each part with indirect arrays, ia_x = [ia_s, ia_n] = [ia_in, ia_out, ia_inv, ia_n], where:\n
			 *  - \a ia_x: the full indirect array pointing to all terms of x. x(ia_x) is the same as x(:) in Matlab.
			 * 	- \a ia_s: the part of used states in the vector. x(ia_s) = s.\n
			 * 	- \a ia_in: the part of input states to the Jacobian. x(ia_in) = in;\n
			 * 	- \a ia_out: the part of output states from the Jacobian. x(ia_out) = out;\n
			 * 	- \a ia_inv: the part of states not affected by the product. x(ia_inv) = inv.\n
			 * 	- \a ia_n: the part of non-used states (ignored, not used, just here to explain). x(ia_n) = n.\n
			 *
			 * Then:\n
			 * 	- X(ia_out,ia_out) = A_oi * X(ia_in,ia_in) * A_oi' + Y \n
			 *  - X(ia_out,ia_inv) = A_oi* X(ia_in,ia_inv)\n
			 *  - X(ia_inv,ia_out) is not computed as the matrix X is symmetric.\n
			 *  - X(ia_inv,ia_inv) is unchanged\n
			 *  - X(ia_n,ia_x) and its transpose X(ia_x,ia_n) are ignored.
			 *
			 * \param X a storage covariances matrix
			 * \param ia_invariant the indices in \a x of invariant states
			 * \param A_oi a Jacobian matrix maping \a x(in) into \a x(out)=A_oi*x(in)
			 * \param ia_in the indices in \a x that are to be affected by \a A_oi.
			 * \param ia_out the indices in \a x resulting from the \a A_oi mapping
			 * \param Y a symmetric matrix to be added to the output square block \a X(ia_out,ia_out)
			 */
			template<class bubMatrixJ, class bubMatrixY>
			void ixaxpy_prod(sym_mat & X, const ind_array & ia_invariant, const bubMatrixJ & A_oi, const ind_array & ia_in,
			    const ind_array & ia_out, const bubMatrixY Y_oo)
			{
				ixaxpy_offdiag_subprod(X, ia_invariant, A_oi, ia_in, ia_out);
				ixaxpy_diag_subprod(X, A_oi, ia_in, ia_out, Y_oo);
			}

			/**
			 * Symmetric, sparse matrix product with indirect indexing.
			 * This function performs the product X = A*X*A' when X is a symmetric matrix and A is sparse.
			 *
			 * \sa See the overloaded companion for more detailed information.
			 */
			template<class bubMatrixJ>
			void ixaxpy_prod(sym_mat & X, const ind_array & ia_invariant, const bubMatrixJ & A_oi, const ind_array & ia_in,
			    const ind_array & ia_out)
			{
				ixaxpy_offdiag_subprod(X, ia_invariant, A_oi, ia_in, ia_out);
				ixaxpy_diag_subprod(X, A_oi, ia_in, ia_out);
			}

		}
	}
}

#endif /* IXAXPY_HPP_ */
