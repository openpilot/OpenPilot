/* $Id$ */

#ifndef JMATH_DELAUNAY_HPP
#define JMATH_DELAUNAY_HPP

#include <list>

// #define DEBUG_TTL

#include <HeTriang.h>
#include <HeDart.h>
#include <HeTraits.h>

namespace jafar {
  namespace jmath {


    class DNode : public hed::Node {

    protected:

      unsigned int _id;

    public:

      DNode(double x_, double y_, double z_=0);
      DNode(unsigned int id_, double x_, double y_, double z_=0);

      unsigned int id() const {return _id;};

    };
    

    class Delaunay {

    protected:
      typedef std::list<DNode*>::iterator nodes_it;
      
      std::list<DNode*> nodes;

      hed::Triangulation triangulation;

    public:

      Delaunay(double xmin_, double ymin_, double xmax_, double ymax_, double zref_=0.0);
      ~Delaunay();

      void addNode(int id, double x_, double y_, double z_=0);

      int nbTriangles() const;
      std::string getTriangles();
      
    };

  }
}

#endif // JMATH_DELAUNAY_HPP
