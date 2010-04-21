/**
 * \file activeSearch.cpp
 *
 * \date 10/04/2010
 * \author jsola@laas.fr
 *
 *
 *  ## Add a description here ##
 *
 * \ingroup rtslam
 */

#include "rtslam/activeSearch.hpp"

#include "rtslam/observationAbstract.hpp"
#include "rtslam/sensorAbstract.hpp"

namespace jafar {
	namespace rtslam {
		using namespace std;
		using namespace jblas;

		///////////////////////////////////////////
		// ACTIVE SEARCH TESSELATION GRID
		///////////////////////////////////////////

		ostream& operator <<(ostream & s, ActiveSearchGrid & grid) {
			s << "feature count: " << grid.projectionsCount;
			return s;
		}


		// CLASS ActiveSearchGrid
		ActiveSearchGrid::ActiveSearchGrid(const int & imgSize_h, const int & imgSize_v, const int & nCells_h,
		    const int & nCells_v, const int & separation) :
			projectionsCount(nCells_h + 1, nCells_v + 1), emptyCellsTile_tmp((nCells_h + 1) * (nCells_v + 1), 2), separation(
			    separation) {
			imgSize(0) = imgSize_h;
			imgSize(1) = imgSize_v;
			gridSize(0) = projectionsCount.size1();
			gridSize(1) = projectionsCount.size2(), cellSize(0) = imgSize_h / nCells_h;
			cellSize(1) = imgSize_v / nCells_v;
			offset = -cellSize / 2;
			renew();
		}


		// Functions to fill in cells
		void ActiveSearchGrid::addPixel(const vec2 & p) {
			addToCell(pix2cell(p));
		}
		void ActiveSearchGrid::addToCell(const veci2 & cell) {
			projectionsCount(cell(0), cell(1))++;
		}

		void ActiveSearchGrid::clear() {
			projectionsCount.clear();
		}
		void ActiveSearchGrid::renew() {
			offset(0) = rand() % cellSize(0); // from 0 to cellSize(0)-1
			offset(1) = rand() % cellSize(1); // from 0 to cellSize(0)-1
			//			offset(0) = (int) ((double) rand() / RAND_MAX * cellSize(0));
			//			offset(1) = (int) ((double) rand() / RAND_MAX * cellSize(1));
			clear();
		}


		/*
		 * Get one empty cell
		 */
		bool ActiveSearchGrid::pickEmptyCell(veci2 & cell) {
			int k = 0;
			veci2 cell0;
			for (int i = 1; i < gridSize(0) - 1; i++) {
				for (int j = 1; j < gridSize(1) - 1; j++) {
					cell0(0) = i;
					cell0(1) = j;
					if (isEmpty(cell0)) {
						ublas::row(emptyCellsTile_tmp, k) = cell0;
						k++;
					}
				}
			}
			if (k > 0) { // number of empty inner cells
				//				int idx = (double) rand() / RAND_MAX * k;
				int idx = rand() % k; // between 0 and k-1
				cell(0) = emptyCellsTile_tmp(idx, 0);
				cell(1) = emptyCellsTile_tmp(idx, 1);
				return true;
			}
			else
				return false;
		}

		bool ActiveSearchGrid::isEmpty(const veci2 & cell) {
			return (projectionsCount(cell(0), cell(1)) == 0);
		}


		/*
		 * Get cell origin (exact pixel)
		 */
		veci2 ActiveSearchGrid::cellOrigin(const veci2 & cell) {
			veci2 cell0;
			cell0(0) = offset(0) + cellSize(0) * cell(0);
			cell0(1) = offset(1) + cellSize(1) * cell(1);
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
			ul(0) += separation;
			ul(1) += separation;
			roi.upleft(ul);
			veci2 s = cellSize;
			s(0) -= 2 * separation;
			s(1) -= 2 * separation;
			roi.size(s);
		}


		/**
		 * Get ROI of one random empty cell
		 */
		bool ActiveSearchGrid::getROI(ROI & roi) {
			veci2 cell;
			if (pickEmptyCell(cell)) {
				cell2roi(cell, roi);
				return true;
			}
			else
				return false;
		}


		////////////////////////////////////////////////////////
		//    ACTIVE SEARCH ALGORITHMS
		////////////////////////////////////////////////////////

		map<double, observation_ptr_t> ActiveSearch::projectAll(const sensor_ptr_t & senPtr, size_t & numVis) {
			map<double, observation_ptr_t> visObs;
			for (SensorAbstract::ObservationList::iterator obsIter = senPtr->observationList().begin(); obsIter
			    != senPtr->observationList().end(); obsIter++) {
				observation_ptr_t obsPtr = *obsIter;
				obsPtr->project();
				obsPtr->predictVisibility();
				if (obsPtr->isVisible()) {
					obsPtr->predictInfoGain();
					visObs[obsPtr->expectation.infoGain] = obsPtr; // this automatically sorts the observations ! ;-)
				}
			}
			return visObs;
		}

		void ActiveSearch::predictApp(const observation_ptr_t & obsPtr) {

			// Get landmark descriptor
			landmark_ptr_t lmkPtr = obsPtr->landmarkPtr();

			// Get the sensor's current global pose
			vec7 senPose = obsPtr->sensorPtr()->globalPose();
		}

		void ActiveSearch::scanObs(const observation_ptr_t & obsPtr, const ROI & roi) {
		}

	}
}
