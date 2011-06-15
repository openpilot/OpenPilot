/**
 * \file appearanceImage.hpp
 * \author jmcodol
 * \ingroup rtslam
 */

#ifndef __AppearenceImageSimu_H__
#define __AppearenceImageSimu_H__

/* --------------------------------------------------------------------- */
/* --- INCLUDE --------------------------------------------------------- */
/* --------------------------------------------------------------------- */

#include "jmath/jblas.hpp"
#include "image/Image.hpp"
#include "rtslam/appearanceAbstract.hpp"
#include "rtslam/gaussian.hpp"
#include "boost/shared_ptr.hpp"


#ifdef HAVE_MODULE_DSEG
	#include "dseg/SegmentsSet.hpp"
	#include "dseg/SegmentHypothesis.hpp"
#endif

namespace jafar {
	namespace rtslam {

		class AppearanceImagePoint;
      typedef boost::shared_ptr<AppearanceImagePoint> app_img_pnt_ptr_t;

		/** Appearence for matching
		 * rtslam.
		 *
		 * @ingroup rtslam
		 */
		class AppearanceImagePoint: public AppearanceAbstract {
			public:
				image::Image patch;
				unsigned int patchSum;
				unsigned int patchSquareSum;
				Gaussian offset; ///< offset between the center of the patch and the real position of the feature
			public:
				AppearanceImagePoint(const image::Image& patch, Gaussian const &offset);
				AppearanceImagePoint(int width, int height, int depth):
					patch(width, height, depth, JfrImage_CS_GRAY), offset(2) {
//					cout << "Created patch with " << width << "x" << height << " pixels; depth: " << depth << "; color space: " << JfrImage_CS_GRAY << endl;
				}
				virtual ~AppearanceImagePoint();
				virtual AppearanceAbstract* clone();

			private:
				void computePatchIntegrals();
		};

#ifdef HAVE_MODULE_DSEG

		class AppearanceImageSegment;
      typedef boost::shared_ptr<AppearanceImageSegment> app_img_seg_ptr_t;

      /** Appearence for matching
       * rtslam.
       *
       * @ingroup rtslam
       */
      class AppearanceImageSegment: public AppearanceAbstract {
         public:
            image::Image patch;
				unsigned int patchMeanLeft;
				unsigned int patchMeanRight;
				Gaussian offsetTop;
				Gaussian offsetBottom;
				dseg::SegmentsSet m_hypothesis;
			public:
				AppearanceImageSegment(const image::Image& patch, Gaussian const &offsetTop, Gaussian const &offsetBottom, dseg::SegmentHypothesis* _hypothesis = NULL);
				AppearanceImageSegment(int width, int height, int depth, dseg::SegmentHypothesis* _hypothesis = NULL):
					patch(width, height, depth, JfrImage_CS_GRAY),
					patchMeanLeft(0),patchMeanRight(0),
					offsetTop(2), offsetBottom(2) {
					if(_hypothesis != NULL)m_hypothesis.addSegment(_hypothesis);
//					cout << "Created patch with " << width << "x" << height << " pixels; depth: " << depth << "; color space: " << JfrImage_CS_GRAY << endl;
            }
            virtual ~AppearanceImageSegment();
            virtual AppearanceAbstract* clone();

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
			
#endif //HAVE_MODULE_DSEG

	}

}


#endif // #ifndef __AppearenceImageSimu_H__
/*
 * Local variables:
 * mode: c++
 * indent-tabs-mode: t
 * tab-width: 2
 * c-basic-offset: 2
 * End:
 */
