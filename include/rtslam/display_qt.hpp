/**
 * \file display_qt.hpp
 * \author croussil@laas.fr
 * \date 25/03/2010
 * File defining a display architecture for qt with qdisplay module
 * \ingroup rtslam
 */

#ifndef DISPLAY_QT__HPP_
#define DISPLAY_QT__HPP_

#include "qdisplay/ImageView.hpp"
#include "qdisplay/Viewer.hpp"
#include "qdisplay/Shape.hpp"
#include "qdisplay/Ellipsoid.hpp"
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
// Objects

#if DEFINE_USELESS_OBJECTS

/** **************************************************************************

*/
class WorldQt : public WorldDisplay
{
	public:
		WorldQt(rtslam::WorldAbstract *_slamWor, WorldDisplay *garbage):
			WorldDisplay(_slamWor, garbage) {}
		void bufferize() {}
		void render() {}
};

/** **************************************************************************

*/
class MapQt : public MapDisplay
{
	public:
		MapQt(rtslam::MapAbstract *_slamMap, WorldDisplay *_dispWorld):
			MapDisplay(_slamMap, _dispWorld) {}
		void bufferize() {}
		void render() {}
};

/** **************************************************************************

*/
class RobotQt : public RobotDisplay
{
	public:
		// buffered data
		//Gaussian pose_;
		//std::string model3d_;
		// graphical objects
	public:
		RobotQt(rtslam::RobotAbstract *_slamRob, MapDisplay *_dispMap):
			RobotDisplay(_slamRob, _dispMap) {}
		void bufferize() {}
		void render() {}
};
#endif

/** **************************************************************************

*/
class SensorQt : public SensorDisplay
{
	public:
		// buffered data
		image::Image image;
		int framenumber;
		double avg_framerate;
		double t;
		// graphical objects
		qdisplay::Viewer *viewer_;
		qdisplay::ImageView* view_;
		QGraphicsTextItem* framenumber_label;
	public:
		SensorQt(rtslam::SensorAbstract *_slamSen, RobotDisplay *_dispRob): 
			SensorDisplay(_slamSen, _dispRob)
		{
			viewer_ = new qdisplay::Viewer();
			view_ = new qdisplay::ImageView();
			viewer_->setImageView(view_, 0, 0);
			viewer_->resize(660,500);
			framenumber = 0;
			t = 0.;
			avg_framerate = 0.;
			
			framenumber_label = new QGraphicsTextItem(view_);
			//framenumber_label->setFont(QFont( m_label->font().family(), m_fontSize));
			framenumber_label->setDefaultTextColor(QColor(0,192,0));
			framenumber_label->translate(0,0);
		}
		~SensorQt()
		{
			delete view_;
			delete viewer_;
			delete framenumber_label;
		}
		void bufferize()
		{
			if (slamSen_->rawCounter != framenumber)
			{
				avg_framerate = (slamSen_->rawPtr->timestamp-t)/(slamSen_->rawCounter-framenumber);
				if (framenumber == 0) avg_framerate = 0.;
				framenumber = slamSen_->rawCounter;
				t = slamSen_->rawPtr->timestamp;
				raw_ptr_t raw = slamSen_->getRaw();
				if (raw) image = static_cast<RawImage&>(*raw).img->clone();
			}
		}
		void render()
		{
			switch (type_)
			{
				case SensorDisplay::stCameraPinhole:
				case SensorDisplay::stCameraBarreto: {
					view_->setImage(image);
					std::ostringstream oss; oss << "#" << framenumber << "  |  " << std::setprecision(3) << avg_framerate*1000 << " ms";
					framenumber_label->setPlainText(oss.str().c_str());
					break; }
				default:
					JFR_ERROR(RtslamException, RtslamException::UNKNOWN_SENSOR_TYPE, "Don't know how to display this type of sensor" << type_);
			}
		}
};

#if DEFINE_USELESS_OBJECTS
/** **************************************************************************

*/
class LandmarkQt : public LandmarkDisplay
{
	public:
		// buffered data
		// jmath::vec data_;
		// graphical objects
	public:
		LandmarkQt(rtslam::LandmarkAbstract *_slamLmk, MapDisplay *_dispMap):
			LandmarkDisplay(_slamLmk, _dispMap) {}
		void bufferize() {}
		void render() {}
};
#endif

/** **************************************************************************

*/
class ObservationQt : public ObservationDisplay
{
	public:
		// buffered data
		bool visible_;
		bool matched_;
		bool predicted_;
		bool updated_;
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
		qdisplay::ImageView* view_; // not owner
		typedef std::list<qdisplay::Shape*> ItemList;
		ItemList items_;
	public:
		ObservationQt(rtslam::ObservationAbstract *_slamObs, SensorQt *_dispSen):
			ObservationDisplay(_slamObs, _dispSen), view_(_dispSen->view_)
		{
#if EMBED_PREDICTED_APP
			predictedApp_ = NULL;
#endif
			switch (landmarkGeomType_)
			{
				case LandmarkDisplay::ltPoint:
					predObs_.resize(2);
					predObsCov_.resize(2);
					measObs_.resize(2);
					break;
				default:
					JFR_ERROR(RtslamException, RtslamException::UNKNOWN_FEATURE_TYPE, "Don't know how to display this type of landmark: " << landmarkGeomType_);
					break;
			}
		}
		~ObservationQt()
		{
//JFR_DEBUG("deleting ObservationQt and all the graphics objects " << items_.size());
			for(ItemList::iterator it = items_.begin(); it != items_.end(); ++it)
			{
				//(*it)->setParentItem(NULL);
				//viewer_->scene()->removeItem(*it);
				delete *it;
			}
		}
		
		void bufferize()
		{
			switch (landmarkGeomType_)
			{
				case LandmarkDisplay::ltPoint:
					//LandmarkEuclidean *lmk = static_cast<LandmarkEuclidean*>(slamObs_->landmarkPtr->convertToStandardParametrization());
					predicted_ = slamObs_->events.predicted;
					matched_ = slamObs_->events.matched;
					visible_ = slamObs_->events.visible;
					updated_ = slamObs_->events.updated;
					landmarkPhase_ = LandmarkDisplay::convertPhase(&*slamObs_->landmarkPtr());
					id_ = slamObs_->landmarkPtr()->id();
					if (visible_)
					{
						if (predicted_)
						{
							predObs_ = slamObs_->expectation.x();
							predObsCov_ = slamObs_->innovation.P();
//							cout << "display: noise/exp/inn:  " << slamObs_->measurement.P() << " / " << slamObs_->expectation.P() << " / " << slamObs_->innovation.P() << " / " << endl;
						}
						if (matched_ || !predicted_)
						{
							measObs_ = slamObs_->measurement.x();
							match_score = slamObs_->getMatchScore();
						}
					}
#if EMBED_PREDICTED_APP
					delete predictedApp_;
					predictedApp_ = slamObs_->predictedAppearance->clone();
#endif
					break;
				default:
					JFR_ERROR(RtslamException, RtslamException::UNKNOWN_FEATURE_TYPE, "Don't know how to display this type of landmark: " << landmarkGeomType_);
					break;
			}
		}
		void render()
		{
			switch (landmarkGeomType_)
			{
				case LandmarkDisplay::ltPoint:
				{
					// Build display objects if it is the first time they are displayed
					if (items_.size() != 3)
					{
						// clear
						items_.clear();
						if (!(visible_ && predicted_))
						{
							predObs_.clear();
							predObsCov_ = identity_mat(2);
						}
						if (!(visible_ && matched_))
						{
							measObs_.clear();
						}
						
						qdisplay::Shape *s;

						// prediction point
						s = new qdisplay::Shape(qdisplay::Shape::ShapeCross, predObs_(0), predObs_(1), 3, 3);
						s->setColor(0,0,0);
						s->setFontSize(8);
						s->setFontColor(0,0,0);
						s->setVisible(false);
						items_.push_back(s);
						view_->addShape(s);
						
						// prediction ellipse
						s = new qdisplay::Ellipsoid(predObs_, predObsCov_, 3.0);
						s->setColor(0,0,0);
						s->setVisible(false);
						items_.push_back(s);
						view_->addShape(s);
						
						// measure point
						s = new qdisplay::Shape(qdisplay::Shape::ShapeCross, measObs_(0), measObs_(1), 3, 3, 45);
						s->setColor(0,0,0);
						s->setVisible(false);
						items_.push_back(s);
						view_->addShape(s);
						
					}
					// Refresh the display objects every time
					{
						colorRGB c; c.set(255,255,255);
						lmk_state lmkstate = lmk_state_init;
						if (landmarkPhase_==LandmarkDisplay::init)      lmkstate = lmk_state_init ;
						if (landmarkPhase_==LandmarkDisplay::converged) lmkstate = lmk_state_converged ;
						lmk_state_advanced lmkstateadvanced = lmk_state_advanced_not_predicted ;
						if (predicted_) lmkstateadvanced = lmk_state_advanced_predicted ;
						if (matched_)   lmkstateadvanced = lmk_state_advanced_matched ;
						if (updated_)   lmkstateadvanced = lmk_state_advanced_updated ;


						bool dispPred = visible_ && predicted_;
						bool dispMeas = visible_ && (matched_ || !predicted_);

						// prediction point
						ItemList::iterator it = items_.begin();
						if (dispPred)
						{
							c = getColorRGB(getColorObject_prediction(lmkstate,lmkstateadvanced)) ;
							(*it)->setColor(c.R,c.G,c.B); //
							(*it)->setFontColor(c.R,c.G,c.B); //
							std::ostringstream oss; oss << id_; if (dispMeas) oss << " - " << int(match_score*100);
							(*it)->setLabel(oss.str().c_str());
							(*it)->setPos(predObs_(0), predObs_(1));
						}
						(*it)->setVisible(dispPred);
						
						// prediction ellipse
						++it;
						if (dispPred)
						{
							(*it)->setColor(c.R,c.G,c.B); // yellow
							qdisplay::Ellipsoid *ell = dynamic_cast<qdisplay::Ellipsoid*>(*it);
							ell->set(predObs_, predObsCov_, 3.0);
						}
						(*it)->setVisible(dispPred);
						
						// measure point
						++it;
//std::cout << "display obs " << id_ << " with flags visible " << visible_ << " matched " << matched_
//		<< " predicted " << predicted_ << " position " << measObs_ << std::endl;
						if (dispMeas)
						{
							c = getColorRGB(getColorObject_measure(lmkstate,lmkstateadvanced)) ;
							(*it)->setColor(c.R,c.G,c.B); // red
							(*it)->setPos(measObs_(0), measObs_(1));
						}
						(*it)->setVisible(dispMeas);

#if EMBED_PREDICTED_APP
						// display predicted appearance
						switch (slamObs_->sensorPtr()->type)
						{
							case SensorAbstract::PINHOLE: case SensorAbstract::BARRETO:
							{
								AppearanceImagePoint* appImgPtr = PTR_CAST<AppearanceImagePoint*>(slamObs_->predictedAppearance.get());
								jblas::veci shift(2); shift(0) = (appImgPtr->patch.width()-1)/2; shift(1) = (appImgPtr->patch.height()-1)/2;
								appImgPtr->patch.robustCopy(PTR_CAST<SensorQt*>(dispSen_)->image, 0, 0, predObs_(0)-shift(0), predObs_(1)-shift(1));
							}
							default:
							{

							}

						}
#endif

					}
					break;
				}

				default:
					JFR_ERROR(RtslamException, RtslamException::UNKNOWN_FEATURE_TYPE, "Don't know how to display this type of landmark: " << landmarkGeomType_);
			}
		}


};



//******************************************************************************
// Viewer

#if DEFINE_USELESS_OBJECTS
typedef Viewer<WorldQt,MapQt,RobotQt,SensorQt,LandmarkQt,ObservationQt> ViewerQt;
#else
#error "does not work"
typedef Viewer<WorldDisplay,MapDisplay,RobotDisplay,SensorQt,LandmarkDisplay,ObservationQt> ViewerQt;
#endif



}}}

#endif

