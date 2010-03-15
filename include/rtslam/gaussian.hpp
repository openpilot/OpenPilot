/*
 * gaussian.hpp
 *
 *  Created on: 13/02/2010
 *      Author: jsola, croussil
 */

/**
 * \file gaussian.hpp
 * File defining the gaussian classes
 * \ingroup rtslam
 */

#ifndef GAUSSIAN_HPP_
#define GAUSSIAN_HPP_

#include <jmath/jblas.hpp>
#include "jmath/indirectArray.hpp"
#include "rtslam/rtslamException.hpp"

namespace jafar {
	namespace rtslam {


		/////////////////////////
		//    CLASS Gaussian
		/////////////////////////
		/**
		 * Class for Gaussians having LOCAL or REMOTE storage.
		 * \author jsola@laas.fr & croussil@laas.fr
		 *
		 * This class defines a multi-dimensional Gaussian variable.
		 * It allows mean and covariances data to be stored locally or remotelly, depending on the constructor
		 * (the storage modality is selectable only at construction time).
		 * Remote Gaussians are indexed with indirect arrays.
		 *
		 * - Mean and covariances are accessed through \a x() and \a P().\n
		 * - The indirect array is part of the Gaussian class, and is accessible through \a ia().\n
		 *
		 * <b> Defining a local Gaussian </b>
		 *
		 * You just give a size, a pair {\a x , \a P } or another Gaussian:
		 * \code
		 * Gaussian GLs(7);                // Construct from size
		 * Gaussian GLa(x, P);             // Construct from given vector and matrix. these are copied to the local storage.
		 * Gaussian GLb(GLa);              // Copy-construct from local Gaussian.
		 * Gaussian GLc(GRa, GRa.LOCAL);   // Copy-construct from remote Gaussian but force local storage.
		 * \endcode
		 *
		 * see that the last constructor takes a remote Gaussian (see below). The directive \a GRa.LOCAL tells the constructor to force local storage.
		 * The result is a local Gaussian (the data is copied to the local storage).
		 *
		 * You can create Gaussians with no covariance by providing just the mean vector. This is only possible for local Gaussians:
		 * \code
		 * Gaussian GLx(x)
		 * \endcode
		 *
		 * <b> Defining a remote Gaussian </b>
		 *
		 * There are four methods to create a remote Gaussian:
		 * \code
		 * Gaussian GRa(GR);              // Copy a remote Gaussian
		 * Gaussian GRb(GL, GL.REMOTE);   // Point to a whole local Gaussian
		 * Gaussian GRc(GL, ia);          // Point to a part of a given local Gaussian
		 * Gaussian GRd(x, P, ia);        // Point to a part of given vector and covariances matrix.
		 * \endcode
		 *
		 * To specify the set of indices the remote Gaussian points to, you add an indirect array to the constructor:
		 *
		 * See that the remote constructor \a GRc wants a local Gaussian \a GL to point to. Do NEVER provide a remote Gaussian to such constructor.
		 *
		 * For information about indirect arrays, see the documentation of ia_range(), ia_head() and similar functions in namespace ublasExtra.
		 *
		 * <b> Managing cross-variances through indirect indexing</b>
		 *
		 * Having two remote Gaussian instances \a G1 and \a G2 pointing to the same data (Gaussian \a G or pair {\a x , \a P })
		 * allows recovering their cross-variances matrix.
		 *
		 * You can recover a hard copy of the cross-variance block with
		 * \code
		 * mat c = project(P, G1.ia(), G2.ia());
		 * \endcode
		 * You can have an indirect access to the cross-variance block with
		 * \code
		 * sym_mat_indirect ic(P, G1.ia(), G2.ia());
		 * \endcode
		 *
		 * This is a graphical representation of the situation, where we want to access the blocks of data \c ic and \c ic' (in purple):
		 * 	\image html Gaussian.png "Two remote Gaussians G1 and G2 pointing to {\a x, \a P } via indirect arrays. The cross-variances \a ic and \a ic' are shown."
		 *
		 * Bear in mind that indirect arrays are not correlative: they contain sparse indices. Thus this is a more accurate representation of the situation:
		 * 	\image html GaussianSparse.png "Two remote Gaussians G1 and G2 pointing to {\a x, \a P } via indirect arrays. The cross-variances \a ic and \a ic' are shown."
		 *
		 * The following example is borrowed from \c test_gaussian02() in file \c rtslam/test_suite/test_gaussian.cpp:
		 *
		 * \code
		 *	jblas::vec x(300);                                      // The remote mean vector.
		 *	jblas::sym_mat P(300);                                  // The remote covariances matrix.
		 *	P(1,102) = 99;                                          // We put here a value to test later
		 *	Gaussian G1(x, P, ublasExtra::ia_range(0, 3));          // G1 points to positions {0, 1, 2}.
		 *	Gaussian G2(x, P, ublasExtra::ia_range(100, 104));      // G2 points to positions {100, 101, 102, 103}.
		 *	jblas::mat c = ublas::project(P, G1.ia(), G2.ia());     // Cross-variance (hard copy)
		 *	jblas::sym_mat_indirect ic(P, G1.ia(), G2.ia());        // Cross-variance (indirectly indexed)
		 *	cout << " c-value (should be 99): " << c(1,2) << endl;  // We recover here the values
		 *	cout << "ic-value (should be 99): " << ic(1,2) << endl; //
		 *	P(1,102) = 0;                                           // We change the P-value to test ic
		 *	cout << " c-value (should be 99): " << c(1,2) << endl;  // We recover here the values
		 *	cout << "ic-value (should be 0 ): " << ic(1,2) << endl; //
		 * \endcode
		 * which renders the output:
		 * \code
		 * >>  c-value (should be 99): 99
		 * >> ic-value (should be 99): 99
		 * >>  c-value (should be 99): 99
		 * >> ic-value (should be 0 ): 0
		 * \endcode
		 *
		 * Run this test yourself. Follow these steps :
		 * - edit \c rtslam/test_suite/test_gaussian.cpp\n
		 * - go to the bottom of the file, locate the macro function <c> BOOST_AUTO_TEST_CASE( test_gaussian )</c>\n
		 * - in this macro, comment all but \c test_gaussian02(). Save the file.\n
		 * - \c cd to directory \c $JAFAR_DIR/modules/rtslam/
		 * - type  <c> make test_gaussian </c>\n
		 * - Once you are done, recover the file to its original state.
		 *
		 * \ingroup rtslam
		 */
		class Gaussian {
			public:
				/**
				 * Storage type of Gaussians: LOCAL, REMOTE.
				 */
				typedef enum {
					REMOTE, ///< Mean and covariances point to an external pair {x,P}.
					LOCAL, ///< Mean and covariances are stored in \a x_local and \a P_local.
					UNCHANGED ///< Used only as the default flag for the copy constructor.
				} storage_t;

			private:
				bool hasNullCov_; ///< true if covariance is exactly null
				std::size_t size_; ///< size of data
				storage_t storage_; ///< select local or remote storage
				jblas::vec x_local; ///< local storage mean
				jblas::sym_mat P_local; ///< local storage covariance
				jblas::ind_array ia_; ///< indirect array of indices to storage
				jblas::vec_indirect x_; ///< indexed mean
				jblas::sym_mat_indirect P_; ///< indexed covariance

			public:


				// Getters

				inline storage_t storage() const {
					return storage_;
				}

				inline bool hasNullCov() {
					return hasNullCov_;
				}

				inline size_t size() const {
					return size_;
				}

				inline jblas::ind_array & ia() {
					return ia_;
				}

				inline jblas::vec_indirect & x() {
					return x_;
				}

				inline jblas::sym_mat_indirect & P() {
					return P_;
				}


				/*
				 * Setters
				 */
				inline void storage(const storage_t & _s) {
					storage_ = _s;
					if (_s == REMOTE)
						hasNullCov_ = false;
				}

				inline void hasNullCov(bool _hasNullCov) {
					hasNullCov_ = _hasNullCov;
				}

				inline void x(const jblas::vec & _x) {
					JFR_ASSERT(_x.size() == size_, "gaussian.hpp: set_x: size mismatch.");
					x_.assign(_x);
				}


				/**
				 * Set covariances matrix from standard deviations vector.
				 * \param std the vector of standard deviations.
				 */
				inline void std(const jblas::vec& _std) {
					JFR_ASSERT(_std.size() == P_.size1(), "gaussian.hpp: set_P: size mismatch.");
					hasNullCov_ = false;
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
					hasNullCov_ = false;
					P_.assign(_P);
				}


				/**
				 * Set off-diagonal block in the covariances matrix.
				 * Only works for local Gaussians.
				 * \param M the off-diagonal block.
				 * \param ia1 the indirect array for rows.
				 * \param ia2 the indirect array for columns.
				 */
				inline void P(const jblas::mat & M, const jblas::ind_array & ia1, const jblas::ind_array & ia2) {
					hasNullCov_ = false;
					project(P_local, ia1, ia2) = M;
				}


				/**
				 * Clear data, keep sizes and ranges.
				 * Clears the data of \a x_ and \a P_.
				 */
				inline void clear(void) {
					x_.assign(jblas::zero_vec(size_));
					P_.assign(jblas::zero_mat(size_, size_));
				}


				/**
				 * Local constructor from size.
				 * This constructor defines a local-storage Gaussian of size \a _size. Data is cleared automatically.
				 * \param _size the size of the Gaussian.
				 */
				inline Gaussian(const size_t _size) :
					hasNullCov_(false), size_(_size),
					storage_(LOCAL),
					x_local(size_),
					P_local(size_, size_),
					ia_(size_),
					x_(x_local, ia_.all()),
					P_(P_local, ia_.all(), ia_.all()) {
					clear();
					for (size_t i = 0; i < size_; i++)
						ia_(i) = i;
				}


				/**
				 * Local constructor from mean data, no covariance.
				 * This constructor stores the input information in the local storage \a x_local.
				 * This data is accessed through \a x().
				 * \param _x the Gaussian mean.
				 */
				inline Gaussian(const jblas::vec & _x) :
					hasNullCov_(true), size_(_x.size()),
					storage_(LOCAL),
					x_local(_x),
					P_local(size_, size_),
					ia_(size_), x_(x_local, ia_.all()),
					P_(P_local, ia_.all(), ia_.all()) {
					for (size_t i = 0; i < size_; i++)
						ia_(i) = i;
				}


				/**
				 * Local constructor from full data.
				 * This constructor stores the input information in the local storage \a x_local and \a P_local.
				 * This data is accessed through \a x() and \a P().
				 * \param _x the Gaussian mean.
				 * \param _P the Gaussian covariances matrix.
				 */
				inline Gaussian(const jblas::vec& _x, const jblas::sym_mat& _P) :
					hasNullCov_(false), size_(_x.size()),
					storage_(LOCAL),
					x_local(_x), P_local(_P),
					ia_(size_),
					x_(x_local, ia_.all()),
					P_(P_local, ia_.all(), ia_.all()) {
					for (size_t i = 0; i < size_; i++)
						ia_(i) = i;
				}


				/**
				 * Flexible copy constructor.
				 * - Called with Gaussian(G) is a copy constructor.
				 * - Called with Gaussian(G, UNCHANGED) is also a copy constructor.
				 * - Called with Gaussian(G, LOCAL) forces the created Gaussian to have local storage.
				 * - Called with Gaussian(G, REMOTE) forces the created Gaussian to have remote storage.
				 *
				 * \param G the Gaussian to copy.
				 * \param _storage the directive to force local or remote storage.
				 */
				inline Gaussian(const Gaussian & G, storage_t _storage = UNCHANGED) :
					hasNullCov_(G.hasNullCov_),
					size_  (G.size_),
					storage_(_storage == UNCHANGED ? G.storage_                                              : _storage),
					x_local (_storage == LOCAL     ? G.x_                                                    : G.x_local),
					P_local (_storage == LOCAL     ? G.P_                                                    : G.P_local),
					ia_     (_storage == LOCAL     ? jafar::jmath::ublasExtra::ia_range(0, size_)            : G.ia_),
					x_      (storage_ == LOCAL     ? jblas::vec_indirect     (x_local, ia_.all())            : G.x_),
					P_      (storage_ == LOCAL     ? jblas::sym_mat_indirect (P_local, ia_.all(), ia_.all()) : G.P_)
				{
				}


				/**
				 * Remote constructor from LOCAL Gaussian.
				 * <b>WARNING ! : G must be local.</b> Failure to guarantee so will result in a BOOST exception error.
				 *
				 * This constructor uses indirect indexing onto a local Gaussian provided to the constructor.
				 * The indirect array where this new Gaussian points to in the old one is also given as input.
				 * For practical use, the result is such that the new Gaussian Gnew has <i> x_ = G.x_local(ia_) </i> and <i> P_ = G.P_local(ia_,ia_).</i>
				 * The local storage \a x_local and \a P_local are kept at null size for economy.
				 * \param G the local Gaussian.
				 * \param _ia the indirect array.
				 */
				inline Gaussian(Gaussian & G, const jblas::ind_array & _ia) :
					hasNullCov_(false), size_(_ia.size()),
					storage_(REMOTE),
					x_local(0), P_local(0),
					ia_(_ia),
					x_(G.x_local, ia_), P_(G.P_local, ia_, ia_) {
				}


				/**
				 * Remote constructor from data.
				 * This constructor uses indirect indexing onto a remote pair {_x, _P} provided to the constructor.
				 * The indirect array where this new Gaussian points to in {_x, _P} is also given as input.
				 * For practical use, the result is such that the new Gaussian Gnew has <i> x_ = _x(_ia) </i> and <i> P_ = _P(_ia,_ia).</i>
				 * The local storage \a x_local and \a P_local is kept at null size for economy.
				 * \param _x the remote mean vector.
				 * \param _P the remote covariances matrix.
				 * \param _ia the indirect array.
				 */
				inline Gaussian(jblas::vec & _x, jblas::sym_mat & _P, const jblas::ind_array& _ia) :
					hasNullCov_(false), size_(_ia.size()),
					storage_(REMOTE),
					x_local(0), P_local(0),
					ia_(_ia), x_(_x, ia_),
					P_(_P, ia_, ia_) {
					//			JFR_ASSERT((_x.size() == _ia.size()) && (_x.size() == _P.size1()), "gaussian.hpp: Gaussian(): sizes mismatch.");
				}


				/**
				 * Probability density.
				 * \param v the evaluation point
				 */
				double probabilityDensity(const jblas::vec& v) const;

				/**
				 * Operator << for class Gaussian.
				 * It shows different information depending on the Gaussian having local or remote storage.
				 * For local storage, the mean and covariance data is shown.
				 * For remote storage, the size of the remote Gaussian is also shown, and the indirect array too.
				 */
				friend std::ostream& operator <<(std::ostream & s, jafar::rtslam::Gaussian & g_) {

					if (g_.storage() == LOCAL) {
						if (g_.hasNullCov()) {
							s << "\n .x : " << g_.x_ << "\n";
						}
						else { // Null covariance
							s << "\n .x : " << g_.x_ << "\n";
							s << " .P : " << g_.P_;
						}
					}
					else { // REMOTE
						s << "\n REM->[" << g_.x_.data().size() << "](...)\n";
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
