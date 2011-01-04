/** @file jann.hpp
 * This is the standard header defined in jafar to bind the flann library
 * @ingroup jmath
 */

#ifndef JMATH_JANN_HPP
#define JMATH_JANN_HPP

#include "jafarConfig.h"

#ifdef HAVE_FLANN

#include <flann/flann.hpp>
#include <boost/numeric/ublas/vector.hpp>
#include <boost/numeric/ublas/matrix.hpp>

/** \addtogroup jmath */
/*@{*/

/// shortcut for ublas namespace
namespace ublas = boost::numeric::ublas;

/// special nemespace to wrap flann library algorithms
namespace jann {
	/// algorithm to be used for research, use only if you want to create a new Index
	enum algorithm {
		LINEAR        = 0,
		KDTREE        = 1,
		KMEANS        = 2,
		COMPOSITE     = 3,
		KDTREE_SINGLE = 4,
		SAVED         = 254,
		AUTOTUNED     = 255
	};
	/// algorithm to initialize centers for the K Means algorithm
	enum centers_init {
		CENTERS_RANDOM   = 0,
		CENTERS_GONZALES = 1,
		CENTERS_KMEANSPP = 2
	};
	/// determine log level
	enum log_level {
		LOG_NONE  = 0,
		LOG_FATAL = 1,
		LOG_ERROR = 2,
		LOG_WARN  = 3,
		LOG_INFO  = 4
	};
	/// supported distances
	enum distance {
		EUCLIDEAN        = 1,
		MANHATTAN        = 2,
		MINKOWSKI        = 3,
		MAX_DIST         = 4,
		HIST_INTERSECT   = 5,
		HELLINGER        = 6,
		CS               = 7,
		CHI_SQUARE       = 7,
		KL               = 8,
		KULLBACK_LEIBLER = 8
	};

	/** Class index_factory
	 * base class for all searching indexes. You should not used directly unless you want
	 * to create a new index searching algorithm.
	 */
	template<typename D>
	class index_factory 
	{
		flann::Index<D> *m_index;
	public:
		///element type of D
		typedef typename D::ElementType element;
		///distance type of D
		typedef typename D::DistanceType distance;
		///dataset stored as a flann::Matrix
		flann::Matrix<element> dataset;
		/**  
		 * @param _data: dataset in a ubls::matrix format
		 * @param params: index parameters as specified in flann manual
		 * @param d: flann::distance structure
		 */
		index_factory(const ublas::matrix<element>& _data, 
									const flann::IndexParams& params, D d = D() )
		{
			dataset = flann::Matrix<element>(new element[_data.size1()*_data.size2()], 
																			 _data.size1(), _data.size2());
			for(size_t row = 0; row < dataset.size1();row++)
				for(size_t col = 0; col < dataset.size2();col++)
					dataset[row][col] = _data(row,col);
			m_index = new flann::Index(dataset, params, d);
		}
		///free the dataset
		virtual ~index_factory() {
			dataset.free();
		}
		///builds index from dataset
		void build() {
			m_index->buildIndex();
		}
		///operates a batch k nearest neighbours search
		void knn_search(const ublas::matrix<element>& queries, 
										ublas::matrix<int>& indices, 
										ublas::matrix<distance>& dists, int knn, 
										const search_params& params) 
		{
			size_t rows = queries.size1();
			size_t cols = queries.size2();				
			flann::Matrix<element> _queries(element[rows*cols], rows, cols);
			for(size_t row = 0; row < rows;row++)
				for(size_t col = 0; col < cols;col++)
					_queries[row][col] = queries(row,col);

			flann::Matrix<int> _indices(new int[rows*cols], rows, cols);
			flann::Matrix<distance> _dists(new distance[rows*cols], rows, cols);
			m_index->knnSearch(_queries, _indices, _dists, knn, params);

			for(size_t row = 0; row < rows;row++) {
				for(size_t col = 0; col < cols;col++) {
					dists(row,col) = _dists[row][col];
					indices(row,col) = _indices[row][col];
				}
			}
			_queries.free();
			_dists.free();
			_indices.free();
		}
		///operates a k nearest neighbours search on a query
		void knn_search(const ublas::vector<element>& query, 
										ublas::vector<int>& indices, ublas::vector<distance>& dists, 
										int knn, const search_params& params) 
		{
			size_t rows = query.size();
			flann::Matrix<element> _query(new element[rows], rows, 1);
			for(size_t row = 0; row < rows;row++)
				_query[row] = query[row];
				
			flann::Matrix<int> _indices(new int[rows], rows, 1);
			flann::Matrix<distance> _dists(new distance[rows], rows, 1);
			m_index->knnSearch(_query, _indices, _dists, knn, params);

			for(size_t row = 0; row < rows;row++) {
				dists[row] = _dists[row];
				indices[row] = _indices[row];
			}

			_query.free();
			_dists.free();
			_indices.free();
		}
		///operates a batch fixed radius search
		int radius_search(const ublas::matrix<element>& query, 
											ublas::matrix<int>& indices, 
											ublas::matrix<distance>& dists, 
											float radius, const search_params& params) {
			size_t rows = queries.size1();
			size_t cols = queries.size2();
			flann::Matrix<element> _queries(new element[rows*cols], rows, cols);
			for(size_t row = 0; row < rows;row++)
				for(size_t col = 0; col < cols;col++)
					_queries[row][col] = queries(row,col);

			flann::Matrix<int> _indices(new int[query.rows*cols], rows, cols);
			flann::Matrix<distance> _dists(new distance[query.rows*cols], rows, cols);
			int result = m_index->radiusSearch(_queries, _indices, _dists, radius, params);

			for(size_t row = 0; row < rows;row++) {
				for(size_t col = 0; col < cols;col++) {
					dists(row,col) = dists[row][col];
					indices(row,col) = indices[row][col];
				}
			}
			_queries.free();
			_dists.free();
			_indices.free();
			return result;
		}
		///operates a radius search on a query
		int radius_search(const ublas::vector<element>& query, 
											ublas::vector<int>& indices, ublas::vector<distance>& dists, 
											float radius, const search_params& params) 
		{
			size_t rows = query.size();
			flann::Matrix<element> _query(new element[rows], rows, 1);
			for(size_t row = 0; row < rows;row++)
				_query[row] = query[row];
				
			flann::Matrix<int> _indices(new int[rows], rows, 1);
			flann::Matrix<distance> _dists(new distance[rows], rows, 1);
			int result = m_index->radiusSearch(_query, _indices, _dists, radius, params);

			for(size_t row = 0; row < rows;row++) {
				dists[row] = _dists[row];
				indices[row] = _indices[row];
			}
			_query.free();
			_dists.free();
			_indices.free();
			return result;
		}

	public:
		///save index to a file
		void save(std::string filename) const
		{
			m_index->save(filename);
		}
		///@return dataset size
		int data_size() const 
		{
			return m_index->veclen();
		}
		///@return size of dataset
		int size() const
		{
			return m_index->size();
		}
		///@return a pointer to flann::NNIndex
		flann::NNIndex<distance>* index() 
		{ 
			return m_index->nnIndex; 
		}
		///@return a pointer to flann::IndexParams
		const flann::IndexParams* parameters() { 
			return m_index->nnIndex->getParameters(); 
		}
	};
		
	// template<typename DISTANCE>
	// class Index {
	// protected :
	// 	SearchParams params;
	// 	index_factory<DISTANCE> m_index;
	// public:
	// 	Index(const ublas::matrix<element>& dataset, IndexParams*)
	// 	void knn_search(const ublas::vector<element>& query, int knn,
	// 									ublas::vector<int>& indices, ublas::vector<distance>& dists) 
	// 	{
	// 		m_index.knn_search(query, knn, indices, dists, params);
	// 	}
	// 	void knn_search(const ublas::matrix<element>& query, int knn,
	// 								 ublas::matrix<int>& indices, ublas::matrix<distance>& dists) 
	// 	{
	// 		m_index.knn_search(query, knn, indices, dists, params);
	// 	}
	// 	int radius_search(const ublas::vector<element>& query, float radius,
	// 								 ublas::vector<int>& indices, ublas::vector<distance>& dists)
	// 	{
	// 		m_index.radius_search(query, radius, indices, dists, params);
	// 	}
	// 	int radius_search(const ublas::matrix<element>& query, float radius,
	// 								 ublas::matrix<int>& indices, ublas::matrix<distance>& dists) 
	// 	{
	// 		m_index.radius_search(query, radius, indices, dists, params);
	// 	}
	// 	virtual ~Index() {}
	// }
		
	/** Class linear_index
	 * specified index for linear search
	 */
	template<typename DISTANCE>
	class linear_index : public index_factory<DISTANCE> {
	public:
		linear_index(const ublas::matrix<T>& dataset) : 
			index_factory<DISTANCE>(dataset, flann::LinearIndexParams()) {}
	};
		
	/** Class KD_tree_index
	 * specified index for KD tree search algorithm
	 */
	template<typename DISTANCE>
	class KD_tree_index : public index_factory<DISTANCE> {
	public:
		/// @param nb_trees: number of trees to be constructed
		KD_tree_index(const ublas::matrix<T>& dataset, int nb_trees = 4) : 
			index_factory<DISTANCE>(dataset, 
															flann::KDTreeIndexParams(nb_trees)) {}
	};
		
	/** Class K_means_index
	 * specified index for K means search algorithm
	 */
	template<typename DISTANCE>
	class K_means_index : public index_factory<DISTANCE> {
	public:
		/**  
		 * @param branching: branching factor
		 * @param iterations: max iterations to perform in one kmeans clustering
		 * @param init: algorithm used for picking the initial cluster centers
		 * @param cb_index: cluster boundary index. 
		 */	
		K_means_index(const ublas::matrix<T>& dataset,
									int branching = 32, int iterations = 11, 
									centers_init init = CENTERS_RANDOM, 
									float cb_index = 0.2 ) :
			index_factory<DISTANCE>(dataset, 
															flann::KMeansIndexParams(branching, iterations, 
																											 centersinit, cbindex)) {}
	};

	/** Class composite_index
	 * specified index for a composite K-Means KD-Tree search algorithm
	 */
	template<typename DISTANCE>
	class composite_index : public index_factory<DISTANCE> {
		/**
		 * @param trees number of randomized trees to use (for kdtree)
		 * @param branching branching factor (for kmeans tree)
		 * @param iterations max iterations to perform in one kmeans clustering (kmeans tree)
		 * @param centers_init algorithm used for picking the initial cluster centers for kmeans tree
		 * @param cb_index cluster boundary index. Used when searching the kmeans tree.
		 */
		composite_index(const ublas::matrix<T>& dataset,
										int trees = 4, int branching = 32, int iterations = 11,
										centers_init init = CENTERSRANDOM, float cb_index = 0.2 ) :
			index_factory<DISTANCE>(dataset, 
															flann::CompositeIndexParams(trees, branching,
																													iterations, init,
																													cb_index)) {}
	};

	/** Class autotuned_index
	 * specified index for an autotuned index search algorithm
	 */
	template<typename DISTANCE>
	class autotuned_index : public index_factory<DISTANCE> {
	public:
		/**
		 * @param target_precision: precision desired
		 * @param build_weight: build tree time weighting factor
		 * @param memory_weight: index memory weighting factor
		 * @param sample_fraction: what fraction of the dataset to use for autotuning
		 */
		autotuned_index(const ublas::matrix<T>& dataset, 
										float target_precision = 0.9, float build_weight = 0.01,
										float memory_weight = 0, float sample_fraction = 0.1) :		
		
			index_factory<DISTANCE>(dataset,
															flann::AutotunedIndexParams(target_precision, build_weight,
																													memory_weightm, sample_fraction)) {}
	};
	
	// template<typename DISTANCE>
	// class saved_index : public index_factory<DISTANCE> {
	// 	/**
	// 	 * @param filename: file where the index was stored
	// 	 */
	// 	 saved_index(const std::string& filename) 

	// };
	/** Class search_params holds the search parameters
	 */
	class search_params : public flann::SearchParams {
	public:
		/**
		 * @param checks: how many leafs to visit when searching for neighbours (-1 for unlimited)
		 * @param eps: search for eps-approximate neighbours (default: 0)
		 * @param sorted: only for radius search, require neighbours sorted by distance (default: true)
		 */
		search_params(int checks = 32, float eps = 0, bool sorted = true ) :
			flann::SearchParams(checks, eps, sorted){}		
	};

} // namespace jann

/*@}*/	
/* End of Doxygen group */

#endif // HAVE_FLANN
#endif // JMATH_JANN_HPP
