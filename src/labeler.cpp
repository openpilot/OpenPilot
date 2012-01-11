#include "qdisplay/labeler.hpp"

// jafar debug include
#include "kernel/jafarDebug.hpp"
#include "kernel/jafarTestMacro.hpp"

#include <QApplication>

#include <iostream>

namespace jafar {
namespace qdisplay {


/******************************************************************************/

Labeler::Labeler(int nimages, char** images)
{
	this->images = images;
	this->nimages = nimages;
	pos = 0;
	viewer = new qdisplay::Viewer;
	view = new qdisplay::ImageView;
	load();
	viewer->setImageView(view, 0, 0);

	if (!connect(viewer, SIGNAL(onKeyPress(QKeyEvent*)), this, SLOT(onKeyPress(QKeyEvent*)), Qt::DirectConnection))
		std::cout << "connect onKeyPress failed" << std::endl;
	if (!connect(viewer, SIGNAL(onMouseClick(QGraphicsSceneMouseEvent*, bool)), this, SLOT(onMouseClick(QGraphicsSceneMouseEvent*, bool)), Qt::DirectConnection))
		std::cout << "connect onMouseClick failed" << std::endl;
}

Labeler::~Labeler()
{
	delete view; delete viewer;
}

void Labeler::load()
{
	img.load(images[pos]);
	view->setImage(img);
}


/******************************************************************************/

void Labeler::onKeyPress(QKeyEvent *event)
{
	switch (event->key())
	{
		case Qt::Key_Space:
		case Qt::Key_Right:
		case Qt::Key_N: // next
			if (pos < nimages-1) ++pos;
			load();
			break;
		case Qt::Key_Left:
		case Qt::Key_P: // prev
			if (pos > 0) --pos;
			load();
			break;
		case Qt::Key_Q: // quit
			QApplication::quit();
			break;
	}
}

void Labeler::onMouseClick(QGraphicsSceneMouseEvent *mouseEvent, bool isClick)
{
	if (!isClick) return;

	mouseEvent->buttonDownScenePos(mouseEvent->button());

	std::cout << "click" << std::endl;
}


#include "labeler.moc"


}}

