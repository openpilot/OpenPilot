/* $Id$ */

#ifdef HAVE_TTL

#include "kernel/jafarException.hpp"

#include "jmath/delaunay.hpp"

using namespace jafar::jmath;
using namespace hed;

DNode::DNode(double x_, double y_, double z_) :
  Node(x_,y_,z_),
  _id()
{}


DNode::DNode(unsigned int id_, double x_, double y_, double z_) :
  Node(x_,y_,z_),
  _id(id_)
{}

Delaunay::Delaunay(double xmin_, double ymin_, double xmax_, double ymax_, double zref_) :
  nodes(),
  triangulation()
{
  nodes.push_back(new DNode(xmin_,ymin_,zref_));
  nodes.push_back(new DNode(xmin_,ymax_,zref_));
  nodes.push_back(new DNode(xmax_,ymin_,zref_));
  nodes.push_back(new DNode(xmax_,ymax_,zref_));
  triangulation.createDelaunay(nodes.begin(), nodes.end());
}

Delaunay::~Delaunay() {
  // class DNode derives Node wich has a built-in ref counter 
//   for (nodes_it it = nodes.begin() ; it != nodes.end() ; ++it) {
//     delete *it;
//   }
}

void Delaunay::addNode(int id, double x, double y, double z)
{
  Dart dart = triangulation.createDart();
  nodes.push_back(new DNode(id,x,y,z));
  bool status = ttl::insertNode<hed::TTLtraits>(dart, *(nodes.back()));
  JFR_POSTCOND(status, "Delaunay::addNode: delaunay triangulation failed");
}

int Delaunay::nbTriangles () const {
  return triangulation.noTriangles();
}

std::string Delaunay::getTriangles() {
  std::stringstream os;

  Dart dart(triangulation.getBoundaryEdge());
  ttl::removeRectangularBoundary<hed::TTLtraits>(dart);

  list<Edge*> const& leadingEdges = triangulation.getLeadingEdges();
  list<Edge*>::const_iterator it;
  for (it = leadingEdges.begin(); it != leadingEdges.end(); ++it) {
    Edge* edge = *it;
    os << "{ ";
    for (int i = 0; i < 3; ++i) {
      DNode& node = *(static_cast<DNode*>(edge->getSourceNode()));
      os << "{ " << node.id() << " " << node.x() << " " << node.y() << " " << node.z() << " } ";
      edge = edge->getNextEdgeInFace();
    }
    os << "} ";
  }
  return os.str(); 
}

#endif // HAVE_TTL
