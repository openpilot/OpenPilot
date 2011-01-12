/**
 * \file worldAbstract.cpp
 * \date 08/06/2010
 * \author croussil
 * \ingroup rtslam
 */

#include "rtslam/worldAbstract.hpp"
#include "rtslam/display.hpp"

namespace jafar {
	namespace rtslam {

		void WorldAbstract::addDisplayViewer(display::ViewerAbstract *viewer, unsigned id)
		{
			if (display_viewers.size() <= id)
			{
				int oldsize = display_viewers.size();
				display_viewers.resize(id+1);
				for(unsigned int i = oldsize; i < id; ++i)
					display_viewers[i] = NULL;
			}
			display_viewers[id] = viewer;
		}
		
		display::ViewerAbstract* WorldAbstract::getDisplayViewer(unsigned id)
		{
			if (display_viewers.size() <= id) return NULL;
			return display_viewers[id];
		}

	}
}
