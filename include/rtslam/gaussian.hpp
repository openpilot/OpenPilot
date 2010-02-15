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

namespace jafar
{
	namespace rtslam
	{

		/**
		 * Storage type of Gaussians: LOCAL, REMOTE.
		 */
		typedef enum
		{
			LOCAL, REMOTE
		} storage_type;

		/**
		 * Class for indirect Gaussians
		 * \ingroup rtslam
		 */
		class Gaussian
		{
			protected:
				jblas::vec x_;
				jblas::sym_mat P_;
				storage_type storage;
				std::size_t size;
				jblas::ind_array r;
				jblas::vec_indirect x;
				jblas::sym_mat_indirect P;
			public:

				/**
				 * Copy constructor.
				 */
				Gaussian(const Gaussian& G);

				/**
				 * Local constructors.
				 * These constructors store the input information in the local storage x_ and P_.
				 * The range is set to occupy the full data size.
				 */
				Gaussian(const jblas::vec& _x);
				Gaussian(const jblas::vec& _x, const jblas::vec& _std);
				Gaussian(const jblas::vec& _x, const jblas::sym_mat& _P);

				/**
				 * Remote constructors.
				 * These constructors use indirect indexing onto an external Gaussian provided to the constructor.
				 * The range is also given as input.
				 * If mean and covariance are provided, these affect the remote Gaussian.
				 * The local storage is kept at null size for economy.
				 */
				Gaussian(const Gaussian& G, const jblas::ind_array& _r);
				Gaussian(const Gaussian& G, const jblas::ind_array& _r, const jblas::vec& _x);
				Gaussian(const Gaussian& G, const jblas::ind_array& _r, const jblas::vec& _x, const jblas::vec& _std);
				Gaussian(const Gaussian& G, const jblas::ind_array& _r, const jblas::vec& _x, const jblas::sym_mat& _P);

				/**
				 * Clear all data, keep sizes and ranges
				 */
				void clear(void);

				/**
				 * Accessors
				 */
				void set_storage_type(const storage_type _s);
				void set_r(const jblas::ind_array& _r);
				void set_size(const std::size_t _size);
				void set_x(const jblas::vec& _x);
				void set_P(const jblas::vec& _std);
				void set_P(const jblas::sym_mat& _P);
				void set(const jblas::vec& _x, const jblas::vec& _std);
				void set(const jblas::vec& _x, const jblas::sym_mat& _P);
				void set(const Gaussian& G, const jblas::ind_array& _r, const jblas::vec& _x, const jblas::vec& _std);
				void set(const Gaussian& G, const jblas::ind_array& _r, const jblas::vec& _x, const jblas::sym_mat& _P);

				std::ostream& jafar::jmath::operator <<(std::ostream& s, const Gaussian& g_);
		};

		/*
		 * Set cov from std
		 */
		void Gaussian::set_P(const jblas::vec& _std)
		{
			P.assign(jblas::zero_mat(size));
			for (std::size_t i = 0; i < size; i++)
			{
				P(i, i) = _std(i) * _std(i);
			}
		}

		/*
		 * Local constructor
		 */
		Gaussian::Gaussian(const jblas::vec& _x) :
			storage(LOCAL), size(_x.size()), x_(_x), r(0, size)
		{
			P_.resize(size, size);
			P_.clear();
			x(x_, r); // affect to local data
			P(P_, r, r); // affect to local data
		}

		/*
		 * Local constructor
		 */
		Gaussian::Gaussian(const jblas::vec& _x, const jblas::vec& _std) :
			storage(LOCAL), size(_x.size()), x_(_x), r(0, size)
		{
			JFR_PRECOND(_x.size() == _std.size(),
									"gaussian::Gaussian: sizes of _x and _std do not match");
			P_.resize(size, size);
			for (size_t i = 0; i < size; i++)
			{
				P_(i, i) = _std(i);
			}
			x(x_, r); // direct to local data
			P(P_, r, r); // direct to local data
		}

		/*
		 * Local constructor
		 */
		Gaussian::Gaussian(const jblas::vec& _x, const jblas::sym_mat& _P) :
			storage(LOCAL), size(_x.size()), x_(_x), P_(_P), r(0, size), x(x_, r), P(P_, r, r)
		{
		}

		/*
		 * Remote constructor
		 */
		Gaussian::Gaussian(const Gaussian G, const jblas::ind_array& _r) :
			storage(REMOTE), r(_r), size(_r.size()), x(G, r), P(G, r, r)
		{
		}

		/*
		 * Remote constructor
		 */
		Gaussian::Gaussian(const Gaussian G, const jblas::ind_array& _r, const jblas::vec& _x) :
			storage(REMOTE), r(_r), size(_r.size()), x(G, r), P(G, r, r)
		{
			x.assign(_x);
			P.assign(jblas::zero_mat(size));
		}

		/*
		 * Remote constructor
		 */
		Gaussian::Gaussian(const Gaussian G, const jblas::ind_array& _r, const jblas::vec& _x, const jblas::sym_mat& _P) :
			storage(REMOTE), r(_r), size(_r.size()), x(G, r), P(G, r, r)
		{
			x.assign(_x);
			P.assign(_P);
		}

		/*
		 * Operators
		 */
		std::ostream& jafar::jmath::operator <<(std::ostream& s, const Gaussian& g_)
		{
			s << g_.x << " - " << g_.P;
			return s;
		}
	}
}

#endif /* GAUSSIAN_HPP_ */
