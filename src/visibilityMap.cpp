
#include "jmath/angle.hpp"
#include "rtslam/visibilityMap.hpp"
#include "rtslam/observationAbstract.hpp"



namespace jafar {
namespace rtslam {

	std::ostream& operator <<(std::ostream & s, VisibilityMap::Cell const & cell)
	{
		s << "| succ: " << cell.nSuccess << " | fail: " << cell.nFailure << 
			" | lastres: " << cell.lastResult << " | lastframe: " << cell.lastTryFrame << " |";
		return s;
	}
	
	std::ostream& operator <<(std::ostream & s, VisibilityMap const & vismap)
	{
		s << "| vis: " << vismap.lastVis << " | visUncert: " << vismap.lastVisUncert << " | cells: " << vismap.map.size() << " |";
		if (vismap.lastCell) s << *vismap.lastCell;
		return s;
	}

	VisibilityMap::VisibilityMap(double angularRes, double distInit, double distFactor, int nDist, int nCertainty):
		ndist(nDist), distInit(distInit), distFactor(distFactor), nCertainty(nCertainty), lastCell(NULL), lastVis(0.5), lastVisUncert(0.0)
	{
		nang = jmath::round(180.0/angularRes);
	}
	
	VisibilityMap::VisibilityMap():
		ndist(4), distInit(3.0), distFactor(3.0), nCertainty(10), lastCell(NULL), lastVis(0.5), lastVisUncert(0.0)
	{
		double angularRes = 10.0;
		nang = jmath::round(180.0/angularRes);
	}

	
	VisibilityMap::Cell* VisibilityMap::getCell(const observation_ptr_t obsPtr, bool create)
	{
		jblas::vec3 posObs = ublas::subrange(obsPtr->landmarkPtr()->reparametrized(), 0, 3); // FIXME should get some average position instead eg for segments
		jblas::vec3 posSen = ublas::subrange(obsPtr->sensorPtr()->globalPose(), 0, 3);
		return getCell(posSen-posObs, create);
	}

	VisibilityMap::Cell* VisibilityMap::getCell(jblas::vec3 trans, bool create)
	{
		double theta = jmath::radToDeg(std::atan2(trans(0), trans(1)));
		double phi = jmath::radToDeg(std::atan2(trans(2), ublas::norm_2(ublas::subrange(trans,0,2))));
		double r = ublas::norm_2(trans);
		
		int theta_i = (int)((theta+180.)/360. * 2*nang);
		int phi_i = (int)((phi+90.)/180. * nang);
		int r_i = -1;
		double dist = distInit;
		for(int i = 0; i < ndist-1; ++i) { if (r < dist) { r_i = i; break; } dist *= distFactor; }
		if (r_i == -1) r_i = ndist-1;
		
		JFR_ASSERT(theta_i >= 0 && theta_i < 2*nang, "theta_i out of bounds " << theta_i << " | " << 2*nang);
		JFR_ASSERT(phi_i >= 0 && phi_i < nang, "phi_i out of bounds " << phi_i << " | " << nang);
		JFR_ASSERT(r_i >= 0 && r_i < ndist, "r_i out of bounds " << r_i << " | " << ndist);
		
		/*
		 TODO to avoid repeated accesses to the map (log n), store the last used cell and coords,
		 check that the coords are still the same, and reuse directly if so
		 we could also if we are not close to the border of a cell infer quickly
		 that the cell hasn't changed, avoiding computations of sqrt and atan2 and / and...
		 eg by computing directly the cos or sin angle with dot or cross prod...
		 */
		if (create)
			return &(map[Index(theta_i, phi_i, r_i)]);
		else
		{
			std::map<Index,Cell>::iterator it = map.find(Index(theta_i, phi_i, r_i));
			return (it == map.end() ? NULL : &(it->second));
		}
	}
	
	
	void VisibilityMap::addObservation(const observation_ptr_t obsPtr)
	{
		if (obsPtr->events.measured)
		{
			Cell &cell = *(getCell(obsPtr, true));
			cell.lastTryFrame = obsPtr->sensorPtr()->rawCounter;
			cell.lastResult = obsPtr->events.updated;
			if (cell.lastResult) cell.nSuccess++; else cell.nFailure++;
			lastCell = &cell;
		}
	}
	
	
	void VisibilityMap::estimateVisibility(const observation_ptr_t obsPtr, double &visibility, double &certainty)
	{
		Cell *cell = getCell(obsPtr, false);
		if (!cell) cell = lastCell;
		if (!cell) { lastVis = visibility = 0.5; lastVisUncert = certainty = 0.;  return; }
		if (cell->lastResult) { lastVis = visibility = 1.; lastVisUncert = certainty = 1.; return; }
		int nTries = cell->nSuccess + cell->nFailure;
		double rate = cell->nSuccess / (double)nTries;
		
		lastVis = visibility = rate;
		lastVisUncert = certainty = (nTries >= nCertainty ? 1.0 : nTries/(double)nCertainty);
	}



}}

