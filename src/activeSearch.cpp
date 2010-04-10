/**
 * \file activeSearch.cpp
 *
 *  Created on: 10/04/2010
 *     \author: jsola@laas.fr
 *
 *
 *  ## Add a description here ##
 *
 * \ingroup rtslam
 */

#include "rtslam/activeSearch.hpp"

namespace jafar {
	namespace rtslam {
		using namespace std;
		using namespace jblas;

		ostream& operator <<(ostream & s, ActiveSearchGrid & grid) {
			s << "Active search grid." << endl;
			s << "feature count: " << grid.projectionsCount << endl;
			return s;
		}


		// CLASS ActiveSearchGrid
		ActiveSearchGrid::ActiveSearchGrid(const int & imgSize_h, const int & imgSize_v, const int & nCells_h,
		    const int & nCells_v, const int & separation) :
			projectionsCount(nCells_h + 1, nCells_v + 1),
			roiGuard(separation)
		{
			imgSize(0) = imgSize_h;
			imgSize(1) = imgSize_v;
			gridSize(0) = projectionsCount.size1();
			gridSize(1) = projectionsCount.size2(), cellSize(0) = imgSize_h / nCells_h;
			cellSize(1) = imgSize_v / nCells_v;
			gridOffset = -cellSize / 2;
			renew();
		}


		// Functions to fill in cells
		void ActiveSearchGrid::addPixel(const vec2 & exp) {
			addToCell(pix2cell(exp));
		}
		void ActiveSearchGrid::addToCell(const veci2 & cell) {
			projectionsCount(cell(0), cell(1))++;
		}

		void ActiveSearchGrid::clear() {
			projectionsCount.clear();
		}
		void ActiveSearchGrid::renew() {
			gridOffset(0) = (int) ((double) rand() / RAND_MAX * cellSize(0));
			gridOffset(1) = (int) ((double) rand() / RAND_MAX * cellSize(1));
			clear();
		}

		/**
		 * Get one empty cell
		 */
		bool ActiveSearchGrid::pickEmptyCell(veci2 & cell) {
			int k = 0;
			mati M((gridSize(0)-2) * (gridSize(1)-2), 2);
			M.clear();
			for (int i = 1; i < gridSize(0) -1; i++) {
				for (int j = 1; j < gridSize(1)-1; j++) {
					if (projectionsCount(i, j) == 0) {
						M(k, 0) = i;
						M(k, 1) = j;
						k++;
					}
				}
			}
			if (k > 0) {
				int n = (double) rand() / RAND_MAX * k; // number of empty inner cells
				cell(0) = M(n, 0);
				cell(1) = M(n, 1);
				return true;
			}
			else
				return false;
		}


		/*
		 * Get cell origin (exact pixel)
		 */
		veci2 ActiveSearchGrid::cellOrigin(const veci2 & cell) {
			veci2 cell0;
			cell0(0) = gridOffset(0) + cellSize(0) * cell(0);
			cell0(1) = gridOffset(1) + cellSize(1) * cell(1);
			return cell0;
		}


		/*
		 * Get cell center (can be decimal if size of cell is an odd number of pixels)
		 */
		vec2 ActiveSearchGrid::cellCenter(const veci2 & cell) {
			return cellOrigin(cell) + cellSize / 2;
		}

		void ActiveSearchGrid::cell2roi(const veci2 & cell, ROI & roi) {
			veci2 ul = cellOrigin(cell);
			ul(0) += roiGuard;
			ul(1) += roiGuard;
			roi.upleft() = ul;
			veci2 s = cellSize;
			s(0) -= 2 * roiGuard;
			s(1) -= 2 * roiGuard;
			roi.size() = s;
		}

		/**
		 * Get ROI of one random empty cell
		 */
		bool ActiveSearchGrid::pickROI(ROI & roi) {
			veci2 cell;
			if (pickEmptyCell(cell)) {
				cell2roi(cell, roi);
				return true;
			}
			else
				return false;
		}

	}
}
