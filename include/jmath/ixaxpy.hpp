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

			//			template<class bubMatrix>
			//			symmetric_adaptor<matrix_indirect<bubMatrix> > projectSym(bubMatrix & M, const ind_array & ia) {
			//				matrix_indirect<bubMatrix> Mrr(M, ia, ia);
			//				symmetric_adaptor<matrix_indirect<bubMatrix> > Srr(Mrr);
			//				return Srr;
			//			}

			//////////////////////////
			// HERE WITH SYMMETRIC ADAPTORS
			template<class bubMatrix1, class bubMatrix2>
			void ixaxpy_offdiag_subprod(symmetric_adaptor<bubMatrix1> S, const ind_array & iax, const bubMatrix2 & Hr,
			    const ind_array & iar) {
				size_t sr = iar.size();
				size_t ss = iax.size();
				size_t sm = ss - sr;
				ind_array iac(sm);
				iac = ia_complement(iax, iar);
				project(S, iar, iac) = prod(Hr, project(S, iar, iac));
			}

			template<class bubMatrix1, class bubMatrix2>
			void ixaxpy_diag_subprod(symmetric_adaptor<bubMatrix1> S, const bubMatrix2 & Hr, const ind_array & iar) {
				matrix<double> HrSrr = prod(Hr, project(S, iar, iar));
				project(S, iar, iar) = prod(HrSrr, trans(Hr));
			}

			/**
			 * Quadratic sym_matrix product with square Jacobian, indirectly indexed.
			 * See documentation in overloaded function
			 */
			template<class bubMatrix1, class bubMatrix2>
			void ixaxpy_prod(symmetric_adaptor<bubMatrix1> S, const ind_array & iax, const bubMatrix2 & Hr,
			    const ind_array & iar) {
				ixaxpy_offdiag_subprod(S, iax, Hr, iar);
				ixaxpy_diag_subprod(S, Hr, iar);
			}

			///////////////////////
			// HERE WITH ALL SYMMETRIC - EVEN THE SUBPRODS
			template<class bubMatrix>
			void ixaxpy_offdiag_subprod(sym_mat & S, const ind_array & iax, const bubMatrix & Hr, const ind_array & iar) {
				ind_array iac = ia_complement(iax, iar);
				project(S, iar, iac) = prod(Hr, project(S, iar, iac));
			}

			template<class bubMatrix>
			void ixaxpy_diag_subprod(sym_mat & S, const bubMatrix & Hr, const ind_array & iar) {
				matrix<double> HrSrr = prod(Hr, project(S, iar, iar));
				project(S, iar, iar) = prod<sym_mat> (HrSrr, trans(Hr));
			}

			/**
			 * Quadratic sym_matrix product with square Jacobian, indirectly indexed.
			 * This function performs the product H*P*H' when P is a symmetric matrix
			 * having the role of a covariances matrix, and H is a sparse Jacobian, H=[Hr 0; 0 I],
			 * with \a Hr square and the elements of \a Hr indexed by indirect array \a iar.
			 * The input symmetric matrix S is indexed by indices iax so that P = S(iax,iax).
			 *
			 * To explain the operation, one has to consider three parts in the storage state vector, s = [x, n] = [r, c, n], where:
			 *  - \a x is the used part of s.
			 *  - \a r is the part of \a s (of \a x) affected by \a Hr.
			 *  - \a c is the part of \a s (of \a x) not affected by \a Hr.
			 *  - \a n is the part of \a s that is not used at all, and it only serves as reserved storage.
			 *
			 * We index each part with an indirect array, ias = [iax, ian] = [iar, iac, ian], where:\n
			 *  - \a ias: the full indirect array pointing to all terms of s. s(ias) is the same as s(:) in Matlab.
			 * 	- \a iax: the part of used states in the vector. s(iax) = x.\n
			 * 	- \a ian: the part of non-used states (ignored, not used, just here to explain). s(ian) = n.\n
			 * 	- \a iar: the part of non-null Jacobian. s(iar) = r;\n
			 * 	- \a iac: the complement of \a iar in \a iax (coputed internally with iac = ia_complement(iax,iar)). s(iac) = c.\n
			 *
			 * Having understood the partition of s, the storage covariances matrix S could be partitioned as:
			 *  - S = [Sxx Sxn; Snx Snn] = [Srr Src Srn ; Scr Scc Scn ; Snr Snc Snn], with Sxx = P, the used block.
			 *
			 * Then:\n
			 * 	- S(iar,iar) = Hr * S(iar,iar) * Hr'\n
			 *  - S(iar,iac) = Hr * S(iar,iac)\n
			 *  - S(iac,iar) is not computed as the matrix S is symmetric.\n
			 *  - S(iac,iac) is unchanged\n
			 *  - S(ian,ias) and its transpose S(ias,ian) are ignored.
			 *
			 * \param S a storage covariances matrix
			 * \param iax the indices in S of used states
			 * \param Hr a square Jacobian matrix
			 * \param iar the indices in S that are to be affected by Hr.
			 */
			template<class bubMatrix>
			void ixaxpy_prod(sym_mat & S, const ind_array & iax, const bubMatrix & Hr,
			    const ind_array & iar) {
				ixaxpy_offdiag_subprod(S, iax, Hr, iar);
				ixaxpy_diag_subprod(S, Hr, iar);
			}


			///////////////////////
			// HERE WITH ALL SYMMETRIC - DIFFERENT IN AND OUT INDICES - AND ADDED COVARIANCE
			template<class bubMatrix>
			void ixaxpy_offdiag_subprod(sym_mat & S, const ind_array & ia_x, const bubMatrix & J_oi, const ind_array & ia_in, const ind_array & ia_out){
					ind_array iac = ia_complement(ia_x, ia_out);
					project(S, ia_out, iac) = prod(J_oi, project(S, ia_in, iac));
			}

			template<class bubMatrix, class bubMatrixY>
			void ixaxpy_diag_subprod(sym_mat & S, const bubMatrix & J, const ind_array & ia_in, const ind_array & ia_out, const bubMatrixY Y) {
					matrix<double> JSii = prod(J, project(S, ia_in, ia_in));
					project(S, ia_out, ia_out) = prod<sym_mat> (JSii, trans(J));
					project(S, ia_out, ia_out) += Y;
			}

			template<class bubMatrix>
			void ixaxpy_diag_subprod(sym_mat & S, const bubMatrix & J, const ind_array & ia_in, const ind_array & ia_out) {
					matrix<double> JSii = prod(J, project(S, ia_in, ia_in));
					project(S, ia_out, ia_out) = prod<sym_mat> (JSii, trans(J));
			}

			template<class bubMatrixJ, class bubMatrixY>
			void ixaxpy_prod(sym_mat & S, const ind_array & ia_x, const bubMatrixJ & J, const ind_array & ia_in, const ind_array & ia_out, const bubMatrixY Y){
					ixaxpy_offdiag_subprod(S, ia_x, J, ia_in, ia_out);
					ixaxpy_diag_subprod(S, J, ia_in, ia_out, Y);
			}

			template<class bubMatrixJ>
			void ixaxpy_prod(sym_mat & S, const ind_array & ia_x, const bubMatrixJ & J, const ind_array & ia_in, const ind_array & ia_out){
					ixaxpy_offdiag_subprod(S, ia_x, J, ia_in, ia_out);
					ixaxpy_diag_subprod(S, J, ia_in, ia_out);
			}

		}
	}
}

#endif /* IXAXPY_HPP_ */
