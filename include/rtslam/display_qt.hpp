/**
 * \file display_qt.hpp
 * \date 25/03/2010
 * \author croussil
 * File defining a display architecture for qt with qdisplay module
 * \ingroup rtslam
 */

#ifndef DISPLAY_QT__HPP_
#define DISPLAY_QT__HPP_

#include "jafarConfig.h"

#ifdef HAVE_MODULE_QDISPLAY

#include "qdisplay/ImageView.hpp"
#include "qdisplay/Viewer.hpp"
#include "qdisplay/Shape.hpp"
#include "qdisplay/Ellipsoid.hpp"
#include "qdisplay/Line.hpp"
#include "qdisplay/init.hpp"

#include "rtslam/display.hpp"
#include "rtslam/rawImage.hpp"

#include "jmath/misc.hpp"

#define DEFINE_USELESS_OBJECTS 1

// FIXME for EMBED_PREDICTED_APP: the image is modified (in obs) after it is displayed (in sensor)...
#define EMBED_PREDICTED_APP 0

namespace jafar {
namespace rtslam {
namespace display {

//******************************************************************************
// Viewer

class WorldQt;
class MapQt;
class RobotQt;
class SensorQt;
class LandmarkQt;
class ObservationQt;

struct RunStatus
{
	bool pause;
	bool next;
	bool render_all;
	boost::condition_variable condition;
	boost::mutex mutex;
	RunStatus(): next(false) {}
};


#if DEFINE_USELESS_OBJECTS
class ViewerQt: public Viewer<WorldQt,MapQt,RobotQt,SensorQt,LandmarkQt,ObservationQt,
                              boost::variant<QGraphicsItem*, qdisplay::Viewer*, qdisplay::ImageView*> >
{
	public:
		int fontSize;
		double ellipsesScale;
		bool doDump;
		std::string dump_pattern; // pattern with %d for sensor id and frame id
		std::map<int,SensorQt*> sensorsList;
	public:
		ViewerQt(int _fontSize = 8, double _ellipsesScale = 3.0, bool _dump = false, std::string _dump_pattern = "data/rendered2D_%02d-%06d.png"): 
			fontSize(_fontSize), ellipsesScale(_ellipsesScale), doDump(_dump), dump_pattern(_dump_pattern) {}
		void dump(std::string filepattern); // pattern with %d for sensor id
		static RunStatus runStatus;
};
#else
#error "does not work"
//typedef Viewer<WorldDisplay,MapDisplay,RobotDisplay,SensorQt,LandmarkDisplay,ObservationQt> ViewerQt;
#endif


//******************************************************************************
// Objects

#if DEFINE_USELESS_OBJECTS

/** **************************************************************************

*/
class WorldQt : public WorldDisplay
{
		ViewerQt *viewerQt;
	public:
		WorldQt(ViewerAbstract *_viewer, rtslam::WorldAbstract *_slamWor, WorldDisplay *garbage);
		void bufferize() {}
		void render() {}
};

/** **************************************************************************

*/
class MapQt : public MapDisplay
{
		ViewerQt *viewerQt;
	public:
		MapQt(ViewerAbstract *_viewer, rtslam::MapAbstract *_slamMap, WorldQt *_dispWorld);
		void bufferize() {}
		void render() {}
};

/** **************************************************************************

*/
class RobotQt : public RobotDisplay
{
		ViewerQt *viewerQt;
	public:
		// buffered data
		//Gaussian pose_;
		//std::string model3d_;
		// graphical objects
	public:
		RobotQt(ViewerAbstract *_viewer, rtslam::RobotAbstract *_slamRob, MapQt *_dispMap);
		void bufferize() {}
		void render() {}
};
#endif

/** **************************************************************************

*/
class SensorQt : public QObject, public SensorDisplay
{
	Q_OBJECT
	private:
		ViewerQt *viewerQt;
	public:
		// buffered data
		image::Image image;
		unsigned framenumber;
		double avg_framerate;
		double t;
		vec7 pose;
		unsigned int id_;
		cv::Size size;
		char isImage;
		// graphical objects
		qdisplay::Viewer *viewer_;
		qdisplay::ImageView* view_private;
		QGraphicsTextItem* framenumber_label;
		QGraphicsTextItem* sensorpose_label;
		qdisplay::ImageView* view();
	public:
		SensorQt(ViewerAbstract *_viewer, rtslam::SensorExteroAbstract *_slamSen, RobotQt *_dispRob);
		~SensorQt();
		void bufferize();
		void render();
		void dump(std::string filename);
	public slots:
		void onKeyPress(QKeyEvent *event);
		void onMouseClick(QGraphicsSceneMouseEvent *mouseEvent, bool isClick);
};

#if DEFINE_USELESS_OBJECTS
/** **************************************************************************

*/
class LandmarkQt : public LandmarkDisplay
{
		ViewerQt *viewerQt;
	public:
		// buffered data
		// jmath::vec data_;
		// graphical objects
	public:
		LandmarkQt(ViewerAbstract *_viewer, rtslam::LandmarkAbstract *_slamLmk, MapQt *_dispMap);
		void bufferize() {}
		void render() {}
};
#endif

/** **************************************************************************

*/
class ObservationQt : public ObservationDisplay
{
		ViewerQt *viewerQt;
	public:
		// buffered data
		ObservationAbstract::Events events_;
/*		bool predicted_;
		bool visible_;
		bool measured_;
		bool matched_;
		bool updated_;*/
		jblas::vec predObs_;
		jblas::sym_mat predObsCov_;
		jblas::vec measObs_;
#if EMBED_PREDICTED_APP
		AppearanceAbstract *predictedApp_;
#endif
		unsigned int id_;
		double match_score;
		// TODO grid
		// graphical objects
		SensorQt *dispSen_; // not owner
		typedef std::list<qdisplay::Shape*> ItemList;
      ItemList items_;
   public:
		ObservationQt(ViewerAbstract *_viewer, rtslam::ObservationAbstract *_slamObs, SensorQt *_dispSen);
		~ObservationQt();
		void bufferize();
		void render();
};




}}}

#endif
#endif

