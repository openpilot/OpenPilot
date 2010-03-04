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
#include "jmath/ublasExtra.hpp"

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
				storage_t storage_;
				std::size_t size_;
				jblas::vec x_;
				jblas::sym_mat P_;
			public:
				jblas::ind_array ia;
				jblas::vec_indirect x;
				jblas::sym_mat_indirect P;

				/**
				 * Empty constructor
				 */
				Gaussian();

				/**
				 * Copy constructor.
				 */
				Gaussian(const Gaussian& G);

				/**
				 * Local constructor.
				 * This constructor defines a local-storage Gaussian of size \a _size.
				 * \param _size the size of the Gaussian.
				 */
				Gaussian(const size_t _size);
				/**
				 * Local constructor.
				 * This constructor stores the input information in the local storage \a x_ and \a P_.
				 * This data is accessed through \a x and \a P.
				 * \param _x the Gaussian mean.
				 * \param _P the Gaussian covariances matrix.
				 */
				Gaussian(const jblas::vec& _x, const jblas::sym_mat& _P);

				/**
				 * Remote constructor.
				 * This constructor uses indirect indexing onto an remote Gaussian provided to the constructor.
				 * The indirect array where this new Gaussian points to in the old one is also given as input.
				 * For practical use, the result is such that the new Gaussian Gnew has <i> x = G.x(ia) </i> and <i> P = G.P(ia,ia).</i>
				 * The local storage \a x_ and \a P_ is kept at null size for economy.
				 * \param G the remote Gaussian.
				 * \param _ia the indirect array of indices pointing to G.x_ and G.P_,
				 * such that <i> x = G.x(ia) </i> and <i> P = G.P(ia,ia).</i>
				 */
				Gaussian(Gaussian& G, const jblas::ind_array& _ia);

				/**
				 * Remote constructor.
				 * This constructor uses indirect indexing onto a remote pair {x, P} provided to the constructor.
				 * The indirect array where this new Gaussian points to in {x, P} is also given as input.
				 * For practical use, the result is such that the new Gaussian Gnew has <i> this.x = x(ia) </i> and <i> this.P = P(ia,ia).</i>
				 * The local storage \a x_ and \a P_ is kept at null size for economy.
				 * \param x the remote mean vector.
				 * \param P the remote covariances matrix.
				 * \param _ia the indirect array of indices pointing to x and P,
				 * such that <i> this.x = x(ia) </i> and <i> this.P = P(ia,ia).</i>
				 */
				Gaussian(jblas::vec & _x, jblas::sym_mat & _P, const jblas::ind_array& _ia);

				/**
				 * Clear data, keep sizes and ranges.
				 * Clears the data of \a x and \a P.
				 */
				void clear(void);

				/**
				 * Clear local data, keep sizes and ranges
				 * Clears the local data of \a x_ and \a P_.
				 */
				void clear_local(void);

				/*
				 * Accessors
				 */
				void set_storage(const storage_t & _s) {
					storage_ = _s;
				}
				void set_size(const std::size_t & _size) {
					size_ = _size;
				}
				void set_ia(const jblas::ind_array & _ia);
				void set_x(const jblas::vec & _x);

				/**
				 * Set covariances matrix from standard deviations vector.
				 * \param the vector of standard deviations.
				 */
				void set_P(const jblas::vec & _std);
				/**
				 * Set covariances matrix from a covariances matrix.
				 * \param P the covariances matrix.
				 */
				void set_P(const jblas::sym_mat & _P);

				/**
				 * Set off-diagonal block in the covariances matrix.
				 * Only tested for local Gaussians.
				 * \param M the off-diagonal block.
				 * \param ia1 the indirect array for rows.
				 * \param ia2 the indirect array for columns.
				 */
				void set_P_off_diag(const jblas::mat& M, const jblas::ind_array & ia1, const jblas::ind_array& ia2);

				storage_t storage() {
					return storage_;
				}

				size_t size() {
					return size_;
				}

		};

		/**
		 * Set mean
		 */
		void Gaussian::set_x(const jblas::vec & _x) {
			JFR_ASSERT(_x.size() == size_, "gaussian.hpp: set_x: size mismatch.");
			x.assign(_x);
		}

		/**
		 * Set covariance
		 */
		void Gaussian::set_P(const jblas::sym_mat & _P) {
			JFR_ASSERT(_P.size1() == size_, "gaussian.hpp: set_P: size mismatch.");
			P.assign(_P);
		}

		/*
		 * Set cov from std
		 */
		void Gaussian::set_P(const jblas::vec& _std) {
			JFR_ASSERT(_std.size() == P.size1(), "gaussian.hpp: set_P: size mismatch.");
			P.assign(jblas::zero_mat(size_));
			for (std::size_t i = 0; i < size_; i++) {
				P(i, i) = _std(i) * _std(i);
			}
		}

		void Gaussian::set_P_off_diag(const jblas::mat & M, const jblas::ind_array & ia1, const jblas::ind_array & ia2) {
			project(P_, ia1, ia2) = M;
		}

		/**
		 * Empty constructor
		 */
		Gaussian::Gaussian() :
			storage_(GAUSSIAN_LOCAL), x_(0), P_(0), ia(0), x(x_, 0), P(P_, 0, 0) {
		}

		/**
		 * Copy constructor
		 */
		Gaussian::Gaussian(const Gaussian & G) :
			storage_(G.storage_), size_(G.size_), x_(G.x_), P_(G.P_), ia(G.ia), x(G.x), P(G.P) {
		}

		/*
		 * Local constructor from size
		 */
		Gaussian::Gaussian(const size_t _size) :
			storage_(GAUSSIAN_LOCAL), size_(_size), x_(size_), P_(size_, size_), ia(size_), x(x_, ia.all()), P(P_, ia.all(),
			    ia.all()) {
		}

		/*
		 * Local constructor from data
		 */
		Gaussian::Gaussian(const jblas::vec& _x, const jblas::sym_mat& _P) :
			storage_(GAUSSIAN_LOCAL), size_(_x.size()), x_(_x), P_(_P), ia(size_), x(x_, ia.all()), P(P_, ia.all(), ia.all()) {
		}

		/*
		 * Remote constructor
		 */
		Gaussian::Gaussian(Gaussian & G, const jblas::ind_array & _ia) :
			storage_(GAUSSIAN_REMOTE), size_(_ia.size()), x_(0), P_(0), ia(_ia), x(G.x_, ia), P(G.P_, ia, ia) {
		}

		/*
		 * Remote constructor
		 */
		Gaussian::Gaussian(jblas::vec & _x, jblas::sym_mat & _P, const jblas::ind_array& _ia) :
			storage_(GAUSSIAN_REMOTE), size_(_ia.size()), x_(0), P_(0), ia(_ia), x(_x, ia), P(_P, ia, ia) {
			//			JFR_ASSERT((_x.size() == _ia.size()) && (_x.size() == -P.size1()), "gaussian.hpp: Gaussian(): sizes mismatch.");
		}

		/**
		 * Clear data, keep sizes and ranges
		 */
		void Gaussian::clear(void) {
			x.assign(jblas::zero_vec(size_));
			P.assign(jblas::zero_mat(size_, size_));
		}

		/**
		 * Clear local data, keep sizes and ranges
		 */
		void Gaussian::clear_local(void) {
			x_.clear();
			P_.clear();
		}

		//		/**
		//		 * Operator << for class for ind_array
		//		 * TODO: see where to put this operator!
		//		 */
		//		template <class A>
		//		std::ostream& operator <<(std::ostream & s, boost::numeric::ublas::indirect_array<A> & ia_) {
		//			s << "[" << ia_.size() << "]{";
		//			for (size_t i = 0; i < ia_.size() - 1; i++)
		//				s << ia_(i) << ",";
		//			s << ia_(ia_.size() - 1) << "}";
		//			return s;
		//		}


		/**
		 * Operator << for class Gaussian.
		 * It shows different information depending on the Gaussian having local or remote storage.
		 * For local storage, the mean and covariance data is shown.
		 * For remote storage, the size of the remote Gaussian is also shown, and the indirect array too.
		 */
		std::ostream& operator <<(std::ostream & s, jafar::rtslam::Gaussian & g_) {

			using namespace jafar::jmath;
			using namespace boost::numeric::ublas;

			if (g_.storage() == GAUSSIAN_LOCAL)
				s << "Gaussian with local storage: \n";
			else {
				size_t sz = g_.x.data().size();
				s << "Gaussian with remote storage: \n";
				s << "->x_: [" << sz << "](...data not shown...)\n";
				s << "->P_: [" << sz << "," << sz << "](...data not shown...)\n";
				s << " .ia: " << g_.ia << "\n";
			}
			s << " .x : " << g_.x;
			s << "\n .P : " << g_.P;
			return s;
		}

	}
}

#endif /* GAUSSIAN_HPP_ */
