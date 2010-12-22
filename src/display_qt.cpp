/**
 * \file display_qt.chpp
 * \author croussil@laas.fr
 * \date 25/03/2010
 * \ingroup rtslam
 */

#ifdef HAVE_MODULE_QDISPLAY

#include "rtslam/display_qt.hpp"

namespace jafar {
namespace rtslam {
namespace display {

	RunStatus ViewerQt::runStatus;
	
	void ViewerQt::dump(std::string filepattern) // pattern with %d for sensor id
	{
		char filename[256];
		for(std::map<int,SensorQt*>::iterator it = sensorsList.begin(); it != sensorsList.end(); ++it)
		{
			snprintf(filename, 256, filepattern.c_str(), it->first);
			it->second->dump(filename);
		}
	}

	WorldQt::WorldQt(ViewerAbstract *_viewer, rtslam::WorldAbstract *_slamWor, WorldDisplay *garbage):
		WorldDisplay(_viewer, _slamWor, garbage), viewerQt(PTR_CAST<ViewerQt*>(_viewer)) {}
	MapQt::MapQt(ViewerAbstract *_viewer, rtslam::MapAbstract *_slamMap, WorldQt *_dispWorld):
		MapDisplay(_viewer, _slamMap, _dispWorld), viewerQt(PTR_CAST<ViewerQt*>(_viewer)) {}
	RobotQt::RobotQt(ViewerAbstract *_viewer, rtslam::RobotAbstract *_slamRob, MapQt *_dispMap):
		RobotDisplay(_viewer, _slamRob, _dispMap), viewerQt(PTR_CAST<ViewerQt*>(_viewer)) {}
	LandmarkQt::LandmarkQt(ViewerAbstract *_viewer, rtslam::LandmarkAbstract *_slamLmk, MapQt *_dispMap):
		LandmarkDisplay(_viewer, _slamLmk, _dispMap), viewerQt(PTR_CAST<ViewerQt*>(_viewer)) {}


	/** **************************************************************************
	
	*/
	SensorQt::SensorQt(ViewerAbstract *_viewer, rtslam::SensorAbstract *_slamSen, RobotQt *_dispRob): 
		SensorDisplay(_viewer, _slamSen, _dispRob), viewerQt(PTR_CAST<ViewerQt*>(_viewer)), 
		viewer_(NULL), view_private(NULL), framenumber_label(NULL), sensorpose_label(NULL)
	{
		framenumber = -1;
		t = 0.;
		pose.clear();
		id_ = slamSen_->id();
		avg_framerate = 0.;
		size = cv::Size(640,480);
		isImage = 2; // unknown
	}
	
	SensorQt::~SensorQt()
	{
		viewerQt->release(view_private);
		viewerQt->release(viewer_);
		viewerQt->release(framenumber_label);
		viewerQt->release(sensorpose_label);
	}
	
	void SensorQt::bufferize()
	{
		if (framenumber+1 <= 0) avg_framerate = 0.;
		if (slamSen_->rawCounter != framenumber+1)
		{
			if (framenumber+1 > 0) avg_framerate = (slamSen_->rawPtr->timestamp-t)/(slamSen_->rawCounter-1-framenumber);
			framenumber = slamSen_->rawCounter-1;
			t = slamSen_->rawPtr->timestamp;
			raw_ptr_t raw = slamSen_->getRaw();
			if (raw)
			{
				RawImage *rawImg = NULL;
				if (isImage == 2)
				{
					rawImg = dynamic_cast<RawImage*>(raw.get());
					isImage = ((rawImg != NULL) ? 1 : 0);
					if (isImage) size = image.size();
				} else
				if (isImage) rawImg = PTR_CAST<RawImage*>(raw.get());
				// FIXME RawSimu should export a size somehow

				if (rawImg) image = rawImg->img->clone();
			}
			pose = slamSen_->robotPtr()->pose.x();
			
		}
	}
	
	qdisplay::ImageView* SensorQt::view()
	{
		if (view_private == NULL)
		{
			viewer_ = new qdisplay::Viewer();
std::cout << "connecting slots" << std::endl;
//connect(this, SIGNAL(onKeyPress(QKeyEvent*)), this, SLOT(test_slots(QKeyEvent*)));
			if (!connect(viewer_, SIGNAL(onKeyPress(QKeyEvent*)), this, SLOT(onKeyPress(QKeyEvent*)), Qt::DirectConnection))
				std::cout << "connect onKeyPress failed" << std::endl;
			if (!connect(viewer_, SIGNAL(onMouseClick(QGraphicsSceneMouseEvent*, bool)), this, SLOT(onMouseClick(QGraphicsSceneMouseEvent*, bool)), Qt::DirectConnection))
				std::cout << "connect onMouseClick failed" << std::endl;
		
			view_private = new qdisplay::ImageView();
			viewer_->setImageView(view_private, 0, 0);
			viewer_->resize(size.width+20,size.height+20);
			viewer_->scene()->setSceneRect(0,0,size.width,size.height);
			viewer_->setBackgroundColor(0,0,0);
			std::ostringstream oss; oss << "Sensor " << id_;
			viewer_->setTitle(oss.str());
			
			framenumber_label = new QGraphicsTextItem(view_private);
			//framenumber_label->setFont(QFont( m_label->font().family(), m_fontSize));
			framenumber_label->setFont(QFont("monospace", 10, QFont::Bold));
			framenumber_label->setDefaultTextColor(QColor(0,192,0));
			framenumber_label->translate(0,15);
			
			sensorpose_label = new QGraphicsTextItem(view_private);
			//sensorpose_label->setFont(QFont( m_label->font().family(), m_fontSize));
			sensorpose_label->setFont(QFont("monospace", 10, QFont::Bold));
			sensorpose_label->setDefaultTextColor(QColor(0,192,0));
			sensorpose_label->translate(0,0);
		}
		return view_private;
	}
	
	void SensorQt::render()
	{
		switch (type_)
		{
			case SensorAbstract::PINHOLE:
			case SensorAbstract::BARRETO: {
				view()->setImage(image);
				std::ostringstream oss; oss << "#" << framenumber << "  |  " << std::setprecision(3) << avg_framerate*1000 << " ms";
				framenumber_label->setPlainText(oss.str().c_str());
				
				vec3 position = ublas::subrange(pose,0,3) * 100.0;
				vec3 euler = quaternion::q2e(ublas::subrange(pose,3,7)) * 180./M_PI;
				std::swap(euler(0), euler(2)); // FIXME-EULER-CONVENTION
				oss.str("");
//std::cout << pose << " ; " << position << " ; " << euler << std::endl;
				oss << "[" <<  std::setfill(' ') << std::setw(4) << (int)position(0) << ", " <<  
					std::setw(4) << (int)position(1) << ", " <<  std::setw(4) << (int)position(2) << "] cm ; ["
					<< std::setw(4) << (int)euler(0) << ", " <<  std::setw(4) << (int)euler(1) << ", " <<  std::setw(4) << (int)euler(2) << "] deg";
				sensorpose_label->setPlainText(oss.str().c_str());
				
				break; }
			default:
				JFR_ERROR(RtslamException, RtslamException::UNKNOWN_SENSOR_TYPE, "Don't know how to display this type of sensor" << type_);
		}
		
		// save image
		if (viewerQt->doDump)
		{
			char filename[256];
			snprintf(filename, 256, viewerQt->dump_pattern.c_str(), id_, framenumber);
			dump(filename);
		}
		viewerQt->sensorsList[id_] = this;
	}

	void SensorQt::dump(std::string filename)
	{
		switch (type_)
		{
			case SensorAbstract::PINHOLE:
			case SensorAbstract::BARRETO: {
				if (isImage == 1)
					view()->exportView(filename);
				else
					viewer_->exportView(filename);
			}
		}
	}

	/** **************************************************************************
	
	*/
	ObservationQt::ObservationQt(ViewerAbstract *_viewer, rtslam::ObservationAbstract *_slamObs, SensorQt *_dispSen):
		ObservationDisplay(_viewer, _slamObs, _dispSen), viewerQt(PTR_CAST<ViewerQt*>(_viewer)), dispSen_(_dispSen)
	{
#if EMBED_PREDICTED_APP
		predictedApp_ = NULL;
#endif
		id_ = _slamObs->landmarkPtr()->id();
		predObs_.resize(_slamObs->expectation.x().size());
		predObsCov_.resize(_slamObs->expectation.P().size1(), slamObs_->expectation.P().size2());
		measObs_.resize(_slamObs->measurement.x().size());
	}
	
	ObservationQt::~ObservationQt()
	{
//JFR_DEBUG("deleting ObservationQt and all the graphics objects " << items_.size());
		for(ItemList::iterator it = items_.begin(); it != items_.end(); ++it)
		{
// 			(*it)->setParentItem(NULL);
// 			dispSen_->viewer_->scene()->removeItem(*it);
// 			delete *it;
			viewerQt->release(*it);
		}
	}
	
	void ObservationQt::bufferize()
	{
		events_ = slamObs_->events;
		
		if (events_.visible)
		{
			if (events_.predicted)
			{
				predObs_ = slamObs_->expectation.x();
				if (events_.matched)
					predObsCov_ = slamObs_->innovation.P(); else
					predObsCov_ = slamObs_->expectation.P();
//							cout << "display: noise/exp/inn:  " << slamObs_->measurement.P() << " / " << slamObs_->expectation.P() << " / " << slamObs_->innovation.P() << " / " << endl;
			}
			if (events_.measured || events_.matched || !events_.predicted)
			{
				measObs_ = slamObs_->measurement.x();
				match_score = slamObs_->getMatchScore();
			}
		}
		
		
		switch (landmarkGeomType_)
		{
			case LandmarkDisplay::ltPoint:
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
	
	void ObservationQt::render()
	{
		switch (landmarkGeomType_)
		{
			case LandmarkDisplay::ltPoint:
			{
				bool dispPred = events_.visible && events_.predicted;
				bool dispMeas = events_.visible && (events_.measured || events_.matched || !events_.predicted);
				bool dispInit = events_.visible && !events_.predicted;

				// Build display objects if it is the first time they are displayed
				if (items_.size() != 3)
				{
					// clear
					items_.clear();
					if (!dispPred)
					{
						predObs_.clear();
						predObsCov_ = identity_mat(2);
					}
					if (!dispMeas)
					{
						measObs_.clear();
					}
					
					qdisplay::Shape *s;

					// prediction point
					s = new qdisplay::Shape(qdisplay::Shape::ShapeCross, predObs_(0), predObs_(1), 3, 3);
					s->setFontSize(viewerQt->fontSize);
					s->setVisible(false);
					items_.push_back(s);
					dispSen_->view()->addShape(s);
					
					// prediction ellipse
					s = new qdisplay::Ellipsoid(predObs_, predObsCov_, viewerQt->ellipsesScale);
					s->setVisible(false);
					items_.push_back(s);
					dispSen_->view()->addShape(s);
					
					// measure point
					s = new qdisplay::Shape(qdisplay::Shape::ShapeCrossX, measObs_(0), measObs_(1), 3, 3);
					s->setFontSize(viewerQt->fontSize);
					s->setVisible(false);
					items_.push_back(s);
					dispSen_->view()->addShape(s);
					
				}
				// Refresh the display objects every time
				{
					colorRGB c; c.set(255,255,255);
					
/*					
					lmk_state lmkstate = lmk_state_init;
					if (landmarkPhase_==LandmarkDisplay::init)      lmkstate = lmk_state_init ;
					if (landmarkPhase_==LandmarkDisplay::converged) lmkstate = lmk_state_converged ;
					lmk_events lmkstateadvanced = lmk_events_not_predicted ;
					if (events_.predicted) lmkstateadvanced = lmk_events_predicted ;
					if (events_.matched)   lmkstateadvanced = lmk_events_matched ;
					if (updated_)   lmkstateadvanced = lmk_events_updated ;
*/

					// prediction point
					ItemList::iterator it = items_.begin();
					if (dispPred)
					{
						c = getColorRGB(ColorManager::getColorObject_prediction(landmarkPhase_,events_)) ;
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
						qdisplay::Ellipsoid *ell = PTR_CAST<qdisplay::Ellipsoid*>(*it);
						ell->set(predObs_, predObsCov_, viewerQt->ellipsesScale);
					}
					(*it)->setVisible(dispPred);
//JFR_DEBUG("drawn ellipse for obs " << id_ << " at " << predObs_ << " with size " << predObsCov_ << ", scale " << viewerQt->ellipsesScale << " and visibility " << dispPred);
					
					// measure point
					++it;
//std::cout << "display obs " << id_ << " with flags visible " << events_.visible << " matched " << events_.matched
//		<< " predicted " << events_.predicted << " position " << measObs_ << std::endl;
					if (dispMeas)
					{
						c = getColorRGB(ColorManager::getColorObject_measure(landmarkPhase_,events_)) ;
						if (dispInit)
						{
							(*it)->setFontColor(c.R,c.G,c.B); //
							(*it)->setLabel(jmath::toStr(id_).c_str());
						} else
						{
							(*it)->setLabel("");
						}
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


	void SensorQt::onKeyPress(QKeyEvent *event)
	{
		switch (event->key())
		{
			case Qt::Key_Space:
			case Qt::Key_P: { // pause/run
				boost::unique_lock<boost::mutex> runStatus_lock(viewerQt->runStatus.mutex);
				viewerQt->runStatus.pause = !viewerQt->runStatus.pause;
				runStatus_lock.unlock();
				viewerQt->runStatus.condition.notify_all();
				std::cout << "pause: " << (viewerQt->runStatus.pause ? "ON" : "OFF") << std::endl;
				break;
			}
			case Qt::Key_Right:
			case Qt::Key_Period:
			case Qt::Key_N: { // next frame
				boost::unique_lock<boost::mutex> runStatus_lock(viewerQt->runStatus.mutex);
				viewerQt->runStatus.next = 1;
				viewerQt->runStatus.pause = 1;
				runStatus_lock.unlock();
				viewerQt->runStatus.condition.notify_all();
				break;
			}
			case Qt::Key_A:
			case Qt::Key_R:
			case Qt::Key_F: { // fast mode (don't render all)
				boost::unique_lock<boost::mutex> runStatus_lock(viewerQt->runStatus.mutex);
				viewerQt->runStatus.render_all = !viewerQt->runStatus.render_all;
				runStatus_lock.unlock();
				std::cout << "render-all: " << (viewerQt->runStatus.render_all ? "ON" : "OFF") << std::endl;
				break;
			}
			case Qt::Key_Q: { // quit
				QApplication::quit();
				break;
			}
		}
	}
	
	void SensorQt::onMouseClick(QGraphicsSceneMouseEvent *mouseEvent, bool isClick)
	{
		if (!isClick) return;
		QGraphicsItem *clickedItem = viewer_->scene()->itemAt(mouseEvent->buttonDownScenePos(mouseEvent->button()));
		ObservationAbstract *clickedObs = NULL;
		
		for(SensorAbstract::DataManagerList::iterator itDm = slamSen_->dataManagerList().begin();
		    !clickedObs && itDm != slamSen_->dataManagerList().end(); ++itDm)
		for(DataManagerAbstract::ObservationList::iterator itObs = (*itDm)->observationList().begin();
		    !clickedObs && itObs != (*itDm)->observationList().end(); ++itObs)
		{
			ObservationQt *obs = PTR_CAST<ObservationQt*>((*itObs)->displayData[ViewerQt::id()]);
			
			for(ObservationQt::ItemList::iterator itItem = obs->items_.begin();
			    !clickedObs && itItem != obs->items_.end(); ++itItem)
				if ((*itItem)->hasItem(clickedItem)) clickedObs = itObs->get();
		}
		
		if (clickedObs)
		{
			std::cout << "----------------------------------------------- at frame " << framenumber << std::endl;
			std::cout << *clickedObs << std::endl;
			std::cout << clickedObs->landmark() << std::endl;
		}

		
	}



}}}

#include "display_qt.moc"

#endif
