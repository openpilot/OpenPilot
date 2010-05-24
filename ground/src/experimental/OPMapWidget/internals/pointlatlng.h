#ifndef POINTLATLNG_H
#define POINTLATLNG_H

#include <QHash>
#include <QString>
#include "sizelatlng.h"
struct PointLatLng
{
    //friend uint qHash(PointLatLng const& point);
    friend bool operator==(PointLatLng const& lhs,PointLatLng const& rhs);
    friend bool operator!=(PointLatLng const& left, PointLatLng const& right);
    friend PointLatLng operator+(PointLatLng pt, SizeLatLng sz);
    friend PointLatLng operator-(PointLatLng pt, SizeLatLng sz);

   //TODO Sizelatlng friend PointLatLng operator+(PointLatLng pt, SizeLatLng sz);

   private:
    double lat;
    double lng;

   public:
    PointLatLng();


    static PointLatLng Empty;

      PointLatLng(const double &lat,const double &lng)
      {
         this->lat = lat;
         this->lng = lng;
      }

      bool IsEmpty()
      {
            return ((this->lng == 0) && (this->lat == 0));
      }

      double Lat()const
      {
          return this->lat;
      }

      void SetLat(const double &value)
      {
          this->lat = value;
      }


      double Lng()const
      {
          return this->lng;
      }
      void SetLng(const double &value)
      {
          this->lng = value;
      }





      static PointLatLng Add(PointLatLng const& pt, SizeLatLng const& sz)
      {
         return PointLatLng(pt.Lat() - sz.HeightLat(), pt.Lng() + sz.WidthLng());
      }

      static PointLatLng Subtract(PointLatLng const& pt, SizeLatLng const& sz)
      {
         return PointLatLng(pt.Lat() + sz.HeightLat(), pt.Lng() - sz.WidthLng());
      }


      void Offset(PointLatLng const& pos)
      {
         this->Offset(pos.Lat(), pos.Lng());
      }

      void Offset(double const& lat, double const& lng)
      {
         this->lng += lng;
         this->lat -= lat;
      }


      QString ToString()const
      {
         return QString("{Lat=%1, Lng=%2}").arg(this->lat).arg(this->lng);
      }

////      static PointLatLng()
////      {
////         Empty = new PointLatLng();
////      }
   };


//
#endif // POINTLATLNG_H
