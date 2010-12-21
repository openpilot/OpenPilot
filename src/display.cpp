/**
 * \file display.hpp
 * \author croussil@laas.fr
 * \date 25/03/2010
 * File defining a generic display architecture
 * \ingroup rtslam
 */

#include "rtslam/display.hpp"

namespace jafar {
namespace rtslam {
namespace display {

		colorRGB getColorRGB(color colorOrigin)  {
			colorRGB result ;
			result.set(0,0,0);
			switch (colorOrigin) {
				case color_blue:
					result.set(0,0,224);
					break;
				case color_cyan:
					result.set(0,255,255);
					break;
				case color_magenta:
					result.set(255,0,192);
					break;
				case color_darkred:
					result.set(144,0,0);
					break;
				case color_red:
					result.set(255,0,0);
					break;
				case color_transparency:
					result.set(255,255,255);
					break;
				case color_yellow:
					result.set(255,255,0);
					break;
				case color_pink:
						result.set(253,103,223);
						break;
				case color_orange:
						result.set(255,128,0);
						break;
				default:
					//std::cout << __FILE__ << ":" << __LINE__ << " Unknown lmk lmk_events " << lmk_events << std::endl;
					break;
			}
			return result ;
		}

}}}


