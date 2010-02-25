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
				Gaussian( Gaussian& G);

				/**
				 * Local constructors.
				 * These constructors store the input information in the local storage x_ and P_.
				 * The range is set to occupy the full data size.
				 */
				Gaussian( jblas::vec& _x);
				Gaussian( jblas::vec& _x,  jblas::vec& _std);
				Gaussian( jblas::vec& _x,  jblas::sym_mat& _P);
				//				:
				//					storage(LOCAL),
				//					size(_x.size())
				//					x_(_x),
				//					P_(_P)
				//					{
				//					x(x_, r.all());
				//					P(P_, r.all(), r.all());
				//				}

				/**
				 * Remote constructors.
				 * These constructors use indirect indexing onto an external Gaussian provided to the constructor.
				 * The range is also given as input.
				 * If mean and covariance are provided, these affect the remote Gaussian.
				 * The local storage is kept at null size for economy.
				 */
				Gaussian( Gaussian& G,  jblas::ind_array& _r);
				Gaussian( Gaussian& G,  jblas::ind_array& _r,  jblas::vec& _x);
				Gaussian( Gaussian& G,  jblas::ind_array& _r,  jblas::vec& _x,  jblas::vec& _std);
				Gaussian( Gaussian& G,  jblas::ind_array& _r,  jblas::vec& _x,  jblas::sym_mat& _P);

				/**
				 * Clear all data, keep sizes and ranges
				 */
				void clear(void);

				/**
				 * Accessors
				 */
				void set_storage_type( storage_t _s);
				void set_r( jblas::ind_array& _r);
				void set_size( std::size_t _size);
				void set_x( jblas::vec& _x);
				void set_P( jblas::vec& _std);
				void set_P( jblas::sym_mat& _P);
				void set_P( jblas::sym_mat& _P, jblas::ind_array _r);
				void set_P( jblas::mat& _P, jblas::ind_array _r1, jblas::ind_array _r2);
				void set( jblas::vec& _x,  jblas::vec& _std);
				void set( jblas::vec& _x,  jblas::sym_mat& _P);
				void set( Gaussian& G,  jblas::ind_array& _r,  jblas::vec& _x,  jblas::vec& _std);
				void set( Gaussian& G,  jblas::ind_array& _r,  jblas::vec& _x,  jblas::sym_mat& _P);

				//				std::ostream& jafar::jmath::operator <<(std::ostream& s,  Gaussian& g_);
		};

		/*
		 * Set cov from std
		 */
		//		void Gaussian::set_P( jblas::vec& _std)
		//		{
		//			P.assign(jblas::zero_mat(size));
		//			for (std::size_t i = 0; i < size; i++)
		//			{
		//				P(i, i) = _std(i) * _std(i);
		//			}
		//		}

		/**
		 * Set covariance from a covariances matrix
		 */
		//		void Gaussian::set_P( jblas::sym_mat& _P)
		//		{
		//			JFR_PRECOND(_P.size1() == size, "gaussian::set_P: sizes of _P and Gaussian do not match");
		//			for (std::size_t i = 0; i < size; i++)
		//			{
		//				for (std::size_t j = 0; j <= i; j++)
		//				{
		//					P(i, j) = _P(i, j);
		//				}
		//			}
		//		}

		/**
		 * Set covariances block-diagonal
		 */
		//		void Gaussian::set_P( jblas::mat& _P, jblas::ind_array _r1, jblas::ind_array _r2)
		//		{
		//			JFR_PRECOND((_r1.size() == _P.size1()) && (_r2.size() == _P.size2()),
		//			    "gaussian::set_P: sizes of _P , _r1 and _r2 do not match");
		//			JFR_PRECOND((_r1.size() <= size) && (_r2.size() <= size),
		//			    "gaussian::set_P: size of _P too big for local Gaussian.");
		//			for (std::size_t i = 0; i < _r1.size(); i++)
		//			{
		//				for (std::size_t j = 0; j < _r2.size(); j++)
		//				{
		//					P_(r(i), r(j)) = _P(i, j);
		//				}
		//			}
		//		}

		/**
		 * Set covariance block - any block
		 */
		//		void Gaussian::set_P(){}

		/*
		 * Local constructor
		 */
		//		Gaussian::Gaussian( jblas::vec& _x) :
		//			storage(LOCAL), size(_x.size()), x_(_x), r(size)
		//		{
		//			r.all_;
		//			P_.resize(size, size);
		//			P_.clear();
		//			x(x_, r); // affect to local data
		//			P(P_, r, r); // affect to local data
		//		}

		/*
		 * Local constructor
		 */
		//		Gaussian::Gaussian( jblas::vec& _x,  jblas::vec& _std) :
		//			storage(LOCAL), size(_x.size()), x_(_x), r(0, size)
		//		{
		//			JFR_PRECOND(_x.size() == _std.size(), "gaussian::Gaussian: sizes of _x and _std do not match");
		//			P_.resize(size, size);
		//			for (size_t i = 0; i < size; i++)
		//			{
		//				P_(i, i) = _std(i);
		//			}
		//			x(x_, r); // direct to local data
		//			P(P_, r, r); // direct to local data
		//		}

		/*
		 * Local constructor
		 */
		Gaussian::Gaussian( jblas::vec& _x,  jblas::sym_mat& _P) :
			storage(LOCAL), size(_x.size()), x_(_x), P_(_P), r(size), x(x_, r.all()), P(P_, r.all(), r.all()) {
		}

		/*
		 * Remote constructor
		 */
		Gaussian::Gaussian( Gaussian & G,  jblas::ind_array & _r) :
			storage(REMOTE), size(_r.size()), x_(0), P_(0), r(size), x(G.x_, r), P(G.P_, r, r) {
			r = _r;
			//			x(G.x_, r);
			//			P(G.P_, r, r);
		}

		/*
		 * Remote constructor
		 */
		//		Gaussian::Gaussian( Gaussian G,  jblas::ind_array& _r,  jblas::vec& _x) :
		//			storage(REMOTE), r(_r), size(_r.size()), x(G, r), P(G, r, r)
		//		{
		//			x.assign(_x);
		//			P.assign(jblas::zero_mat(size));
		//		}

		/*
		 * Remote constructor
		 */
		//		Gaussian::Gaussian( Gaussian G,  jblas::ind_array& _r,  jblas::vec& _x,  jblas::sym_mat& _P) :
		//			storage(REMOTE), r(_r), size(_r.size()), x(G, r), P(G, r, r)
		//		{
		//			x.assign(_x);
		//			P.assign(_P);
		//		}

		/*
		 * Operators
		 */
//		std::ostream& jafar::jmath::operator <<(std::ostream& s,  Gaussian& g_) {
//			s << g_.x << " - " << g_.P;
//			return s;
//		}
	}
}

#endif /* GAUSSIAN_HPP_ */
