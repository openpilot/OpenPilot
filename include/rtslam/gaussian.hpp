/*
 * gaussian.hpp
 *
 *  Created on: 13/02/2010
 *      Author: jsola
 */

/**
 * \file gaussian.hpp
 * File defining the gaussian classes
 * \ingroup rtslam
 */

#ifndef GAUSSIAN_HPP_
#define GAUSSIAN_HPP_

#include <jmath/jblas.hpp>
//#include "jmath/ublasExtra.hpp"
#include "jmath/indirectArray.hpp"

namespace jafar {
	namespace rtslam {

		/**
		 * Storage type of Gaussians: LOCAL, REMOTE.
		 */
		typedef enum {
			GAUSSIAN_LOCAL, GAUSSIAN_REMOTE
		} storage_t;

		/**
		 * Class for indirect Gaussians
		 * \ingroup rtslam
		 */
		class Gaussian {
			private:
				storage_t storage_; ///< select local or remote storage
				bool hascov_; ///< false if covariance is exactly null
				std::size_t size_; ///< size of data
				jblas::vec x_local; ///< local storage mean
				jblas::sym_mat P_local; ///< local storage covariance
				jblas::ind_array ia_; ///< indirect array of indices to storage
				jblas::vec_indirect x_; ///< indexed mean
				jblas::sym_mat_indirect P_; ///< indexed covariance

			public:

				/**
				 * Empty constructor
				 */
				inline Gaussian() :
					storage_(GAUSSIAN_LOCAL), hascov_(false), size_(0), x_local(0), P_local(0), ia_(0), x_(x_local, 0), P_(P_local, 0, 0) {
				}

				/**
				 * Copy constructor.
				 */
				inline Gaussian(const Gaussian & G) :
					storage_(G.storage_), hascov_(G.hascov_), size_(G.size_), x_local(G.x_local), P_local(G.P_local), ia_(G.ia_), x_(G.x_), P_(G.P_) {
					//FIXME P_(G.P_) and same for x_. Maybe forbid by declaring it private.
				}

				/**
				 * Local constructor.
				 * This constructor defines a local-storage Gaussian of size \a _size.
				 * \param _size the size of the Gaussian.
				 */
				inline Gaussian(const size_t _size) :
					storage_(GAUSSIAN_LOCAL), hascov_(false), size_(_size), x_local(size_), P_local(size_, size_), ia_(size_), x_(x_local,
					    ia_.all()), P_(P_local, ia_.all(), ia_.all()) {
				}

				/**
				 * Local constructor from mean data, no covariance.
				 * This constructor stores the input information in the local storage \a x_local.
				 * This data is accessed through \a x_.
				 * \param _x the Gaussian mean.
				 */
				inline Gaussian(const jblas::vec & _x) :
					storage_(GAUSSIAN_LOCAL), hascov_(false), size_(_x.size()), x_local(_x), P_local(size_, size_), ia_(size_), x_(x_local,
					    ia_.all()), P_(P_local, ia_.all(), ia_.all()) {
					hascov_ = false;
				}

				/**
				 * Local constructor from full data.
				 * This constructor stores the input information in the local storage \a x_local and \a P_local.
				 * This data is accessed through \a x_ and \a P_.
				 * \param _x the Gaussian mean.
				 * \param _P the Gaussian covariances matrix.
				 */
				inline Gaussian(const jblas::vec& _x, const jblas::sym_mat& _P) :
					storage_(GAUSSIAN_LOCAL), hascov_(true), size_(_x.size()), x_local(_x), P_local(_P), ia_(size_), x_(x_local, ia_.all()), P_(P_local,
					    ia_.all(), ia_.all()) {
				}

				/**
				 * Remote constructor.
				 * This constructor uses indirect indexing onto an remote Gaussian provided to the constructor.
				 * The indirect array where this new Gaussian points to in the old one is also given as input.
				 * For practical use, the result is such that the new Gaussian Gnew has <i> x_ = G.x_(ia_) </i> and <i> P_ = G.P_(ia_,ia_).</i>
				 * The local storage \a x_ and \a P_ is kept at null size for economy.
				 * \param G the remote Gaussian.
				 * \param _ia the indirect array of indices pointing to G.x_ and G.P_,
				 * such that <i> x_ = G.x_(ia_) </i> and <i> P_ = G.P_(ia_,ia_).</i>
				 * TODO: coment that G must be local, or add support for remote G.
				 */
				inline Gaussian(Gaussian & G, const jblas::ind_array & _ia) :
					storage_(GAUSSIAN_REMOTE), hascov_(true), size_(_ia.size()), x_local(0), P_local(0), ia_(_ia), x_(G.x_local, ia_), P_(G.P_local, ia_,
					    ia_) {
				}

				/**
				 * Remote constructor from data.
				 * This constructor uses indirect indexing onto a remote pair {_x, _P} provided to the constructor.
				 * The indirect array where this new Gaussian points to in {_x, _P} is also given as input.
				 * For practical use, the result is such that the new Gaussian Gnew has <i> this.x_ = x_local(ia_) </i> and <i> this.P_ = P_local(ia_,ia_).</i>
				 * The local storage \a x_ and \a P_ is kept at null size for economy.
				 * \param _x the remote mean vector.
				 * \param _P the remote covariances matrix.
				 * \param _ia the indirect array of indices pointing to _x and _P,
				 * such that <i> this.x_ = x_local(ia_) </i> and <i> this.P_ = P_local(ia_,ia_).</i>
				 */
				inline Gaussian(jblas::vec & _x, jblas::sym_mat & _P, const jblas::ind_array& _ia) :
					storage_(GAUSSIAN_REMOTE), hascov_(true), size_(_ia.size()), x_local(0), P_local(0), ia_(_ia), x_(_x, ia_), P_(_P, ia_, ia_) {
					//			JFR_ASSERT((_x.size() == _ia.size()) && (_x.size() == _P.size1()), "gaussian.hpp: Gaussian(): sizes mismatch.");
				}

				inline storage_t storage() {
					return storage_;
				}

				inline size_t size() {
					return size_;
				}

				inline bool hasCov(void) {
					return hascov_;
				}

				inline void setHasCov(bool _hascov = true) {
					hascov_ = _hascov;
				}

				/**
				 * Clear data, keep sizes and ranges.
				 * Clears the data of \a x_ and \a P_.
				 */
				inline void clear(void) {
					x_.assign(jblas::zero_vec(size_));
					P_.assign(jblas::zero_mat(size_, size_));
				}

				/*
				 * Accessors
				 */
				inline void storage(const storage_t & _s) {
					storage_ = _s;
					if (_s == GAUSSIAN_REMOTE)
						hascov_ = true;
				}
				inline void size(const std::size_t & _size) {
					size_ = _size;
				}
				inline void ia(const jblas::ind_array & _ia);
				inline void x(const jblas::vec & _x) {
					JFR_ASSERT(_x.size() == size_, "gaussian.hpp: set_x: size mismatch.");
					x_.assign(_x);
				}

				/**
				 * Set covariances matrix from standard deviations vector.
				 * \param the vector of standard deviations.
				 */
				inline void P(const jblas::vec& _std) {
					JFR_ASSERT(_std.size() == P_.size1(), "gaussian.hpp: set_P: size mismatch.");
					hascov_ = true;
					P_.assign(jblas::zero_mat(size_));
					for (std::size_t i = 0; i < size_; i++) {
						P_(i, i) = _std(i) * _std(i);
					}
				}

				/**
				 * Set covariances matrix from a covariances matrix.
				 * \param _P the covariances matrix.
				 */
				inline void P(const jblas::sym_mat & _P) {
					JFR_ASSERT(_P.size1() == size_, "gaussian.hpp: set_P: size mismatch.");
					hascov_ = true;
					P_.assign(_P);
				}

				/**
				 * Set off-diagonal block in the covariances matrix.
				 * Only tested for local Gaussians.
				 * \param M the off-diagonal block.
				 * \param ia1 the indirect array for rows.
				 * \param ia2 the indirect array for columns.
				 */
				inline void P(const jblas::mat & M, const jblas::ind_array & ia1, const jblas::ind_array & ia2) {
					hascov_ = true;
					project(P_local, ia1, ia2) = M;
				}

				inline jblas::vec_indirect & x() {
					return x_;
				}

				inline jblas::sym_mat_indirect & P() {
					return P_;
				}

				inline jblas::ind_array & ia() {
					return ia_;
				}

				/**
				 * Operator << for class Gaussian.
				 * It shows different information depending on the Gaussian having local or remote storage.
				 * For local storage, the mean and covariance data is shown.
				 * For remote storage, the size of the remote Gaussian is also shown, and the indirect array too.
				 */
				friend std::ostream& operator <<(std::ostream & s, jafar::rtslam::Gaussian & g_) {

					if (g_.storage() == GAUSSIAN_LOCAL) {
						if (g_.hasCov()) {
							s << "Gaussian with local storage: \n";
							s << " .x : " << g_.x_ << "\n";
							s << " .P : " << g_.P_;
						}
						else { // No covariance
							s << "Gaussian with local storage and null covariance: \n";
							s << " .x : " << g_.x_ << "\n";
						}
					}
					else {
						size_t sz = g_.x_.data().size();
						s << "Gaussian with remote storage: \n";
						s << " .rm->[" << sz << "](...data not shown...)\n";
						s << " .ia: " << g_.ia_ << "\n";
						s << " .x : " << g_.x_ << "\n";
						s << " .P : " << g_.P_;
					}
					return s;
				}

		};

	}
}

#endif /* GAUSSIAN_HPP_ */
