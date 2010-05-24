#ifndef RECTLATLNG_H
#define RECTLATLNG_H

//#include "pointlatlng.h"
#include "../internals/pointlatlng.h"
#include "math.h"
#include <QString>
#include "sizelatlng.h"
struct RectLatLng
{
public:
    RectLatLng();
    static RectLatLng Empty;
    friend uint qHash(RectLatLng const& rect);
    friend bool operator==(RectLatLng const& left,RectLatLng const& right);
    friend bool operator!=(RectLatLng const& left,RectLatLng const& right);
    RectLatLng(double const& lat, double const& lng, double const& widthLng, double const& heightLat)
    {
        this->lng = lng;
        this->lat = lat;
        this->widthLng = widthLng;
        this->heightLat = heightLat;
    }
    RectLatLng(PointLatLng const& location, SizeLatLng const& size)
    {
        this->lng = location.Lng();
        this->lat = location.Lat();
        this->widthLng = size.WidthLng();
        this->heightLat = size.HeightLat();
    }
    static RectLatLng FromLTRB(double const& lng, double const& lat, double const& rightLng, double const& bottomLat)
    {
        return RectLatLng(lat, lng, rightLng - lng, lat - bottomLat);
    }
    PointLatLng LocationTopLeft()const
    {
        return  PointLatLng(this->lat, this->lng);
    }
    void SetLocationTopLeft(PointLatLng const& value)
    {
        this->lng = value.Lng();
        this->lat = value.Lat();
    }
    PointLatLng LocationRightBottom()
    {

        PointLatLng ret =  PointLatLng(this->lat, this->lng);
        ret.Offset(HeightLat(), WidthLng());
        return ret;
    }
    SizeLatLng Size()
    {
        return SizeLatLng(this->HeightLat(), this->WidthLng());
    }
    void SetSize(SizeLatLng const& value)
    {
        this->widthLng = value.WidthLng();
        this->heightLat = value.HeightLat();
    }
    double Lng()const
    {
        return this->lng;
    }
    void SetLng(double const& value)
    {
        this->lng = value;
    }


    double Lat()const
    {
        return this->lat;
    }
    void SetLat(double const& value)
    {
        this->lat = value;
    }

    double WidthLng()const
    {
        return this->widthLng;
    }
    void SetWidthLng(double const& value)
    {
        this->widthLng = value;
    }
    double HeightLat()const
    {
        return this->heightLat;
    }
    void SetHeightLat(double const& value)
    {
        this->heightLat = value;
    }
    double Left()const
    {
        return this->Lng();
    }

    double Top()const
    {
        return this->Lat();
    }

    double Right()const
    {
        return (this->Lng() + this->WidthLng());
    }

    double Bottom()const
    {
        return (this->Lat() - this->HeightLat());
    }
    bool IsEmpty()const
    {      
        if(this->WidthLng() > 0)
        {
            return (this->HeightLat() <= 0);
        }
        return true;
    }
    bool Contains(double const& lat, double const& lng)
    {
        return ((((this->Lng() <= lng) && (lng < (this->Lng() + this->WidthLng()))) && (this->Lat() >= lat)) && (lat > (this->Lat() - this->HeightLat())));
    }

    bool Contains(PointLatLng const& pt)
    {
        return this->Contains(pt.Lat(), pt.Lng());
    }

    bool Contains(RectLatLng const& rect)
    {
        return ((((this->Lng() <= rect.Lng()) && ((rect.Lng() + rect.WidthLng()) <= (this->Lng() + this->WidthLng()))) && (this->Lat() >= rect.Lat())) && ((rect.Lat() - rect.HeightLat()) >= (this->Lat() - this->HeightLat())));
    }
    void Inflate(double const& lat, double const& lng)
    {
        this->lng -= lng;
        this->lat += lat;
        this->widthLng += (double)2 * lng;
        this->heightLat +=(double)2 * lat;
    }

    void Inflate(SizeLatLng const& size)
    {
        this->Inflate(size.HeightLat(), size.WidthLng());
    }

    static RectLatLng Inflate(RectLatLng const& rect, double const& lat, double const& lng)
    {
        RectLatLng ef = rect;
        ef.Inflate(lat, lng);
        return ef;
    }

    void Intersect(RectLatLng const& rect)
    {
        RectLatLng ef = Intersect(rect, *this);
        this->lng = ef.Lng();
        this->lat = ef.Lat();
        this->widthLng = ef.WidthLng();
        this->heightLat = ef.HeightLat();
    }
    static RectLatLng Intersect(RectLatLng const& a, RectLatLng const& b)
    {
        double lng = std::max(a.Lng(), b.Lng());
        double num2 = std::min((double) (a.Lng() + a.WidthLng()), (double) (b.Lng() + b.WidthLng()));

        double lat = std::max(a.Lat(), b.Lat());
        double num4 = std::min((double) (a.Lat() + a.HeightLat()), (double) (b.Lat() + b.HeightLat()));

        if((num2 >= lng) && (num4 >= lat))
        {
            return RectLatLng(lng, lat, num2 - lng, num4 - lat);
        }
        return Empty;
    }
   bool IntersectsWith(RectLatLng const& rect)
   {
       return ((((rect.Lng() < (this->Lng() + this->WidthLng())) && (this->Lng() < (rect.Lng() + rect.WidthLng()))) && (rect.Lat() < (this->Lat() + this->HeightLat()))) && (this->Lat() < (rect.Lat() + rect.HeightLat())));
   }

   static RectLatLng Union(RectLatLng const& a, RectLatLng const& b)
   {
       double lng = std::min(a.Lng(), b.Lng());
       double num2 = std::max((double) (a.Lng() + a.WidthLng()), (double) (b.Lng() + b.WidthLng()));
       double lat = std::min(a.Lat(), b.Lat());
       double num4 = std::max((double) (a.Lat() + a.HeightLat()), (double) (b.Lat() + b.HeightLat()));
       return RectLatLng(lng, lat, num2 - lng, num4 - lat);
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

   QString ToString() const
   {
       return ("{Lat=" + QString::number(this->Lat()) + ",Lng=" + QString::number(this->Lng()) + ",WidthLng=" + QString::number(this->WidthLng()) + ",HeightLat=" + QString::number(this->HeightLat()) + "}");
   }

private:
    double lng;
    double lat;
    double widthLng;
    double heightLat;
};

#endif // RECTLATLNG_H



//      static RectLatLng()
//      {
//         Empty = new RectLatLng();
//      }
//   }
