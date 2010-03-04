/**
 * ublasExtra.cpp
 *
 *  Created on: 03/03/2010
 *      Author: jsola
 *
 *  \file ublasExtra.cpp
 *
 *  ## Add a description here ##
 *
 * \ingroup jmath
 */

#include "jmath/ublasExtra.hpp"

namespace jafar {
	namespace jmath {

		namespace ublasExtra {

			/*
			 * Covariance transformation using jacobians (J*P*Jt)
			 */
			jblas::sym_mat prod_JPJt(jblas::sym_mat & P, jblas::mat & J) {
				JFR_PRECOND(J.size2()==P.size1(),
						"ublasExtra::prod_JPJt: size mismatch.");
				return ublas::prod<jblas::sym_mat>(J, ublas::prod<jblas::mat>(P, ublas::trans(J)));
			}

			/*
			 * Normalized estimation error squared (x^T*P^-1*x)
			 */
			double prod_xt_P_x(jblas::sym_mat & P, jblas::vec & x) {
				JFR_PRECOND(x.size() == P.size1(),
						"ublasExtra::prod_xt_P_x: size mismatch.");
				jblas::sym_mat iP(P.size1());
				lu_inv(P, iP);
				return ublas::inner_prod(x, ublas::prod<jblas::vec>(iP, x));
			}

			/*
			 * Normalized estimation error squared (x^T*P^-1*x)
			 */
			double prod_xt_iP_x(jblas::sym_mat & iP, jblas::vec & x) {
				JFR_PRECOND(x.size()==iP.size1(),
						"ublasExtra::prod_xt_iP_x: size mismatch.");
				return ublas::inner_prod(x, ublas::prod(iP, x));
			}

		}
	}
}
