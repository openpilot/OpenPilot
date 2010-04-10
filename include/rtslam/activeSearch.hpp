/**
 * \file activeSearch.hpp
 *
 *  Created on: 10/04/2010
 *     \author: jsola@laas.fr
 *
 *
 *  ## Add a description here ##
 *
 * \ingroup rtslam
 */

#ifndef ACTIVESEARCH_HPP_
#define ACTIVESEARCH_HPP_

#include "jmath/random.hpp"
#include "rtslam/gaussian.hpp"
#include "jmath/jblas.hpp"
#include <iostream>

namespace jafar {
	namespace rtslam {
		using namespace jblas;
		using namespace std;

		class ROI {
			public:
				veci2 upleft_;
				veci2 size_;
				ROI(const int ulx = 0, const int uly = 0, const int sx = 0, const int sy = 0) {
					upleft_(0) = ulx;
					upleft_(1) = uly;
					size_(0) = sx;
					size_(1) = sy;
				}
				veci2 downright() {
					return upleft_ + size_;
				}
				veci2 & upleft() {
					return upleft_;
				}
				veci2 & size() {
					return size_;
				}

		};

		class ActiveSearchGrid {

				friend ostream& operator <<(ostream & s, ActiveSearchGrid & grid);

			public:
				veci2 imgSize;
				veci2 gridSize;
				veci2 cellSize;
				veci2 gridOffset;
				mati projectionsCount;
				int roiGuard;

			public:
				/**
				 * Constructor.
				 * \param _imgSize_h horizontal image size, in pixels
				 * \param _imgSize_v vertical image size
				 * \param nCells_h horizontal number of cells
				 * \param nCells_v vertical number of cells
				 * \param separation minimum separation between existing and new points
				 */
				ActiveSearchGrid(const int & _imgSize_h, const int & _imgSize_v, const int & nCells_h, const int & nCells_v,
				    const int & separation = 0);

				/**
				 * Clear grid.
				 */
				void clear();

				/**
				 * Create a cleared grid at a new random position
				 */
				void renew();

				/**
				 * Add an expected pixel to the grid
				 */
				void addPixel(const vec2 & pix);

				/**
				 * Get cell corresponding to pixel
				 */
				template<class Vec2>
				veci2 pix2cell(const Vec2 & pix) {
					veci2 cell;
					cell(0) = (pix(0) - gridOffset(0)) / cellSize(0);
					cell(1) = (pix(1) - gridOffset(1)) / cellSize(1);
					return cell;
				}

				/**
				 * Get the  region of interest, reduced by a margin.
				 * This is the size of the cell minus a margin.
				 */
				void cell2roi(const veci2 & cell, ROI & roi);

				/**
				 * Get one empty cell
				 */
				bool pickEmptyCell(veci2 & cell);

				/**
				 * Get cell origin (exact pixel)
				 */
				veci2 cellOrigin(const veci2 & cell);

				/**
				 * Get cell center (can be decimal if size of cell is an odd number of pixels)
				 */
				vec2 cellCenter(const veci2 & cell);

				/**
				 * Get ROI of empty cell
				 */
				bool pickROI(ROI & roi);

				/**
				 * increment cell counter
				 */
				void addToCell(const veci2 & cell);

		};

		class ActiveSearch {
			public:

		};

	}
}

#endif /* ACTIVESEARCH_HPP_ */
