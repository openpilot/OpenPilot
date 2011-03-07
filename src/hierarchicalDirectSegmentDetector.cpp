#include "rtslam/hierarchicalDirectSegmentDetector.hpp"
#include "dseg/SegmentsSet.hpp"
#include "dseg/SegmentHypothesis.hpp"

namespace jafar{
   namespace rtslam {

      HierarchicalDirectSegmentDetector::HierarchicalDirectSegmentDetector()
      {
      }

      HierarchicalDirectSegmentDetector::~HierarchicalDirectSegmentDetector()
      {
      }

      bool HierarchicalDirectSegmentDetector::detectIn(image::Image const& image, feat_seg_ptr_t featPtr, const image::ConvexRoi * roiPtr)
      {
         bool ret = false;
         dseg::SegmentsSet set;
         detector.detectSegment(image, roiPtr, set);

         if(set.count() > 0)
         {
/*				Take the best (someday)
            for(int i=0 ; i<set.count() ; i++)
            {
               if(set.segmentAt(i)->l)
            }
*/
            featPtr.reset(new FeatureSegment());
            featPtr->measurement.x(0) = set.segmentAt(0)->x1();
            featPtr->measurement.x(1) = set.segmentAt(0)->y1();
            featPtr->measurement.x(2) = set.segmentAt(0)->x2();
            featPtr->measurement.x(3) = set.segmentAt(0)->y2();

            featPtr->appearancePtr.reset(new AppearanceSegment());

            ret = true;
         }

         return ret;
      }

   };
};
