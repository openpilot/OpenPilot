#include "qdisplay/labeler.hpp"

// jafar debug include
#include "kernel/jafarDebug.hpp"
#include "kernel/jafarTestMacro.hpp"

#include <QApplication>

#include <iostream>
#include <fstream>

namespace jafar {
namespace qdisplay {


/******************************************************************************/

Labeler::Labeler(int nimages, char** images):
	qual(LabeledImage::qlGood), cursor(NULL), mark(NULL)
{
	this->images.resize(nimages);
	for(int i = 0; i < nimages; ++i)
		this->images[i].name = images[i];
	this->nimages = nimages;
	w = h = 23.; // FIXME
	pos = 0;
	lastpos = -1;
	viewer = new qdisplay::Viewer;
	view = new qdisplay::ImageView;
	display();
	viewer->setImageView(view, 0, 0);

	if (!connect(viewer, SIGNAL(onKeyPress(QKeyEvent*)), this, SLOT(onKeyPress(QKeyEvent*)), Qt::DirectConnection))
		std::cout << "connect onKeyPress failed" << std::endl;
	if (!connect(viewer, SIGNAL(onMouseClick(QGraphicsSceneMouseEvent*, bool)), this, SLOT(onMouseClick(QGraphicsSceneMouseEvent*, bool)), Qt::DirectConnection))
		std::cout << "connect onMouseClick failed" << std::endl;
	if (!connect(viewer, SIGNAL(onMouseMove(QGraphicsSceneMouseEvent*)), this, SLOT(onMouseMove(QGraphicsSceneMouseEvent*)), Qt::DirectConnection))
		std::cout << "connect onMouseMove failed" << std::endl;
}

Labeler::~Labeler()
{
	delete view; delete viewer;
}

void Labeler::display()
{
	img.load(images[pos].name);
	view->setImage(img);

	if (images[pos].status == LabeledImage::stManual || images[pos].status == LabeledImage::stInterpol)
		qual = images[pos].quality;

	displayRects();
}

void Labeler::displayRects()
{
	unsigned char lum = 255 - qual*255/3;

	// mark
	if (!mark)
	{
		mark = new qdisplay::Shape(qdisplay::Shape::ShapeRectangle, w/2, h/2, w, h);
		mark->setVisible(false);
		view->addShape(mark);
	}

	mark->hide();
	if (images[pos].status == LabeledImage::stManual || images[pos].status == LabeledImage::stInterpol)
	{
		switch (images[pos].status)
		{
			case LabeledImage::stManual: mark->setColor(0,lum,0); break;
			case LabeledImage::stInterpol: mark->setColor(lum,lum,0); break;
			default: break;
		}
		mark->setBoundingBox(images[pos].rect.x+images[pos].rect.width/2., images[pos].rect.y+images[pos].rect.height/2., images[pos].rect.width, images[pos].rect.height);
		mark->show();
	}

	// cursor
	if (!cursor)
	{
		cursor = new qdisplay::Shape(qdisplay::Shape::ShapeRectangle, w/2, h/2, w, h);
		cursor->setVisible(false);
		view->addShape(cursor);
	}

	cursor->hide();
	switch (images[pos].status)
	{
		case LabeledImage::stUninit:
		case LabeledImage::stManual:
		case LabeledImage::stInterpol: cursor->setColor(lum,0,0); break;
		case LabeledImage::stNothing: cursor->setColor(0,0,lum); break;
		default: break;
	}
	cursor->show();
}


void Labeler::interpol(int start, LabeledImage::Status statusFind1, LabeledImage::Status statusFill1,
																	LabeledImage::Status statusFind2, LabeledImage::Status statusFill2)
{
	for(int i = start-1; i >= 0; --i)
	{
		if (images[i].status == statusFind1) { interpol(i, start, statusFill1); break; } else
		if (images[i].status == statusFind2) { interpol(i, start, statusFill2); break; }
	}

	if (start > lastpos) { lastpos = start; return; }

	for(int i = start+1; i < nimages; ++i)
	{
		if (images[i].status == statusFind1) { interpol(start, i, statusFill1); break; } else
		if (images[i].status == statusFind2) { interpol(start, i, statusFill2); break; }
	}
}


void Labeler::interpol(int before, int after, LabeledImage::Status status)
{
	for(int i = before+1; i < after; ++i)
	{
		if (status == LabeledImage::stInterpol)
			images[i].rect = images[before].rect + ((images[after].rect-images[before].rect)*(double)(i-before))/(double)(after-before);
		images[i].status = status;
		images[i].quality = std::max(images[before].quality, images[after].quality);
	}
}

/******************************************************************************/

void Labeler::save()
{
	for(int i = 0; i < nimages; ++i)
	{
		if (images[i].status == LabeledImage::stUninit) continue;
		std::ofstream file;
		file.open((images[i].name+".txt").c_str());
		file << images[i].rect.x << "\t" << images[i].rect.y << "\t"
				 << images[i].rect.width << "\t" << images[i].rect.height
				 << std::endl;
		file << images[i].status << "\t" << images[i].quality << std::endl;
		file.close();
	}

	std::ofstream file;
	file.open("labels-readme.txt");
	file << "# format of label files" << std::endl;
	file << "# first line contains the object bounding box:" << std::endl;
	file << "<x> <y> <width> <height>" << std::endl;
	file << "# second line contains some flags:" << std::endl;
	file << "<status> <quality>" << std::endl;
	file << "# status:" << std::endl;
	file << "#    0 = uninitialized (same than label file does not exist)" << std::endl;
	file << "#    1 = manually set bounding box" << std::endl;
	file << "#    2 = interpolated bounding box" << std::endl;
	file << "#    3 = no object in the image" << std::endl;
	file << "# quality:" << std::endl;
	file << "#    0 = good (easy) : can be identified with good confidence" << std::endl;
	file << "#    1 = ok (hard) : main patterns may still be identified by a very good algorithm" << std::endl;
	file << "#    2 = bad (impossible) : not possible to identify it with enough confidence (fuzzy, partial, saturated, ...) but don't consider it as false positive" << std::endl;
	file.close();
}

void Labeler::load()
{
	for(int i = 0; i < nimages; ++i)
	{
		std::ifstream file;
		file.open((images[i].name+".txt").c_str());
		if (!file.is_open()) continue;
		file >> images[i].rect.x >> images[i].rect.y
				 >> images[i].rect.width >> images[i].rect.height
				 >> (int&)(images[i].status) >> (int&)(images[i].quality);
		file.close();
		lastpos = i;
	}
	display();
}


void Labeler::increaseSize(double inc)
{
	w += inc; h = w;
	cursor->hide();
	cursor->setBoundingBox(cursor->scenePos().x(), cursor->scenePos().y(), w,h);
	cursor->show();
}

void Labeler::decreaseSize(double inc)
{
	w -= inc; if (w < 1.0) w = 1.0; h = w;
	cursor->hide();
	cursor->setBoundingBox(cursor->scenePos().x(), cursor->scenePos().y(), w,h);
	cursor->show();
}

void Labeler::remove()
{
	images[pos].status = LabeledImage::stNothing;
	images[pos].quality = LabeledImage::qlGood;
	displayRects();
	interpol(pos, LabeledImage::stNothing, LabeledImage::stNothing, LabeledImage::stManual, LabeledImage::stUninit);
}

void Labeler::next()
{
	if (pos < nimages-1) { ++pos; display(); }
}

void Labeler::prev()
{
	if (pos > 0) { --pos; display(); }
}

void Labeler::find(LabeledImage::Status status)
{
	for(int i = pos+1; i < nimages; ++i)
		if (images[i].status == status)
			{ pos = i; display(); return; }
	for(int i = 0; i < pos; ++i)
		if (images[i].status == status)
			{ pos = i; display(); return; }
}

void Labeler::quality(LabeledImage::Quality quality)
{
	qual = quality;
	if (images[pos].status == LabeledImage::stManual || images[pos].status == LabeledImage::stInterpol)
		images[pos].quality = quality;
	displayRects();
}


/******************************************************************************/

void Labeler::onKeyPress(QKeyEvent *event)
{
	switch (event->key())
	{
		case Qt::Key_Space:
		case Qt::Key_Right:
		case Qt::Key_F: // next
			next();
			break;
		case Qt::Key_Left:
		case Qt::Key_S: // prev
			prev();
			break;
		case Qt::Key_Up: // increase size
		case Qt::Key_E:
			increaseSize(1.0);
			break;
		case Qt::Key_Down: // decrease size
		case Qt::Key_D:
			decreaseSize(1.0);
			break;
		case Qt::Key_R: // remove
			remove();
			next();
			break;
		case Qt::Key_U: // find next uninit
			find(LabeledImage::stUninit);
			break;
		case Qt::Key_O: // find next object
			find(LabeledImage::stManual);
			break;
		case Qt::Key_P: // print to files
			save();
			break;
		case Qt::Key_L: // load
			load();
			break;
		case Qt::Key_W: // qual good
			quality(LabeledImage::qlGood);
			break;
		case Qt::Key_X: // qual ok
			quality(LabeledImage::qlOk);
			break;
		case Qt::Key_C: // qual bad
			quality(LabeledImage::qlBad);
			break;
		case Qt::Key_Q: // quit
			QApplication::quit();
			break;
	}
}

void Labeler::onMouseClick(QGraphicsSceneMouseEvent *mouseEvent, bool isClick)
{
	if (!isClick) return;

	if (mouseEvent->button() == Qt::MiddleButton)
	{
		remove();
		next();
	} else
	if (mouseEvent->button() == Qt::LeftButton)
	{
		QPointF cpos = mouseEvent->buttonDownScenePos(mouseEvent->button());

		images[pos].rect = cv::Rect_<double>(cpos.x()-w/2., cpos.y()-h/2., w, h);
		images[pos].status = LabeledImage::stManual;
		images[pos].quality = qual;

		mark->setBoundingBox(cpos.x(), cpos.y(), w,h);
		displayRects();

		interpol(pos, LabeledImage::stManual, LabeledImage::stInterpol, LabeledImage::stNothing, LabeledImage::stUninit);
		next();
	}
}

void Labeler::onMouseMove(QGraphicsSceneMouseEvent *mouseEvent)
{
	QPointF pos = mouseEvent->scenePos();
	if (pos.x() < -w/2) pos.setX(-w/2);
	if (pos.y() < -h/2) pos.setY(-h/2);
	if (pos.x() > img.width()+w/2) pos.setX(img.width()+w/2);
	if (pos.y() > img.height()+h/2) pos.setY(img.height()+h/2);
	cursor->setPos(pos.x(), pos.y());
}

#include "labeler.moc"


}}

