#ifndef APPEARANCESEGMENT_HPP
#define APPEARANCESEGMENT_HPP

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
         public:
            AppearanceSegment(){}
            virtual ~AppearanceSegment(){}
            virtual AppearanceAbstract* clone(){return new AppearanceSegment();}
      };
   }
}

#endif // APPEARANCESEGMENT_HPP
