#ifndef APPEARANCESEGMENT_HPP
#define APPEARANCESEGMENT_HPP

#include "rtslam/appearanceAbstract.hpp"
#include "dseg/SegmentsSet.hpp"

namespace jafar
{
   namespace rtslam
   {
      class AppearanceSegment;
      typedef boost::shared_ptr<AppearanceSegment> app_seg_ptr_t;

      /** Appearence for matching
       * rtslam.
       *
       * @ingroup rtslam
       */
      class AppearanceSegment: public AppearanceAbstract {
         private:
            dseg::SegmentsSet m_hypothesis;
         public:
            AppearanceSegment(dseg::SegmentHypothesis* _hypothesis = NULL){if(_hypothesis != NULL)m_hypothesis.addSegment(_hypothesis);}
            virtual ~AppearanceSegment(){}
            virtual AppearanceAbstract* clone(){return new AppearanceSegment(hypothesis());}

            dseg::SegmentHypothesis* hypothesis() {return m_hypothesis.segmentAt(0);}
            void setHypothesis(dseg::SegmentHypothesis* _hypothesis)
            {
               m_hypothesis = dseg::SegmentsSet();
               m_hypothesis.addSegment(_hypothesis);
            }
      };
   }
}

#endif // APPEARANCESEGMENT_HPP
