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
            int bestId = 0;
            double bestSqrLength = 0;
            for(int i=0 ; i<set.count() ; i++)
            {
               const dseg::SegmentHypothesis* seg = set.segmentAt(i);

               double dx = seg->x1() - seg->x2();
               double dy = seg->y1() - seg->y2();
               double sqrLength = dx*dx + dy*dy;
               if(sqrLength > bestSqrLength)
               {
                  bestId = i;
                  bestSqrLength = sqrLength;
               }
            }

            featPtr->measurement.x(0) = set.segmentAt(bestId)->x1();
            featPtr->measurement.x(1) = set.segmentAt(bestId)->y1();
            featPtr->measurement.x(2) = set.segmentAt(bestId)->x2();
            featPtr->measurement.x(3) = set.segmentAt(bestId)->y2();
            featPtr->measurement.matchScore = 1; // No idea of what this thingy does, 1 sounds fine

            featPtr->appearancePtr.reset(new AppearanceSegment(set.segmentAt(bestId)));

            ret = true;
         }

         return ret;
      }

   };
};
