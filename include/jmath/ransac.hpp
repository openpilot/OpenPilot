/**************************************************************************
 This file is kind of copy paste from the MRPT project http://www.mrpt.org.
 From MRPT: the C++ port "is highly inspired on Peter Kovesi's MATLAB 
 scripts (http://www.csse.uwa.edu.au/~pk)"
***************************************************************************/
#ifndef  JMATH_RANSAC_HPP
#define  JMATH_RANSAC_HPP
#include "jmath/uniform_random.hpp"

namespace jafar {
	namespace jmath {
		template <typename NUMTYPE = double>
			/** An implementation of the RANSAC algorithm for robust fitting of models to data.
			 * @ingroup jmath
			 */
		class RANSAC {
		public:

			/** The type of the fitting function passed to jafar::jmath::ransac */
			typedef void (*RANSAC_fit_functor)(const ublas::matrix<NUMTYPE>  &data,
																				const std::vector<size_t> &indices,
																				ublas::vector< ublas::matrix<NUMTYPE> > &fit_models);

			/** The type of the distance function passed to jafar::jmath::ransac */
			typedef void (*RANSAC_distance_functor)(const ublas::matrix<NUMTYPE>  &data,
																						 const ublas::vector< ublas::matrix<NUMTYPE> > & test_models,
																						 NUMTYPE   distance_threshold,
																						 unsigned int & best_model_index,
																						 std::vector<size_t>& inlier_indices );

			/** The type of the function passed to mrpt::math::ransac */
			typedef bool (*RANSAC_degenerate_functor)(const ublas::matrix<NUMTYPE>  &data,
																							 const std::vector<size_t> &indices );

			/** execution task
			 *
			 *  \param data A DxN matrix with all the observed data. D is the dimensionality of data points and N the number of points.
			 *  \param
			 *
			 *  This implementation is highly inspired on Peter Kovesi's MATLAB scripts 
			 * (http://www.csse.uwa.edu.au/~pk).
			 * \return false if no good solution can be found, true on success.
			 */
			static bool execute(const ublas::matrix<NUMTYPE>	  &data,
													RANSAC_fit_functor			    fit_func,
													RANSAC_distance_functor  	dist_func,
													RANSAC_degenerate_functor 	degen_func,
													const NUMTYPE   				    distance_threshold,
													const unsigned int			  min_size_samples_to_fit,
													std::vector<size_t>			  &out_best_inliers,
													ublas::matrix<NUMTYPE>    &out_best_model,
													const NUMTYPE              prob_good_sample = 0.999,
													const size_t				      max_iter = 200000) 
			{
				JFR_ASSERT(min_size_samples_to_fit>=1, 
									 "minimum size of samples to fit must be greater than 0")
					const size_t D = data.size1();  //  dimensionality
				const size_t nb_pts = data.size2();
				
				JFR_ASSERT(D>=1, "data diemension must be greater than 0");
				JFR_ASSERT(nb_pts>1, "number of data points must be at least 2");
				// Maximum number of attempts to select a non-degenerate data set.
				const size_t maxDataTrials = 100; 
				// Sentinel value allowing detection of solution failure.
				out_best_model.resize(0,0);  

				size_t trialcount = 0;  //
				size_t bestscore =  0;
				size_t N = 1;     // Dummy initialisation for number of trials.
				
				std::vector<size_t>   ind( min_size_samples_to_fit );
				
				while (N > trialcount) {
					// Select at random s datapoints to form a trial model, M.
					// In selecting these points we have to check that they are not in
					// a degenerate configuration.
					bool degenerate=true;
					size_t count = 1;
					ublas::vector< ublas::matrix<NUMTYPE> >  MODELS;

					while (degenerate) {
            // Generate s random indicies in the range 1..npts
            ind.resize( min_size_samples_to_fit );
						
						random::uniform_fill((unsigned int)0, (unsigned int)nb_pts-1, ind);

            // Test that these points are not a degenerate configuration.
            degenerate = degen_func(data, ind);

            if (!degenerate) {
							// Fit model to this random selection of data points.
							// Note that M may represent a set of models that fit the data
							fit_func(data,ind,MODELS);

							// Depending on your problem it might be that the only way you
							// can determine whether a data set is degenerate or not is to
							// try to fit a model and see if it succeeds.  If it fails we
							// reset degenerate to true.
							degenerate = MODELS.empty();
						}

            // Safeguard against being stuck in this loop forever
            if (++count > maxDataTrials) {
							JFR_DEBUG("unable to select a nondegenerate data set")
            }
					}
					JFR_DEBUG("found a non degenerate data set")
					// Once we are out here we should have some kind of model...
					// Evaluate distances between points and model returning the indices
					// of elements in x that are inliers.  Additionally, if M is a cell
					// array of possible models 'distfn' will return the model that has
					// the most inliers.  After this call M will be a non-cell objec
					// representing only one model.
					unsigned int best_model_index = -1;
					std::vector<size_t>   inliers;
					if (!degenerate) {
						dist_func(data,MODELS, distance_threshold, best_model_index, inliers);
						// JFR_ASSERT(((best_model_index < MODELS.size()) && (best_model_index > -1)), 
						// 						"best model index must be in [0.."<<MODELS.size()<<"[");
					}
					JFR_DEBUG("best model index "<<best_model_index)
					// Find the number of inliers to this model.
					const size_t ninliers = inliers.size();
					JFR_DEBUG("nb inliers "<<ninliers)
					if (ninliers > bestscore ) {
						bestscore = ninliers;  // Record data for this model
						out_best_model.resize(MODELS[best_model_index].size1(), MODELS[best_model_index].size2());
						out_best_model    = MODELS[best_model_index];
						out_best_inliers  = inliers;

						// Update estimate of N, the number of trials to ensure we pick,
						// with probability p, a data set with no outliers.
						NUMTYPE fracinliers =  ninliers/static_cast<NUMTYPE>(nb_pts);
						NUMTYPE pNoOutliers = 1 -  pow(fracinliers,static_cast<NUMTYPE>(min_size_samples_to_fit));
					
						pNoOutliers = std::max( std::numeric_limits<NUMTYPE>::epsilon(), pNoOutliers);  // Avoid division by -Inf
						pNoOutliers = std::min(1.0 - std::numeric_limits<NUMTYPE>::epsilon() , pNoOutliers); // Avoid division by 0.
						// Number of
						N = log(1-prob_good_sample)/log(pNoOutliers);
						JFR_DEBUG("iter #"<<(unsigned)trialcount<<" Estimated number of iters: "<<(unsigned)N<<" pNoOutliers = "<<pNoOutliers<<" #inliers: "<<(unsigned)ninliers)
					}

					++trialcount;

					JFR_DEBUG("trial "<<(unsigned int)trialcount<<" out of "<<(unsigned int)ceil(static_cast<double>(N)))

					// Safeguard against being stuck in this loop forever
					if (trialcount > max_iter) {
						JFR_DEBUG("Warning: maximum number of trials ("<<max_iter<<") reached")
						break;
					}
				}

				if (out_best_model.size1()>0) {  // We got a solution
					JFR_DEBUG("Finished in "<<(unsigned)trialcount<<" iterations")
					return true;
				}
				else {
					JFR_DEBUG("Warning: Finished without any proper solution.")
					return false;
				}
			}
		}; // end class
		/// The default instance of RANSAC, for double type
		typedef RANSAC<double> ransac;   

	} // end of namespace jmath
} // end of namespace jafar

#endif
