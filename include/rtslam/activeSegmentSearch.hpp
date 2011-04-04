/**
 * \file activeSegmentSearch.hpp
 *
 *  Active search detection and matching for segments.
 *
 * \date 07/03/2011
 * \author bhautboi
 *
 * ## Add detailed description here ##
 *
 * \ingroup rtslam
 */

#ifndef ACTIVESEGMENTSEARCH_HPP
#define ACTIVESEGMENTSEARCH_HPP

#include "jmath/random.hpp"
#include "jmath/jblas.hpp"
#include "image/roi.hpp"

#include "rtslam/gaussian.hpp"
#include "rtslam/rtSlam.hpp"

#include <iostream>

namespace jafar {
   namespace rtslam {
      using namespace jblas;


      /**
       * Active search tesselation grid for segments.
       *
       * \author bhautboi
       *
       * \ingroup rtslam
       */
      class ActiveSegmentSearchGrid {

            friend std::ostream& operator <<(std::ostream & s, ActiveSegmentSearchGrid const & grid);

         private:
            veci2 imgSize;
            veci2 gridSize;
            veci2 cellSize;
            veci2 offset;
            mati projectionsCount;
            mati emptyCellsTile_tmp;
            int separation;
            int margin;

         public:
            /**
             * Constructor.
             * \param imgSize_h horizontal image size, in pixels.
             * \param imgSize_v vertical image size.
             * \param nCells_h horizontal number of cells per image width.
             * \param nCells_v vertical number of cells per image height.
             * \param separation minimum separation between existing and new points.
             */
            ActiveSegmentSearchGrid(const int & imgSize_h, const int & imgSize_v, const int & nCells_h, const int & nCells_v, const int & margin = 0,
                const int & separation = 0);

            /**
             * Clear grid.
             * Sets all cell counters to zero.
             */
            void clear();

            /**
             * Clear grid and position it at a new random location.
             * Sets all cell counters to zero and sets a new random grid position.
             */
            void renew();

            /**
             * Add a projected segment to the grid.
             * \param pix the segment to add.
             */
            void addObs(const vec4 & pix);

            /**
             * Get ROI of a random empty cell.
             * \param roi the resulting ROI
             * \return true if ROI exists.
             */
            bool getRoi(image::ConvexRoi & roi);

            /**
             * Call this after getRoi if no point was found in the roi
             * in order to avoid searching again in it.
             * \param roi the ROI where nothing was found
             */
            void setFailed(const image::ConvexRoi & roi);


         private:
            /**
             * Get cell corresponding to pixel
             */
            template<class Vec>
				veci4 pix2cell(const Vec & pix) {
					veci4 cell;
               cell(0) = (pix(0) - offset(0)) / cellSize(0);
					cell(1) = (pix(1) - offset(1)) / cellSize(1);
					cell(2) = (pix(2) - offset(0)) / cellSize(0);
					cell(3) = (pix(3) - offset(1)) / cellSize(1);
               return cell;
            }

            /**
             * Get cell origin (exact pixel)
             */
            veci2 cellOrigin(const veci2 & cell);

            /**
             * Get cell center (can be decimal if size of cell is an odd number of pixels)
             */
            vec2 cellCenter(const veci2 & cell);

            /**
             * Get one random empty cell
             */
            bool pickEmptyCell(veci2 & cell);

            /**
             * Get the region of interest, reduced by a margin.
             */
            void cell2roi(const veci2 & cell, image::ConvexRoi & roi);

      };

#if 0
      /**
       * Class for active search algorithms.
       * \ingroup rtslam
       */
      class ActiveSearch {
         public:
            vecb visibleObs;
            vecb selectedObs;

            /**
             * Project all landmarks to the sensor space.
             *
             * This function also computes visibility and information gain
             * for each observation.
             * The result is a map of visible observations,
             * ordered from least to most expected information gain.
             *
             * \param senPtr pointer to the sensor under consideration.
             * \return a map of all observations that are visible from the sensor, ordered according to the information gain.
             */
            std::map<double, observation_ptr_t> projectAll(const sensor_ptr_t & senPtr, size_t & numVis);

            /**
             * Predict observed appearance.
             * This function predicts the appearance of the perceived landmark.
             * It does so by computing the appearance of the landmark descriptor from the current sensor position.
             * The result of this operation is an updated observation.
             * \param obsPtr a pointer to the observation.
             */
            void predictApp(const observation_ptr_t & obsPtr);

            /**
             * Scan search region for match.
             */
            void scanObs(const observation_ptr_t & obsPtr, const image::ConvexRoi & roi);
      };
#endif
   }
}

#endif // ACTIVESEGMENTSEARCH_HPP
