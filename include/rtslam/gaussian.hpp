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

namespace jafar {
	namespace rtslam {

		/**
		 * Storage type of Gaussians: LOCAL, REMOTE.
		 */
		typedef enum {
			LOCAL, REMOTE
		} storage_t;

		/**
		 * Class for indirect Gaussians
		 * \ingroup rtslam
		 */
		class Gaussian {
			public:
				storage_t storage;
				std::size_t size;
				jblas::vec x_;
				//				boost::numeric::ublas::symmetric_adaptor<jblas::sym_mat> P_;
				jblas::sym_mat P_;
				jblas::ind_array ia;
				jblas::vec_indirect x;
				jblas::sym_mat_indirect P;

				/**
				 * Empty constructor
				 */
				Gaussian() :
					x(x_, 0), P(P_, 0, 0) {
				}

				/**
				 * Copy constructor.
				 */
				Gaussian(const Gaussian& G);

				/**
				 * Local constructors.
				 * These constructors store the input information in the local storage x_ and P_.
				 * The range is set to occupy the full data size.
				 */
				Gaussian(const jblas::vec& _x, const jblas::sym_mat& _P);

				/**
				 * Remote constructors.
				 * These constructors use indirect indexing onto an external Gaussian provided to the constructor.
				 * The range is also given as input.
				 * If mean and covariance are provided, these affect the remote Gaussian.
				 * The local storage is kept at null size for economy.
				 */
				Gaussian(Gaussian& G, const jblas::ind_array& _ia);

				/**
				 * Clear data, keep sizes and ranges
				 */
				void clear(void);

				/**
				 * Clear local data, keep sizes and ranges
				 */
				void clear_local(void);

				/**
				 * Accessors
				 */
				void set_storage_type(const storage_t _s);
				void set_r(const jblas::ind_array& _ia);
				void set_size(const std::size_t _size);
				void set_x(const jblas::vec& _x);
				void set_P(const jblas::vec& _std);
				void set_P(const jblas::sym_mat& _P);
				void set_P(const jblas::sym_mat& _P, const jblas::ind_array _ia);
				void set_P(const jblas::mat& _P, const jblas::ind_array _r1, const jblas::ind_array _r2);
				void set(const jblas::vec& _x, const jblas::vec& _std);
				void set(const jblas::vec& _x, const jblas::sym_mat& _P);
				void set(jblas::vec _x, jblas::sym_mat _P, const jblas::ind_array& _ia);
				void set(Gaussian& G, const jblas::ind_array& _ia);

		};

		/**
		 * Set mean
		 */
		void Gaussian::set_x(const jblas::vec & _x) {
			JFR_ASSERT(_x.size() == size, "gaussian.hpp: set_x: size mismatch.");
			x.assign(_x);
		}

		/**
		 * Set covariance
		 */
		void Gaussian::set_P(const jblas::sym_mat & _P) {
			JFR_ASSERT(_P.size1() == size, "gaussian.hpp: set_P: size mismatch.");
			P.assign(_P);
		}

		/*
		 * Set cov from std
		 */
		void Gaussian::set_P(const jblas::vec& _std) {
			JFR_ASSERT(_std.size() == P.size1(), "gaussian.hpp: set_P: size mismatch.");
			P.assign(jblas::zero_mat(size));
			for (std::size_t i = 0; i < size; i++) {
				P(i, i) = _std(i) * _std(i);
			}
		}

		/**
		 * Copy constructor
		 */
		Gaussian::Gaussian(const Gaussian & G) :
			storage(G.storage), size(G.size), x_(G.x_), P_(G.P_), ia(G.ia), x(G.x), P(G.P) {
		}

		/*
		 * Local constructor
		 */
		Gaussian::Gaussian(const jblas::vec& _x, const jblas::sym_mat& _P) :
			storage(LOCAL), size(_x.size()), x_(_x), P_(_P), ia(size), x(x_, ia.all()), P(P_, ia.all(), ia.all()) {
		}

		/*
		 * Remote constructor
		 */
		Gaussian::Gaussian(Gaussian & G, const jblas::ind_array & _ia) :
			storage(REMOTE), size(_ia.size()), x_(0), P_(0), ia(_ia), x(G.x_, ia), P(G.P_, ia, ia) {
		}

		/**
		 * Clear data, keep sizes and ranges
		 */
		void Gaussian::clear(void) {
			x.assign(jblas::zero_vec(size));
			P.assign(jblas::zero_mat(size, size));
		}

		/**
		 * Clear local data, keep sizes and ranges
		 */
		void Gaussian::clear_local(void) {
			x_.clear();
			P_.clear();
		}

		/**
		 * Operator << for class for ind_array
		 * TODO: see where to put this operator!
		 */
		std::ostream& operator <<(std::ostream & s, jblas::ind_array & ia_) {
			s << "[" << ia_.size() << "]{";
			for (size_t i = 0; i < ia_.size() - 1; i++)
				s << ia_(i) << ",";
			s << ia_(ia_.size() - 1) << "}";
			return s;
		}

		/**
		 * Operator << for class Gaussian.
		 * It shows different information depending on the Gaussian having local or remote storage.
		 * For local storage, the mean and covariance data is shown.
		 * For remote storage, the size of the remote Gaussian is also shown, and the indirect array too.
		 */
		std::ostream& operator <<(std::ostream & s, jafar::rtslam::Gaussian & g_) {
			if (g_.storage == LOCAL)
				s << "Gaussian with local storage: \n";
			else {
				size_t sz = g_.x.data().size();
				s << "Gaussian with remote storage: \n";
				s << "->x_: [" << sz << "](...data not shown...)\n";
				s << "->P_: [" << sz << "," << sz << "](...data not shown...)\n";
				s << " .ia: " << g_.ia << "\n";
			}
			s << " .x : " << g_.x << "\n .P : " << g_.P;
			return s;
		}

	}
}

#endif /* GAUSSIAN_HPP_ */
