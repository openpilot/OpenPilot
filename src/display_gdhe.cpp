/**
 * \file display_gdhe.cpp
 * \date 25/03/2010
 * \author croussil
 * \ingroup rtslam
 */

#include "rtslam/display_gdhe.hpp"
#ifdef HAVE_MODULE_GDHE
#include "rtslam/quatTools.hpp"
#include "rtslam/ahpTools.hpp"
#include "rtslam/landmarkAnchoredHomogeneousPointsLine.hpp"

#ifdef HAVE_MODULE_DSEG
#include "rtslam/descriptorImageSeg.hpp"
#endif

//#define DISPLAY_SEGMENT_DEPTH

#include "jmath/angle.hpp"

namespace jafar {
namespace rtslam {
namespace display {


	WorldGdhe::WorldGdhe(ViewerAbstract *_viewer, rtslam::WorldAbstract *_slamWor, WorldDisplay *garbage):
		WorldDisplay(_viewer, _slamWor, garbage), viewerGdhe(PTR_CAST<ViewerGdhe*>(_viewer))
	{
		
	}
	
	MapGdhe::MapGdhe(ViewerAbstract *_viewer, rtslam::MapAbstract *_slamMap, WorldGdhe *_dispWorld):
		MapDisplay(_viewer, _slamMap, _dispWorld), viewerGdhe(PTR_CAST<ViewerGdhe*>(_viewer)), frame(NULL)
	{ }
	
	MapGdhe::~MapGdhe()
	{
		viewerGdhe->release(frame);
	}
		
	void MapGdhe::bufferize()
	{
		poseQuat = ublas::subrange(slamMap_->state.x(), 0, 7);
	}
	
	void MapGdhe::render()
	{
		#if 0
		jblas::vec poseEuler = quaternion::q2e_frame(poseQuat);
		for(int i = 3; i < 6; ++i) poseEuler(i) = jmath::radToDeg(poseEuler(i));
		std::swap(poseEuler(3), poseEuler(5)); // FIXME-EULER-CONVENTION
		
		frame.setPose(poseEuler);
		frame.refresh();
		#endif
		
		if (frame == NULL)
		{
			frame = new gdhe::Frame(1);
			frame->setColor(216,216,216);
			viewerGdhe->client.addObject(frame, true);
		}
	}
	
	
	
	
	RobotGdhe::RobotGdhe(ViewerAbstract *_viewer, rtslam::RobotAbstract *_slamRob, MapGdhe *_dispMap):
		RobotDisplay(_viewer, _slamRob, _dispMap), viewerGdhe(PTR_CAST<ViewerGdhe*>(_viewer)), robot(NULL), uncertEll(NULL), traj(NULL)
	{
	}
	
	RobotGdhe::~RobotGdhe()
	{
		viewerGdhe->release(robot);
		viewerGdhe->release(uncertEll);
		viewerGdhe->release(traj);
	}
	
	void RobotGdhe::bufferize()
	{
		poseQuat = slamRob_->pose.x();
		poseQuatUncert = slamRob_->pose.P();
	}
	
	void RobotGdhe::render()
	{
//std::cout << "#### New FRAME" << std::endl;
		if (robot == NULL)
		{
			robot = new gdhe::Robot(viewerGdhe->robot_model);
			viewerGdhe->client.addObject(robot, false);
		}
		if (uncertEll == NULL)
		{
			uncertEll = new gdhe::EllipsoidWire();
			uncertEll->setColor(255,255,0);
			viewerGdhe->client.addObject(uncertEll, false);
		}
		if (traj == NULL)
		{
			traj = new gdhe::Trajectory();
			traj->setColor(0,255,0);
			viewerGdhe->client.addObject(traj, false);
		}

		// convert pose from quat to euler degrees
		jblas::vec poseEuler(6);
		jblas::vec angleEuler(3);
		jblas::sym_mat uncertEuler(3);
		ublas::subrange(poseEuler,0,3) = ublas::subrange(poseQuat,0,3);
		quaternion::q2e(ublas::subrange(poseQuat,3,7), ublas::project(poseQuatUncert,ublas::range(3,7),ublas::range(3,7)), angleEuler, uncertEuler);
		ublas::subrange(poseEuler,3,6) = angleEuler;
		//ublas::subrange(poseEuler,3,6) = quaternion::q2e(ublas::subrange(poseQuat,3,7));
		for(int i = 3; i < 6; ++i) poseEuler(i) = jmath::radToDeg(poseEuler(i));
		std::swap(poseEuler(3), poseEuler(5)); // FIXME-EULER-CONVENTION
		robot->setPose(poseEuler);
/*JFR_DEBUG("robot pos  : " << ublas::subrange(poseQuat,0,3));
JFR_DEBUG("robot euler: " << angleEuler);
JFR_DEBUG("robot POS  : " << ublas::project(poseQuatUncert,ublas::range(0,3),ublas::range(0,3)));
JFR_DEBUG("robot EULER: " << uncertEuler);*/
		// uncertainty
		uncertEll->set(ublas::subrange(poseQuat,0,3), ublas::project(poseQuatUncert,ublas::range(0,3),ublas::range(0,3)), viewerGdhe->ellipsesScale);
		uncertEll->refresh();
		// camera target
		//viewerGdhe->client.setCameraTarget(poseEuler(0), poseEuler(1), poseEuler(2));
		robot->refresh();
		
		// trajectory
		traj->addPoint(poseQuat(0),poseQuat(1),poseQuat(2));
		traj->refresh();
	}
	
	SensorGdhe::SensorGdhe(ViewerAbstract *_viewer, rtslam::SensorExteroAbstract *_slamRob, RobotGdhe *_dispMap):
		SensorDisplay(_viewer, _slamRob, _dispMap), viewerGdhe(PTR_CAST<ViewerGdhe*>(_viewer)) {}
	
	LandmarkGdhe::LandmarkGdhe(ViewerAbstract *_viewer, rtslam::LandmarkAbstract *_slamLmk, MapGdhe *_dispMap):
		LandmarkDisplay(_viewer, _slamLmk, _dispMap), viewerGdhe(PTR_CAST<ViewerGdhe*>(_viewer))
	{
		id_ = _slamLmk->id();
		lmkType_ = _slamLmk->type;
		state_.resize(_slamLmk->state.x().size());
		cov_.resize(_slamLmk->state.P().size1(),_slamLmk->state.P().size2());
	}
	
	LandmarkGdhe::~LandmarkGdhe()
	{
		for(ItemList::iterator it = items_.begin(); it != items_.end(); ++it)
			viewerGdhe->release(*it);
	}

	
	void LandmarkGdhe::bufferize()
	{
		uchar *events = (uchar*)&events_;
		memset(events, 0, sizeof(ObservationAbstract::Events));
		for(LandmarkAbstract::ObservationList::iterator obs = slamLmk_->observationList().begin(); obs != slamLmk_->observationList().end(); ++obs)
		{
			uchar *obsevents = (uchar*)&((*obs)->events);
			for(size_t i = 0; i < sizeof(ObservationAbstract::Events); i++) events[i] |= obsevents[i];
		}
/*		
		events_.predicted_ = events_.visible_ = events_.measured_ = events_.matched_ = events_.updated_ = false;
		for(LandmarkAbstract::ObservationList::iterator obs = slamLmk_->observationList().begin(); obs != slamLmk_->observationList().end(); ++obs)
		{
			events_.predicted |= (*obs)->events.predicted;
			events_.visible |= (*obs)->events.visible;
			events_.measured |= (*obs)->events.measured;
			events_.matched |= (*obs)->events.matched;
			events_.updated |= (*obs)->events.updated;
		}
*/		
		state_ = slamLmk_->state.x();
		cov_ = slamLmk_->state.P();
	}
	
	void LandmarkGdhe::render()
	{
		//const double sph_radius = 0.01;
		switch (lmkType_)
		{
			case LandmarkAbstract::PNT_EUC:
			{
				// Build display objects if it is the first time they are displayed
				if (items_.size() != 1)
				{
					// clear
					items_.clear();

/*					// sphere
					gdhe::Sphere *sph = new gdhe::Sphere(0.01, 12);
					sph->setLabel("");
					items_.push_back(sph);
					viewerGdhe->client.addObject(sph, false);
*/
					// ellipsoid
					gdhe::Ellipsoid *ell = new gdhe::Ellipsoid(12);
					ell->setLabel("");
					items_.push_back(ell);
					viewerGdhe->client.addObject(ell, false);
				}
				// Refresh the display objects every time
				{
					colorRGB c; c.set(255,255,255);

/*					// sphere
					ItemList::iterator it = items_.begin();
					gdhe::Sphere *sph = PTR_CAST<gdhe::Sphere*>(*it);
					sph->setRadius(sph_radius);
					c = getColorRGB(ColorManager::getColorObject_prediction(phase_,events_)) ;
					(*it)->setColor(c.R,c.G,c.B); //
					jblas::vec3 position = lmkAHP::ahp2euc(state_);
					(*it)->setPose(position(0), position(1), position(2), 0, 0, 0);
					(*it)->setLabelColor(c.R,c.G,c.B);
					(*it)->setLabel(jmath::toStr(id_));
					(*it)->refresh();
*/				
					// ellipsoid
					ItemList::iterator it = items_.begin();
					gdhe::Ellipsoid *ell = PTR_CAST<gdhe::Ellipsoid*>(*it);
					ell->set(state_, cov_, viewerGdhe->ellipsesScale);
					c = getColorRGB(ColorManager::getColorObject_prediction(phase_,events_)) ;
					(*it)->setColor(c.R,c.G,c.B); //
					(*it)->setLabelColor(c.R,c.G,c.B);
					(*it)->setLabel(jmath::toStr(id_));
					(*it)->refresh();
				}
				break;
			}
			case LandmarkAbstract::PNT_AH:
			{
				// Build display objects if it is the first time they are displayed
				if (items_.size() != 2)
				{
					// clear
					items_.clear();

/*					// sphere
					gdhe::Sphere *sph = new gdhe::Sphere(sph_radius, 12);
					sph->setLabel("");
					items_.push_back(sph);
					viewerGdhe->client.addObject(sph, false);
*/					
					// ellipsoid
					gdhe::Ellipsoid *ell = new gdhe::Ellipsoid(12);
					ell->setLabel("");
					items_.push_back(ell);
					viewerGdhe->client.addObject(ell, false);
					
					// segment
					gdhe::Polyline *seg = new gdhe::Polyline();
					items_.push_back(seg);
					viewerGdhe->client.addObject(seg, false);
				}
				// Refresh the display objects every time
				{
					colorRGB c; c.set(255,255,255);
/*
					// sphere
					ItemList::iterator it = items_.begin();
					gdhe::Sphere *sph = PTR_CAST<gdhe::Sphere*>(*it);
					sph->setRadius(sph_radius);
					c = getColorRGB(ColorManager::getColorObject_prediction(phase_,events_)) ;
					(*it)->setColor(c.R,c.G,c.B); //
					jblas::vec3 position = lmkAHP::ahp2euc(state_);
					(*it)->setPose(position(0), position(1), position(2), 0, 0, 0);
					(*it)->setLabelColor(c.R,c.G,c.B);
					(*it)->setLabel(jmath::toStr(id_));
					(*it)->refresh();
					*/
					// ellipsoid
					ItemList::iterator it = items_.begin();
					gdhe::Ellipsoid *ell = PTR_CAST<gdhe::Ellipsoid*>(*it);
					jblas::vec xNew; jblas::sym_mat pNew; slamLmk_->reparametrize(LandmarkEuclideanPoint::size(), xNew, pNew);
//std::cout << "x_ahp " << state_ << " P_ahp " << cov_ << " ; x_euc " << xNew << " P_euc " << pNew << std::endl;
					ell->setCompressed(xNew, pNew, viewerGdhe->ellipsesScale);
//					ell->set(xNew, pNew, viewerGdhe->ellipsesScale);
					c = getColorRGB(ColorManager::getColorObject_prediction(phase_,events_)) ;
					(*it)->setColor(c.R,c.G,c.B); //
					(*it)->setLabelColor(c.R,c.G,c.B);
					(*it)->setLabel(jmath::toStr(id_));
					(*it)->refresh();
					
					
					// segment
					++it;
					gdhe::Polyline *seg = PTR_CAST<gdhe::Polyline*>(*it);
					seg->clear();
					double id_std = sqrt(cov_(6,6))*viewerGdhe->ellipsesScale;
					jblas::vec3 position = lmkAHP::ahp2euc(state_);
					jblas::vec7 state = state_; 
					state(6) = state_(6) - id_std; if (state(6) < 1e-4) state(6) = 1e-4;
					jblas::vec3 positionExt = lmkAHP::ahp2euc(state);
					seg->addPoint(positionExt(0)-position(0), positionExt(1)-position(1), positionExt(2)-position(2));
					state(6) = state_(6) + id_std;
					positionExt = lmkAHP::ahp2euc(state);
					seg->addPoint(positionExt(0)-position(0), positionExt(1)-position(1), positionExt(2)-position(2));
					(*it)->setColor(c.R,c.G,c.B);
					(*it)->setPose(position(0), position(1), position(2), 0, 0, 0);
					(*it)->refresh();
				}
				break;
         }
         case LandmarkAbstract::LINE_AHPL:
         {
            // Build display objects if it is the first time they are displayed
            #ifdef DISPLAY_SEGMENT_DEPTH
               if (items_.size() != 5)
            #else
               if (items_.size() != 3)
            #endif
            {
               // clear
               items_.clear();

               // ellipsoids
               gdhe::Ellipsoid *ell = new gdhe::Ellipsoid(12);
               ell->setLabel("");
               items_.push_back(ell);
               viewerGdhe->client.addObject(ell, false);
               ell = new gdhe::Ellipsoid(12);
               ell->setLabel("");
               items_.push_back(ell);
               viewerGdhe->client.addObject(ell, false);
               // segments
               gdhe::Polyline *seg = new gdhe::Polyline();
               items_.push_back(seg);
               viewerGdhe->client.addObject(seg, false);
               #ifdef DISPLAY_SEGMENT_DEPTH
                   seg = new gdhe::Polyline();
                   items_.push_back(seg);
                   viewerGdhe->client.addObject(seg, false);
                   seg = new gdhe::Polyline();
                   items_.push_back(seg);
                   viewerGdhe->client.addObject(seg, false);
               #endif
            }
            // Refresh the display objects every time
            {
               colorRGB c; c.set(255,255,255);

               // ellipsoids
               ItemList::iterator it = items_.begin();
               jblas::vec xNew;  jblas::sym_mat pNew;
               jblas::vec xNew1; jblas::sym_mat pNew1;
               jblas::vec xNew2; jblas::sym_mat pNew2;
               slamLmk_->reparametrize(LandmarkEuclideanPoint::size()*2, xNew, pNew);
               //slamLmk_->reparametrize(LandmarkAnchoredHomogeneousPointsLine::reparamSize(), xNew, pNew);
               xNew1 = subrange(xNew,0,3);
               xNew2 = subrange(xNew,3,6);
               pNew1 = subrange(pNew,0,3,0,3);
               pNew2 = subrange(pNew,3,6,3,6);

               gdhe::Ellipsoid *ell = PTR_CAST<gdhe::Ellipsoid*>(*it);
               ell->setCompressed(xNew1, pNew1, viewerGdhe->ellipsesScale);
               c = getColorRGB(ColorManager::getColorObject_prediction(phase_,events_)) ;
               (*it)->setColor(c.R,c.G,c.B); //
               (*it)->setLabelColor(c.R,c.G,c.B);
               (*it)->setLabel(jmath::toStr(id_));
               (*it)->refresh();
               ++it;
               ell = PTR_CAST<gdhe::Ellipsoid*>(*it);
               ell->setCompressed(xNew2, pNew2, viewerGdhe->ellipsesScale);
               c = getColorRGB(ColorManager::getColorObject_prediction(phase_,events_)) ;
               (*it)->setColor(c.R,c.G,c.B); //
               (*it)->setLabelColor(c.R,c.G,c.B);
               (*it)->setLabel(jmath::toStr(id_));
               (*it)->refresh();

               // segments
               gdhe::Polyline *seg;
               #ifdef DISPLAY_SEGMENT_DEPTH
                  ++it;
                  seg = PTR_CAST<gdhe::Polyline*>(*it);
                  seg->clear();
                  double id_std = sqrt(cov_(6,6))*viewerGdhe->ellipsesScale;
                  jblas::vec7 _state1 = subrange(state_,0,7);
                  jblas::vec3 position = lmkAHP::ahp2euc(_state1);
                  jblas::vec7 state = _state1;
                  state(6) = _state1(6) - id_std; if (state(6) < 1e-4) state(6) = 1e-4;
                  jblas::vec3 positionExt = lmkAHP::ahp2euc(state);
                  seg->addPoint(positionExt(0)-position(0), positionExt(1)-position(1), positionExt(2)-position(2));
                  state(6) = _state1(6) + id_std;
                  positionExt = lmkAHP::ahp2euc(state);
                  seg->addPoint(positionExt(0)-position(0), positionExt(1)-position(1), positionExt(2)-position(2));
                  (*it)->setColor(c.R,c.G,c.B);
                  (*it)->setPose(position(0), position(1), position(2), 0, 0, 0);
                  (*it)->refresh();
                  ++it;
                  seg = PTR_CAST<gdhe::Polyline*>(*it);
                  seg->clear();
                  id_std = sqrt(cov_(6,6))*viewerGdhe->ellipsesScale;
                  jblas::vec7 _state2 = subrange(state_,0,7);
                  subrange(_state2,3,7) = subrange(state_,7,11);
                  position = lmkAHP::ahp2euc(_state2);
                  state = _state2;
                  state(6) = _state2(6) - id_std; if (state(6) < 1e-4) state(6) = 1e-4;
                  positionExt = lmkAHP::ahp2euc(state);
                  seg->addPoint(positionExt(0)-position(0), positionExt(1)-position(1), positionExt(2)-position(2));
                  state(6) = _state2(6) + id_std;
                  positionExt = lmkAHP::ahp2euc(state);
                  seg->addPoint(positionExt(0)-position(0), positionExt(1)-position(1), positionExt(2)-position(2));
                  (*it)->setColor(c.R,c.G,c.B);
                  (*it)->setPose(position(0), position(1), position(2), 0, 0, 0);
                  (*it)->refresh();
               #endif
               // Linking segment
				#ifdef HAVE_MODULE_DSEG
					jblas::vec3 xMiddle = (xNew1 + xNew2)/2;
					desc_img_seg_fv_ptr_t descriptorSpec = SPTR_CAST<DescriptorImageSegFirstView>(slamLmk_->descriptorPtr);
					float left_extremity = 1.0;
					float right_extremity = 1.0;
					if(descriptorSpec != NULL) {
						left_extremity = descriptorSpec->getLeftExtremity();
						right_extremity = descriptorSpec->getRightExtremity();
					}
					xNew1 = left_extremity * (xNew1 - xMiddle) + xMiddle;
					xNew2 = right_extremity * (xNew2 - xMiddle) + xMiddle;
               ++it;
               seg = PTR_CAST<gdhe::Polyline*>(*it);
               seg->clear();
               seg->addPoint(xNew1(0), xNew1(1), xNew1(2));
               seg->addPoint(xNew2(0), xNew2(1), xNew2(2));
               (*it)->setColor(c.R,c.G,c.B);
               (*it)->setPose(0,0,0,0,0,0);
               (*it)->refresh();
				#endif
            }
            break;
         }
			default:
				JFR_ERROR(RtslamException, RtslamException::UNKNOWN_FEATURE_TYPE, "Don't know how to display this type of landmark: " << type_);
		}
	}

	
	
	ObservationGdhe::ObservationGdhe(ViewerAbstract *_viewer, rtslam::ObservationAbstract *_slamLmk, SensorGdhe *_dispMap):
		ObservationDisplay(_viewer, _slamLmk, _dispMap), viewerGdhe(PTR_CAST<ViewerGdhe*>(_viewer)) {}

}}}

#endif



