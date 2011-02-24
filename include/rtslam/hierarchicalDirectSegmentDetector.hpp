/*
 * \file hierarchicalDirectSegmentDetector.hpp
 *
 *     Project: jafar
 *  Created on: Feb 14, 2011
 *      \Author: bhautboi
 *
 *  \ingroup rtslam
 */

#ifndef HIERARCHICALDIRECTSEGMENTDETECTOR_HPP_
#define HIERARCHICALDIRECTSEGMENTDETECTOR_HPP_

#include "image/Image.hpp"
#include "image/roi.hpp"

#include "rtslam/featurePoint.hpp"

namespace jafar{
	namespace rtslam{

		/**
		 * Hierarchical direct segment detector class.
		 * \ingroup rtslam
		 * \author Benjamin Hautbois bhautboi@laas.fr
		 *
		 * This class wraps around the dseg module, it detects segments inside a given region of interest.
		 */
		class HierarchicalDirectSegmentDetector {
      public:
        HierarchicalDirectSegmentDetector();
        ~HierarchicalDirectSegmentDetector();
        virtual bool detectIn(image::Image const& image, feat_img_pnt_ptr_t featPtr, const image::ConvexRoi * roiPtr = 0 );
		};
	}
}




#endif /* HIERARCHICALDIRECTSEGMENTDETECTOR_HPP_ */
