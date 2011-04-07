#ifndef APPEARANCESEGMENT_HPP
#define APPEARANCESEGMENT_HPP

#ifdef HAVE_MODULE_DSEG

#include "rtslam/appearanceAbstract.hpp"
#include "dseg/SegmentsSet.hpp"
#include "dseg/SegmentHypothesis.hpp"
#include "jmath/misc.hpp"

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

				jblas::vec4 realObs()
				{
					jblas::vec4 ret;
					ret.clear();
					if(m_hypothesis.count() > 0)
					{
						ret[0] = m_hypothesis.segmentAt(0)->x1();
						ret[1] = m_hypothesis.segmentAt(0)->y1();
						ret[2] = m_hypothesis.segmentAt(0)->x2();
						ret[3] = m_hypothesis.segmentAt(0)->y2();
					}
					return ret;
				}
      };
   }
}

#endif //HAVE_MODULE_DSEG

#endif // APPEARANCESEGMENT_HPP
