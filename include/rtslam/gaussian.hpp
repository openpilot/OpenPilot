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
				jblas::sym_mat P_;
				jblas::ind_array r;
				jblas::vec_indirect x;
				jblas::sym_mat_indirect P;

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
				Gaussian(Gaussian& G, const jblas::ind_array& _r);

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
				void set_r(const jblas::ind_array& _r);
				void set_size(const std::size_t _size);
				void set_x(const jblas::vec& _x);
				void set_P(const jblas::vec& _std);
				void set_P(const jblas::sym_mat& _P);
				void set_P(const jblas::sym_mat& _P, const jblas::ind_array _r);
				void set_P(const jblas::mat& _P, const jblas::ind_array _r1, const jblas::ind_array _r2);
				void set(const jblas::vec& _x, const jblas::vec& _std);
				void set(const jblas::vec& _x, const jblas::sym_mat& _P);
				void set(jblas::vec _x, jblas::sym_mat _P, const jblas::ind_array& _r);
				void set(Gaussian& G, const jblas::ind_array& _r);

				//				std::ostream& jafar::rtslam::operator <<(std::ostream& s,  Gaussian& g_);
		};

		/**
		 * Set mean
		 */
		void Gaussian::set_x(const jblas::vec & _x)
		{
			JFR_ASSERT(_x.size() == size,"gaussian.hpp: set_x: size mismatch.");
			x.assign(_x);
		}

		/**
		 * Set covariance
		 */
		void Gaussian::set_P(const jblas::sym_mat & _P)
		{
			JFR_ASSERT(_P.size1() == size,"gaussian.hpp: set_P: size mismatch.");
			P.assign(_P);
		}

		/*
		 * Set cov from std
		 */
		void Gaussian::set_P(const jblas::vec& _std) {
			JFR_ASSERT(_std.size() == P.size1(),"gaussian.hpp: set_P: size mismatch.");
			P.assign(jblas::zero_mat(size));
			for (std::size_t i = 0; i < size; i++) {
				P(i, i) = _std(i) * _std(i);
			}
		}


		/**
		 * Copy constructor
		 */
		Gaussian::Gaussian(const Gaussian & G) :
			storage(G.storage), size(G.size), x_(G.x_), P_(G.P_), r(G.r), x(G.x), P(G.P) {
		}

		/*
		 * Local constructor
		 */
		Gaussian::Gaussian(const jblas::vec& _x, const jblas::sym_mat& _P) :
			storage(LOCAL), size(_x.size()), x_(_x), P_(_P), r(size), x(x_, r.all()), P(P_, r.all(), r.all()) {
		}

		/*
		 * Remote constructor
		 */
		Gaussian::Gaussian(Gaussian & G, const jblas::ind_array & _r) :
			storage(REMOTE), size(_r.size()), x_(0), P_(0), r(_r), x(G.x_, r), P(G.P_, r, r) {
		}

		/**
		 * Clear data, keep sizes and ranges
		 */
		void Gaussian::clear(void){
			x.assign(jblas::zero_vec(size));
			P.assign(jblas::zero_mat(size,size));
		}



		/**
		 * Clear
		 */
		void Gaussian::clear_local(void){
			x_.clear();
			P_.clear();
		}

		/*
		 * Operators
		 */
		std::ostream& operator <<(std::ostream & s, jafar::rtslam::Gaussian & g_) {
			if (g_.storage == LOCAL)
				s << "Gaussian with local storage:\n";
			else
				s << "Gaussian with remote storage:\n .r: TODO: gaussian.hpp: operator << for indirect_array<> type." << "\n";
			//			s << "Gaussian with remote storage\n .r: " << g_.r << "\n";
			s << " .x: " << g_.x << "\n .P: " << g_.P;
			return s;
		}
	}
}




#endif /* GAUSSIAN_HPP_ */
