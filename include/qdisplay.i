/* $Id$ */

/** swig interface file for module qdisplay.
 *
 * \file qdisplay.i
 * \ingroup qdisplay
 */      

%module(directors="1") qdisplay

%{
/* ruby defines ALLOC which conflicts with boost */ 
#undef ALLOC
  
/* 
   * headers necessary to compile the wrapper
   */

#include "jafarConfig.h"
#include "qdisplay/Viewer.hpp"
#include "qdisplay/ImageView.hpp"
#include "qdisplay/Shape.hpp"
#include "qdisplay/Line.hpp"
#include "qdisplay/PolyLine.hpp"
#include "qdisplay/AbstractEventHandler.hpp"
#include "qdisplay/ViewerManager.hpp"
// using namespace jafar::qdisplay;

%}

#if defined(SWIGRUBY)
    %trackobjects;
    %markfunc jafar::qdisplay::Viewer "mark_Viewer";
    %markfunc jafar::qdisplay::ViewerManager "mark_ViewerManager";
#endif

%include "jafar.i"
%include "qdisplayException.i"
%include "qdisplay/Shape.hpp"
%include "qdisplay/ViewerManager.hpp"

%feature("director") jafar::qdisplay::AbstractEventHandler;
%include "qdisplay/AbstractEventHandler.hpp"

%include "qdisplay/Line.hpp"
%include "qdisplay/PolyLine.hpp"

#if defined(SWIGRUBY)
    %apply SWIGTYPE *DISOWN { jafar::qdisplay::PolyLine* pl };
    %apply SWIGTYPE *DISOWN { jafar::qdisplay::Line* li };
    %apply SWIGTYPE *DISOWN { jafar::qdisplay::Shape* si };
#endif
%include "qdisplay/ImageView.hpp"

#if defined(SWIGRUBY)
    %apply SWIGTYPE *DISOWN { jafar::qdisplay::ImageView* ii };
#endif
%include "qdisplay/Viewer.hpp"




#if defined(SWIGRUBY)
%header %{

static void mark_ViewerManager(void* ptr) {
  jafar::qdisplay::ViewerManager* obj = (jafar::qdisplay::ViewerManager*) ptr;
  QList<jafar::qdisplay::Viewer*> list = obj->viewers();
  for(QList<jafar::qdisplay::Viewer*>::iterator it = list.begin(); it != list.end(); it++)
  {
    jafar::qdisplay::Viewer* v =*it;
    if(v->isVisible())
    {
      VALUE object = SWIG_RubyInstanceFor(v);
      if (object != Qnil) {
        rb_gc_mark(object);
      }
    }
  }
}

  
static void mark_QObject(void* ptr) {
  QObject* obj = (QObject*) ptr;

  /* Loop over each object and tell the garbage collector
    that we are holding a reference to them. */
  QList<QObject *> children = obj->findChildren<QObject *>();
  for(QList<QObject *>::iterator it = children.begin(); it != children.end(); ++it)
  {
    QObject* child = *it;
    VALUE object = SWIG_RubyInstanceFor(child);
    if (object != Qnil) {
      rb_gc_mark(object);
    }
  }
}

#include <QGraphicsScene>
#include <QGraphicsItem>

static void mark_Viewer(void* ptr) {
  jafar::qdisplay::Viewer* obj = (jafar::qdisplay::Viewer*) ptr;
/*  if(obj->isVisible())
  {
    VALUE robj = SWIG_RubyInstanceFor(ptr);
    rb_gc_mark(robj);
  }*/

  mark_QObject(ptr);

/* Loop over each object and tell the garbage collector
  that we are holding a reference to them. */
  QList<QGraphicsItem *> children = obj->scene()->items();
  for(QList<QGraphicsItem *>::iterator it = children.begin(); it != children.end(); ++it)
  {
    QGraphicsItem* child = *it;
    VALUE object = SWIG_RubyInstanceFor(child);
    if (object != Qnil) {
      rb_gc_mark(object);
    }
  }
  for(int i = 0; i < obj->rows(); i++)
  {
    for(int j = 0; j < obj->cols(); j++)
    {
      VALUE object = SWIG_RubyInstanceFor( obj->imageItem(i,j) );
      if (object != Qnil) {
        rb_gc_mark(object);
      }
    }
  }
}

%}

#endif

/*
 * headers to be wrapped goes here
 */

%include "qdisplayTools.i"
// instantiate some print functions
// replace "Type" with appropriate class name
// %template(print) jafar::qdisplay::print<jafar::qdisplay::Type>;

