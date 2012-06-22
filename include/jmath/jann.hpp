/** @file jann.hpp
 * This is the standard header defined in jafar to bind the flann library
 * @ingroup jmath
 */

#ifndef JMATH_JANN_HPP
#define JMATH_JANN_HPP

#include "jafarConfig.h"

#ifdef HAVE_FLANN

#include "jmath/jmathException.hpp"

#include <flann/flann.hpp>
#include <boost/numeric/ublas/vector.hpp>
#include <boost/numeric/ublas/matrix.hpp>

/** \addtogroup jmath */
/*@{*/

/// shortcut for ublas namespace
namespace ublas = boost::numeric::ublas;

/// special nemespace to wrap flann library
namespace jann {
	// /// algorithm to be used for research, use only if you want to create a new Index
	// enum algorithm {
	// 	LINEAR        = 0,
	// 	KDTREE        = 1,
	// 	KMEANS        = 2,
	// 	COMPOSITE     = 3,
	// 	KDTREE_SINGLE = 4,
	// 	SAVED         = 254,
	// 	AUTOTUNED     = 255
	// };
	// /// algorithm to initialize centers for the K Means algorithm
	// enum centers_init {
	// 	CENTERS_RANDOM   = 0,
	// 	CENTERS_GONZALES = 1,
	// 	CENTERS_KMEANSPP = 2
	// };
	// /// determine log level
	// enum log_level {
	// 	LOG_NONE  = 0,
	// 	LOG_FATAL = 1,
	// 	LOG_ERROR = 2,
	// 	LOG_WARN  = 3,
	// 	LOG_INFO  = 4
	// };
	// /// supported distances
	// enum distance {
	// 	EUCLIDEAN        = 1,
	// 	MANHATTAN        = 2,
	// 	MINKOWSKI        = 3,
	// 	MAX_DIST         = 4,
	// 	HIST_INTERSECT   = 5,
	// 	HELLINGER        = 6,
	// 	CS               = 7,
	// 	CHI_SQUARE       = 7,
	// 	KL               = 8,
	// 	KULLBACK_LEIBLER = 8
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

	/** Class index_factory
	 * base class for all searching indexes. You should not used directly unless you want
	 * to create a new index searching algorithm.
	 * @TODO add try/catch statements
	 */
	template<typename D>
	class index_factory 
	{
		flann::Index<D> *m_index;
		///converts from flann matrix to ublas matrix
		template<typename T>
		inline void convert(const flann::Matrix<T>& flann_mat, ublas::matrix<T>& ublas_mat){
			JFR_ASSERT(((flann_mat.rows == ublas_mat.size1()) && 
									(flann_mat.cols == ublas_mat.size2())),
								 "ublas matrix and flann matrix need to have the same sizes");
			for(size_t r = 0; r < flann_mat.rows; r++)
				for(size_t c = 0; c < flann_mat.cols; c++)
					ublas_mat(r,c) = flann_mat[r][c];
		}
		///converts from ublas matrix to flann matrix
		template<typename T>
		inline void convert(const ublas::matrix<T>& ublas_mat, flann::Matrix<T>& flann_mat){
			JFR_ASSERT(((flann_mat.rows == ublas_mat.size1()) && 
									(flann_mat.cols == ublas_mat.size2())),
								 "ublas matrix and flann matrix need to have the same sizes");
			for(size_t r = 0; r < flann_mat.rows; r++)
				for(size_t c = 0; c < flann_mat.cols; c++)
					flann_mat[r][c] = ublas_mat(r,c);
		}
		/**converts from single row flann matrix to ublas vector
		 * @ublas_vec will be resized to @flann_mat columns number if greater
		 */
		template<typename T>
		inline void convert(const flann::Matrix<T>& flann_mat, ublas::vector<T>& ublas_vec){
			JFR_ASSERT(((flann_mat.rows == 1) && (flann_mat.cols >= ublas_vec.size())),
								 "ublas vector and flann matrix rows need to have the same sizes");
			if(flann_mat.cols > ublas_vec.size())
				ublas_vec.resize(flann_mat.cols);
			for(size_t counter = 0; counter < ublas_vec.size(); counter++)
					ublas_vec[counter] = flann_mat[0][counter];
		}
		///converts from ublas vector to single row flann matrix 
		template<typename T>
		inline void convert(const ublas::vector<T>& ublas_vec, flann::Matrix<T>& flann_mat){
			JFR_ASSERT(((flann_mat.rows == 1) && (flann_mat.cols >= ublas_vec.size())),
								 "ublas vector and flann matrix rows need to have the same sizes");
				for(size_t counter = 0; counter < ublas_vec.size(); counter++)
					flann_mat[0][counter] = ublas_vec[counter];
		}
		///converts from std vector to single row flann matrix 
		template<typename T>
		inline void convert(const std::vector<T>& std_vec, flann::Matrix<T>& flann_mat){
			JFR_ASSERT(((flann_mat.rows == 1) && (flann_mat.cols >= std_vec.size())),
								 "std vector and flann matrix rows need to have the same sizes");
				for(size_t counter = 0; counter < std_vec.size(); counter++)
					flann_mat[0][counter] = std_vec[counter];
		}
	public:
		///element type of D
		typedef typename D::ElementType element;
		///result type of D
		typedef typename D::ResultType result;
		///dataset stored as a flann::Matrix
		flann::Matrix<element> dataset;
		///empty constructor
		index_factory() {}
		/**  
		 * @param _data: dataset in ublas::matrix format
		 * @param params: index parameters as specified in flann manual
		 * @param d: flann::distance structure
		 */		
		void operator()(const ublas::matrix<typename D::ElementType>& _data, 
										const flann::IndexParams& params, D d = D() )
		{
			dataset = flann::Matrix<element>(new element[_data.size1()*_data.size2()], 
																			 _data.size1(), _data.size2());
			convert(_data, dataset);
			m_index = new flann::Index<D>(dataset, params, d);
		}
		/**  
		 * @param _data: dataset in ublas::matrix format
		 * @param params: index parameters as specified in flann manual
		 * @param d: flann::distance structure
		 */
		index_factory(const ublas::matrix<typename D::ElementType>& _data, 
									const flann::IndexParams& params, D d = D() )
		{
			dataset = flann::Matrix<element>(new element[_data.size1()*_data.size2()], 
																			 _data.size1(), _data.size2());
			convert(_data, dataset);
			m_index = new flann::Index<D>(dataset, params, d);
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
		void knn_search(const ublas::matrix<typename D::ElementType>& queries, 
										ublas::matrix<int>& indices, 
										ublas::matrix<result>& dists, int knn, 
										const search_params& params) 
		{
			size_t rows = queries.size1();
			size_t cols = queries.size2();
			JFR_PRED_ERROR(cols == dataset.cols,
										 jafar::jmath::JmathException,
										 jafar::jmath::JmathException::WRONG_SIZE,
										 "queries and dataset need to have same columns size")
			JFR_PRED_ERROR(((indices.size2() >= (size_t)knn) && 
											(dists.size2() >= (size_t)knn)),
										 jafar::jmath::JmathException,
										 jafar::jmath::JmathException::WRONG_SIZE,
										 "indices and dists must have at least "<<knn<<" columns")
			JFR_PRED_ERROR(((indices.size1() == rows) && (dists.size1() == rows)),
										 jafar::jmath::JmathException,
										 jafar::jmath::JmathException::WRONG_SIZE,
										 "queries, indices and dists need to be of same row size")
			flann::Matrix<element> _queries(new element[rows*cols], rows, cols);
			convert(queries, _queries);
			flann::Matrix<int> _indices(new int[rows*indices.size2()], rows, indices.size2());
			flann::Matrix<result> _dists(new result[rows*dists.size2()], rows, dists.size2());
			m_index->knnSearch(_queries, _indices, _dists, knn, params);
			convert(_indices, indices);
			convert(_dists, dists);

			_queries.free();
			_dists.free();
			_indices.free();
		}
		///operates a k nearest neighbours search on a query
		void knn_search(const ublas::vector<element>& query, 
										ublas::vector<int>& indices, ublas::vector<result>& dists, 
										int knn, const search_params& params) 
		{
			size_t length = query.size();
			JFR_PRED_ERROR(length == dataset.cols,
										 jafar::jmath::JmathException,
										 jafar::jmath::JmathException::WRONG_SIZE,
										 "query size must be of dataset columns size")
				JFR_PRED_ERROR(((indices.size() >= size_t(knn)) && (dists.size() >= size_t(knn))),
										 jafar::jmath::JmathException,
										 jafar::jmath::JmathException::WRONG_SIZE,
										 "indices and dists must be at least of size "<<knn)
			flann::Matrix<element> _query(new element[length], 1, length);
			convert(query,_query);
			flann::Matrix<int> _indices(new int[indices.size()], 1, indices.size());
			flann::Matrix<result> _dists(new result[dists.size()], 1, dists.size());
			m_index->knnSearch(_query, _indices, _dists, knn, params);
			convert(_indices, indices);
			convert(_dists, dists);

			_query.free();
			_dists.free();
			_indices.free();
		}
		///operates a k nearest neighbours search on a query
		void knn_search(const std::vector<element>& query, 
										std::vector<int>& indices, std::vector<result>& dists, 
										int knn, const search_params& params) 
		{
			size_t length = query.size();
			JFR_PRED_ERROR(length == dataset.cols,
										 jafar::jmath::JmathException,
										 jafar::jmath::JmathException::WRONG_SIZE,
										 "query size must be of dataset columns size")
				JFR_PRED_ERROR(((indices.size() >= size_t(knn)) && (dists.size() >= size_t(knn))),
										 jafar::jmath::JmathException,
										 jafar::jmath::JmathException::WRONG_SIZE,
										 "indices and dists must be at least of size "<<knn)
			flann::Matrix<element> _query(new element[length], 1, length);
			convert(query,_query);
			flann::Matrix<int> _indices(new int[indices.size()], 1, indices.size());
			flann::Matrix<result> _dists(new result[dists.size()], 1, dists.size());
			m_index->knnSearch(_query, _indices, _dists, knn, params);
			convert(_indices, indices);
			convert(_dists, dists);

			_query.free();
			_dists.free();
			_indices.free();
		}
		/**operates a radius search on a query @return the number of neighbours 
		 * within the search radius
		 */
		int radius_search(const ublas::vector<element>& query, 
											ublas::matrix<int>& indices, 
											ublas::matrix<result>& dists, float radius, 
											const search_params& params) 
		{
			size_t length = query.size();
			JFR_PRED_ERROR(length == dataset.cols,
										 jafar::jmath::JmathException,
										 jafar::jmath::JmathException::WRONG_SIZE,
										 "query length and dataset columns must be equal")
			JFR_PRED_ERROR((indices.size2() == dists.size2()),
										 jafar::jmath::JmathException,
										 jafar::jmath::JmathException::WRONG_SIZE,
										 "indices and dists must have same columns number")
			flann::Matrix<element> _query(new element[length], 1, length);
			convert(query, _query);
			flann::Matrix<int> _indices(new int[indices.size1()*indices.size2()], indices.size1(), indices.size2());
			flann::Matrix<result> _dists(new result[dists.size1()*dists.size2()], dists.size1(), dists.size2());
			m_index->radiusSearch(_query, _indices, _dists, radius, params);
			convert(_indices, indices);
			convert(_dists, dists);

			_query.free();
			_dists.free();
			_indices.free();
		}
		/**operates a radius search on a query @return the number of neighbours 
		 * within the search radius
		 */
		int radius_search(const ublas::vector<element>& query,
											ublas::vector<int>& indices, ublas::vector<result>& dists, 
											float radius, const search_params& params) 
		{
			size_t length = query.size();
			JFR_PRED_ERROR(length == dataset.cols,
										 jafar::jmath::JmathException,
										 jafar::jmath::JmathException::WRONG_SIZE,
										 "query size must be of dataset columns size")
				JFR_PRED_ERROR((indices.size() == dists.size()),
										 jafar::jmath::JmathException,
										 jafar::jmath::JmathException::WRONG_SIZE,
										 "indices and dists must have same size")
			flann::Matrix<element> _query(new element[length], 1, length);
			convert(query,_query);
			flann::Matrix<int> _indices(new int[indices.size()], 1, indices.size());
			flann::Matrix<result> _dists(new result[dists.size()], 1, dists.size());
			int result = m_index->radiusSearch(_query, _indices, _dists, radius, params);
			convert(_indices, indices);
			convert(_dists, dists);

			_query.free();
			_dists.free();
			_indices.free();
			return result;
		}
		/**operates a radius search on a query @return the number of neighbours 
		 * within the search radius
		 */
		int radius_search(const std::vector<element>& query,
											std::vector<int>& indices, std::vector<result>& dists, 
											float radius, const search_params& params) 
		{
			size_t length = query.size();
			JFR_PRED_ERROR(length == dataset.cols,
										 jafar::jmath::JmathException,
										 jafar::jmath::JmathException::WRONG_SIZE,
										 "query size must be of dataset columns size")
				JFR_PRED_ERROR((indices.size() == dists.size()),
										 jafar::jmath::JmathException,
										 jafar::jmath::JmathException::WRONG_SIZE,
										 "indices and dists must have same size")
			flann::Matrix<element> _query(new element[length], 1, length);
			convert(query,_query);
			flann::Matrix<int> _indices(new int[indices.size()], 1, indices.size());
			flann::Matrix<result> _dists(new result[dists.size()], 1, dists.size());
			int result = m_index->radiusSearch(_query, _indices, _dists, radius, params);
			convert(_indices, indices);
			convert(_dists, dists);

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
		flann::NNIndex<result>* index() 
		{ 
			return m_index->nnIndex; 
		}
		///@return a pointer to flann::IndexParams
		const flann::IndexParams* parameters() { 
			return m_index->nnIndex->getParameters(); 
		}
	};
		
	// template<typename D>
	// class Index {
	// protected :
	// 	SearchParams params;
	// 	index_factory<D> m_index;
	// public:
	// 	Index(const ublas::matrix<typename D::ElementType>& dataset, IndexParams*)
	// 	void knn_search(const ublas::vector<element>& query, int knn,
	// 									ublas::vector<int>& indices, ublas::vector<result>& dists) 
	// 	{
	// 		m_index.knn_search(query, knn, indices, dists, params);
	// 	}
	// 	void knn_search(const ublas::matrix<typename D::ElementType>& query, int knn,
	// 								 ublas::matrix<int>& indices, ublas::matrix<result>& dists) 
	// 	{
	// 		m_index.knn_search(query, knn, indices, dists, params);
	// 	}
	// 	int radius_search(const ublas::vector<element>& query, float radius,
	// 								 ublas::vector<int>& indices, ublas::vector<result>& dists)
	// 	{
	// 		m_index.radius_search(query, radius, indices, dists, params);
	// 	}
	// 	int radius_search(const ublas::matrix<typename D::ElementType>& query, float radius,
	// 								 ublas::matrix<int>& indices, ublas::matrix<result>& dists) 
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
		linear_index(const ublas::matrix<typename DISTANCE::ElementType>& dataset) : 
			index_factory<DISTANCE>(dataset, flann::LinearIndexParams()) {}
	};
		
	/** Class KD_tree_index
	 * specified index for KD tree search algorithm
	 */
	template<typename DISTANCE>
	class KD_tree_index : public index_factory<DISTANCE> {
	public:
		/// @param nb_trees: number of trees to be constructed
		KD_tree_index(const ublas::matrix<typename DISTANCE::ElementType>& dataset, 
									int nb_trees = 4) : 
			index_factory<DISTANCE>(dataset, flann::KDTreeIndexParams(nb_trees)) {}
		KD_tree_index() {}
		void operator() (const ublas::matrix<typename DISTANCE::ElementType>& dataset, 
										 int nb_trees = 4) {
			index_factory<DISTANCE>::operator()(dataset, flann::KDTreeIndexParams(nb_trees));
		}
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
		K_means_index(const ublas::matrix<typename DISTANCE::ElementType>& dataset,
									int branching = 32, int iterations = 11, 
									flann::flann_centers_init_t init = flann::CENTERS_RANDOM, 
									float cb_index = 0.2 ) :
			index_factory<DISTANCE>(dataset, 
															flann::KMeansIndexParams(branching, iterations, 
																											 init, cb_index)) {}
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
		composite_index(const ublas::matrix<typename DISTANCE::ElementType>& dataset,
										int trees = 4, int branching = 32, int iterations = 11,
										flann::flann_centers_init_t init = flann::CENTERS_RANDOM, float cb_index = 0.2 ) :
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
		autotuned_index(const ublas::matrix<typename DISTANCE::ElementType>& dataset, 
										float target_precision = 0.9, float build_weight = 0.01,
										float memory_weight = 0, float sample_fraction = 0.1) :		
		
			index_factory<DISTANCE>(dataset,
															flann::AutotunedIndexParams(target_precision, build_weight,
																													memory_weight, sample_fraction)) {}
	};
	
	// template<typename DISTANCE>
	// class saved_index : public index_factory<DISTANCE> {
	// 	/**
	// 	 * @param filename: file where the index was stored
	// 	 */
	// 	 saved_index(const std::string& filename) 

	// };

} // namespace jann

/*@}*/	
/* End of Doxygen group */

#endif // HAVE_FLANN
#endif // JMATH_JANN_HPP
