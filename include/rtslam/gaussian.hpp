
/**
 * \file gaussian.hpp
 * \author jsola, croussil
 * \date 13/02/2010
 * File defining the gaussian class.
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
		 * \author jsola & croussil
		 *
		 * This class defines a multi-dimensional Gaussian variable.
		 * It allows mean and covariances data to be stored locally or remotelly,
		 * depending on the constructor
		 * (the storage modality is selectable only at construction time).
		 * Remote Gaussians are indexed with indirect arrays.
		 *
		 * - Mean and covariances are accessed through \a x() and \a P().\n
		 * - The indirect array is part of the Gaussian class, and is accessible through \a ia().\n
		 *
		 * <i><b> HELP:</b></i> For information about indirect arrays, see the documentation
		 * of ia_range(), ia_head() and similar functions in namespace jafar::jmath::ublasExtra.
		 *
		 * <b> To construct a local Gaussian </b>
		 *
		 * You just give a size, a pair {\a x , \a P } or another Gaussian:
		 * \code
		 * Gaussian GLa(7);               			// Construct from size.
		 * Gaussian GLb(x);               			// Construct from vector.
		 * Gaussian GLc(x, P);            			// Construct from given vector and matrix.
		 * Gaussian GLd(GL);              			// Copy-construct from local Gaussian.
		 * Gaussian GLe(GR, Gaussian::LOCAL); 	// Copy-construct from remote, force local storage.
		 * \endcode
		 *
		 * You can create Gaussians with no covariance by providing just the mean vector.
		 * This is only possible for local Gaussians. The Gaussian \a GLb above is an example.
		 *
		 * See that the last constructor \a GLe takes a remote Gaussian (see below).
		 * The directive \a Gaussian::LOCAL tells the constructor to force local storage.
		 * The result is a local Gaussian (the data is copied to the local storage).
		 *
		 * <b> To construct a remote Gaussian </b>
		 *
		 * Remote Gaussians always point to local Gaussians (or to explicit {x,P} data):
		 * \code
		 * Gaussian GRa(GR);              			// Copy a remote Gaussian
		 * Gaussian GRb(GL, Gaussian::REMOTE);	// Point to a whole local Gaussian
		 * Gaussian GRc(x, P, ia);        			// Point to a part of given x and P.
		 * Gaussian GRd(GL, ia);          			// Point to a part of a given local Gaussian
		 * Gaussian GRe(GR, ia);          			// Point to a part of a given remote Gaussian
		 * \endcode
		 *
		 * In the first case, \a GRa points to the same Gaussian as \a GR.
		 *
		 * In the second case, the directive \a Gaussian::REMOTE tells the constructor not to copy \a GL (as in \a GLd above)
		 * but to create a remote Gaussian that points to \a GL.
		 *
		 * In the last three constructor modes, the produced Gaussian points to just a sub-set of the provided data.
		 * Specify the set of indices to the provided data with an indirect array <c> jblas::ind_array </c>.
		 *
		 * <i><b> REMEMBER: remote Gaussians always point to local Gaussians.</b></i>
		 * See that the remote constructor \a GRe accepts a remote Gaussian \a GR to point to.
		 * In this case, the created Gaussian \a GRe is set to point to the local Gaussian \a GR was pointing to,
		 * say \a GL, and its indirect array is composed accordingly
		 * (composed means that after creation we have <i>GRe.ia(i) = GR.ia(ia(i))</i>, that is,
		 * the resulting \a GR.ia is different from the provided one \a ia).
		 * The reference to \a GR is therefore lost.
		 *
		 * <b> Managing cross-variances through indirect indexing</b>
		 *
		 * Having two remote Gaussian instances \a G1 and \a G2
		 * pointing to the same data (Gaussian \a G or pair {\a x , \a P })
		 * allows recovering their cross-variances matrix.
		 *
		 * You can recover a hard copy of the cross-variance block with one of these:
		 * \code
		 * mat c = project(  P  , G1.ia(), G2.ia());
		 * mat c = project(G.P(), G1.ia(), G2.ia());
		 * \endcode
		 * You can have an indirect access to the cross-variance block with one of these:
		 * \code
		 * sym_mat_indirect ic(  P  , G1.ia(), G2.ia());
		 * sym_mat_indirect ic(G.P(), G1.ia(), G2.ia());
		 * \endcode
		 *
		 * This is a graphical representation of the situation,
		 * where we want to access the blocks of data \c ic and \c ic' (in purple):
		 * 	\image html Gaussian.png "Two remote Gaussians G1 and G2 pointing to {\a x, \a P } via indirect arrays. The cross-variances \a ic and \a ic' are shown."
		 *
		 * Bear in mind that indirect arrays are not correlative: they contain sparse indices.
		 * Thus this is a more accurate representation of the situation:
		 * 	\image html GaussianSparse.png "Two remote Gaussians G1 and G2 pointing to {\a x, \a P } via indirect arrays. The cross-variances \a ic and \a ic' are shown."
		 *
		 * The following example shows the recovery and manipulation of the cross-variance.
		 * It is borrowed from \c test_gaussian02() in file \c rtslam/test_suite/test_gaussian.cpp:
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
					REMOTE, ///< 		Mean and covariances point to an external pair {x,P}.
					LOCAL, ///<			Mean and covariances are stored in \a x_local and \a P_local.
					UNCHANGED ///< 	Used **only** as the default flag for the copy constructor.
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


				/**
				 * Local constructor from size.
				 * This constructor defines a local-storage Gaussian of size \a _size. Data is cleared automatically.
				 * \param _size the size of the Gaussian.
				 */
				inline Gaussian(const size_t _size) :
					hasNullCov_ (false),
					size_       (_size),
					storage_    (LOCAL),
					x_local     (size_),
					P_local     (size_, size_),
					ia_         (size_),
					x_          (x_local, ia_.all()),
					P_          (P_local, ia_.all(), ia_.all())
				{
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
					hasNullCov_ (true),
					size_       (_x.size()),
					storage_    (LOCAL),
					x_local     (_x),
					P_local     (size_, size_),
					ia_         (size_),
					x_          (x_local, ia_.all()),
					P_          (P_local, ia_.all(), ia_.all())
				{
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
					hasNullCov_ (false),
					size_       (_x.size()),
					storage_    (LOCAL),
					x_local     (_x),
					P_local     (_P),
					ia_         (size_),
					x_          (x_local, ia_.all()),
					P_          (P_local, ia_.all(), ia_.all())
				{
					JFR_ASSERT(_x.size() == _P.size1(), "gaussian::Gaussian():: x and P sizes do not match.");
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
				// Thanks to C. Roussillon for this constructor.
				inline Gaussian(const Gaussian & G, storage_t _storage = UNCHANGED) :
					hasNullCov_ (G.hasNullCov_),
					size_       (G.size_),
					//        # check storage       ?              # is local                                  : # is remote
					storage_(_storage == UNCHANGED  ?  G.storage_                                              : _storage),
					x_local (_storage == LOCAL      ?  G.x_                                                    : G.x_local),
					P_local (_storage == LOCAL      ?  G.P_                                                    : G.P_local),
					ia_     (_storage == LOCAL      ?  jafar::jmath::ublasExtra::ia_set(0, size_)            : G.ia_),
					x_      (storage_ == LOCAL      ?  jblas::vec_indirect     (x_local, ia_.all())            : G.x_),
					P_      (storage_ == LOCAL      ?  jblas::sym_mat_indirect (P_local, ia_.all(), ia_.all()) : G.P_)
				{
				}


				/**
				 * Remote constructor from Gaussian.
				 * This constructor uses indirect indexing onto the Gaussian provided to the constructor.
				 * The indirect array where this new Gaussian points to in the old one is also given as input.
				 *
				 * In case the input Gaussian is REMOTE, a composition of indirect arrays is performed,
				 * and the resulting Gaussian points to the Gaussian where the remote Gaussian points to.
				 *
				 * The local storage \a x_local and \a P_local in the constructed Gaussian are kept at null size for economy.
				 * \param G a Gaussian.
				 * \param _ia an indirect array of indices pointing to \a G.
				 */
				inline Gaussian(Gaussian & G, const jblas::ind_array & _ia) :
					hasNullCov_ (false),
					size_       (_ia.size()),
					storage_    (REMOTE),
					x_local     (0),
					P_local     (0),
					//     # check storage   ?                   # is local                   :             # is remote
					ia_(G.storage_ == LOCAL  ?  _ia                                           : G.ia_.compose(_ia)),
					x_ (G.storage_ == LOCAL  ?  jblas::vec_indirect     (G.x_local, ia_)      : jblas::vec_indirect     (G.x_.data(), ia_, 1)     ),
					P_ (G.storage_ == LOCAL  ?  jblas::sym_mat_indirect (G.P_local, ia_, ia_) : jblas::sym_mat_indirect (G.P_.data(), ia_, ia_, 1))
				{
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
					hasNullCov_ (false),
					size_       (_ia.size()),
					storage_    (REMOTE),
					x_local     (0),
					P_local     (0),
					ia_         (_ia),
					x_          (_x, ia_),
					P_          (_P, ia_, ia_)
				{
					JFR_ASSERT(_x.size() == _P.size1(), "gaussian::Gaussian():: x and P sizes do not match.");
				}


				// Getters
				inline bool                      hasNullCov() const    { return hasNullCov_; }
				inline storage_t                 storage() const       { return storage_;    }
				inline size_t                    size() const          { return size_;       }
				inline jblas::ind_array        & ia()                  { return ia_;         }
				inline const jblas::ind_array  & ia() const            { return ia_;         }
		                inline jblas::vec_indirect     & x()                   { return x_;          }
				inline const jblas::vec_indirect & x() const           { return x_;          }
				inline jblas::sym_mat_indirect & P()                   { return P_;          }
				inline const jblas::sym_mat_indirect & P() const       { return P_;          }
				inline double                  & x(size_t i)           { return x_(i);       }
				inline double                  & P(size_t i, size_t j) { return P_(i, j);    }

				 // Setters
				inline void  storage(const storage_t & _s) {
					storage_ = _s;
					if (_s == REMOTE)
						hasNullCov_ = false;
				}
				inline void  hasNullCov(bool _hasNullCov) {
					hasNullCov_ = _hasNullCov;
				}
				inline void  x(const jblas::vec & _x) {
					JFR_ASSERT(_x.size() == size_, "gaussian.hpp: set_x: size mismatch.");
					x_.assign(_x);
				}
				inline void  P(const jblas::sym_mat & _P) {
					JFR_ASSERT(_P.size1() == size_, "gaussian.hpp: set_P: size mismatch.");
					hasNullCov_ = false;
					P_.assign(_P);
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
				 * Set covariances matrix from standard deviation scalar value.
				 * \param std the standard deviation.
				 */
				inline void std(double _std) {
					hasNullCov_ = false;
					P_.assign(jblas::zero_mat(size_));
					for (std::size_t i = 0; i < size_; i++) {
						P_(i, i) = _std * _std;
					}
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
				virtual void clear(void) {
					x_.assign(jblas::zero_vec(size_));
					P_.assign(jblas::zero_mat(size_, size_));
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
				friend std::ostream& operator <<(std::ostream & s, Gaussian const & g_) {

					if (g_.storage() == LOCAL) {
						if (g_.hasNullCov()) {
							s << std::endl << "  .x : " << g_.x_ << std::endl;
						}
						else { // Null covariance
							s << std::endl << "  .x : " << g_.x_ << std::endl;
							s << "  .P : " << g_.P_;
						}
					}
					else { // REMOTE
						s << "  .ia: " << g_.ia_ << std::endl;
						s << "  .x : " << g_.x_ << std::endl;
						s << "  .P : " << g_.P_;
					}
					return s;
				}

		};

	}
}

#endif /* GAUSSIAN_HPP_ */
