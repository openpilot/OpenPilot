#ifndef SIZELATLNG_H
#define SIZELATLNG_H


#include <QString>

struct PointLatLng;
struct SizeLatLng
{
public:
    SizeLatLng();
    static  SizeLatLng Empty;

    SizeLatLng(SizeLatLng const&  size)
    {
       this->widthLng = size.widthLng;
       this->heightLat = size.heightLat;
    }

     SizeLatLng(PointLatLng const&  pt);


     SizeLatLng(double const& heightLat, double const&  widthLng)
    {
       this->heightLat = heightLat;
       this->widthLng = widthLng;
    }

     friend SizeLatLng operator+(SizeLatLng const&  sz1, SizeLatLng const&  sz2);
     friend SizeLatLng operator-(SizeLatLng const&  sz1, SizeLatLng const&  sz2);
     friend bool operator==(SizeLatLng const&  sz1, SizeLatLng const&  sz2);
     friend bool operator!=(SizeLatLng const&  sz1, SizeLatLng const&  sz2);


//     static explicit operator PointLatLng(SizeLatLng size)
//    {
//       return new PointLatLng(size.HeightLat(), size.WidthLng());
//    }


     bool IsEmpty()const
     {
         return ((this->widthLng == 0) && (this->heightLat == 0));
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

     static SizeLatLng Add(SizeLatLng const& sz1, SizeLatLng const& sz2)
    {
       return SizeLatLng(sz1.HeightLat() + sz2.HeightLat(), sz1.WidthLng() + sz2.WidthLng());
    }

     static SizeLatLng Subtract(SizeLatLng const& sz1, SizeLatLng const& sz2)
    {
       return SizeLatLng(sz1.HeightLat() - sz2.HeightLat(), sz1.WidthLng() - sz2.WidthLng());
    }

//     override bool Equals(object obj)
//    {
//       if(!(obj is SizeLatLng))
//       {
//          return false;
//       }
//       SizeLatLng ef = (SizeLatLng) obj;
//       return (((ef.WidthLng == this->WidthLng) && (ef.HeightLat == this->HeightLat)) && ef.GetType().Equals(base.GetType()));
//    }

//     override int GetHashCode()
//    {
//       return base.GetHashCode();
//    }

//     PointLatLng ToPointLatLng()
//    {
//       return (PointLatLng) this;
//    }

     QString ToString()
    {
         return ("{WidthLng=" + QString::number(this->widthLng) + ", HeightLng=" + QString::number(this->heightLat) + "}");
    }


private:
    double heightLat;
    double widthLng;
};

#endif // SIZELATLNG_H

