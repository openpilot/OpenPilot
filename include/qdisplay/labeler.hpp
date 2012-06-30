#ifndef _QDISPLAY_LABELER_H_
#define _QDISPLAY_LABELER_H_

#include <QObject>
#include <QApplication>


#include "qdisplay/Viewer.hpp"
#include "qdisplay/ImageView.hpp"
#include "qdisplay/Shape.hpp"
#include "kernel/threads.hpp"


namespace jafar {
namespace qdisplay {

class LabeledImage;

/******************************************************************************/

class LabeledImage
{
public:
	enum Status { stUninit=0, stManual, stInterpol, stNothing };
	enum Quality { qlGood=0, qlOk, qlBad };
public:
	LabeledImage(): rect(), status(stUninit), quality(qlGood) {}
	std::string name;
	cv::Rect_<double> rect;
	Status status;
	Quality quality;
};


/**
	Keyboard:
	- Left/right | S/F: prev/next image
	- Up/Down | E/D: increase/decrease rect size
	- R: remove label
	- U: go to next uninitialized frame
	- O: go to next frame with labeled object
	- P: print to files
	- L: load from files
	- W: quality good (must be detected by the algorithm)
	- X: quality ok (should be detected if the algorithm is very good)
	- C: quality bad (cannot ask to an algorithm to detect it, for instance if it is partial, but don't consider it as false positive either!)
	- Q: quit
	
	Mouse:
	- Left click: record the position of the object
	- Middle click: record that the object is absent from the frame
	- Wheel: zoom
	
	Cursor color:
	- blue: labeled as nothing (bright/dark/very dark: quality good/ok/bad)
	- red: not labeled or labeled as object (bright/dark/very dark: quality good/ok/bad)

	Object color:
	- green: manually labeled
	- yellow: interpolated
	*/
class Labeler: public QObject
{
	Q_OBJECT
private:
	qdisplay::Viewer *viewer;
	qdisplay::ImageView *view;
	image::Image img;
	std::vector<LabeledImage> images;
	int nimages;
	int pos, lastpos;
	double w,h;
	LabeledImage::Quality qual;
	qdisplay::Shape *cursor;
	qdisplay::Shape *mark;
private:
	void display();
	void displayRects();
	void interpol(int start, LabeledImage::Status statusFind1, LabeledImage::Status statusFill1,
								LabeledImage::Status statusFind2, LabeledImage::Status statusFill2);
	void interpol(int before, int after, LabeledImage::Status status);
	void save();
	void load();
	void increaseSize(double inc);
	void decreaseSize(double inc);
	void remove();
	void next();
	void prev();
	void find(LabeledImage::Status status);
	void quality(LabeledImage::Quality quality);
public:
	Labeler(int nimages, char** images);
	~Labeler();
public slots:
	void onKeyPress(QKeyEvent *event);
	void onMouseClick(QGraphicsSceneMouseEvent *mouseEvent, bool isClick);
	void onMouseMove(QGraphicsSceneMouseEvent *mouseEvent);
};



}}

namespace cv {
template<typename T> Rect_<T> operator+(Rect_<T> const& A, Rect_<T> const& B)
	{ return Rect(A.x+B.x, A.y+B.y, A.width+B.width, A.height+B.height); }
template<typename T> Rect_<T> operator-(Rect_<T> const& A, Rect_<T> const& B)
	{ return Rect(A.x-B.x, A.y-B.y, A.width-B.width, A.height-B.height); }
template<typename T> Rect_<T> operator*(Rect_<T> const& A, T b)
	{ return Rect(A.x*b, A.y*b, A.width*b, A.height*b); }
template<typename T> Rect_<T> operator/(Rect_<T> const& A, T b)
	{ return Rect(A.x/b, A.y/b, A.width/b, A.height/b); }
}

#endif

