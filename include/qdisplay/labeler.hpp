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


/******************************************************************************/

class Labeler: public QObject
{
	Q_OBJECT
private:
	qdisplay::Viewer *viewer;
	qdisplay::ImageView *view;
	image::Image img;
	char** images;
	int nimages;
	int pos;
	int w,h;
private:
	void load();
public:
	Labeler(int nimages, char** images);
	~Labeler();
public slots:
	void onKeyPress(QKeyEvent *event);
	void onMouseClick(QGraphicsSceneMouseEvent *mouseEvent, bool isClick);
};



}}

#endif

