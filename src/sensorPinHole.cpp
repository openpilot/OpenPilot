/**
 * sensorPinHole.cpp
 *
 * \date 10/03/2010
 * \author jsola@laas.fr
 *
 *  \file sensorPinHole.cpp
 *
 *  ## Add a description here ##
 *
 * \ingroup rtslam
 */

#include "rtslam/pinholeTools.hpp"
#include "rtslam/sensorPinHole.hpp"
#include "rtslam/rawImage.hpp"

#include "image/Image.hpp"

#include "qdisplay/Viewer.hpp"
#include "qdisplay/ViewerManager.hpp"
#include "qdisplay/ImageView.hpp"

namespace jafar {
	namespace rtslam {
		using namespace std;
		using namespace jafar::image;


		///////////////////////////////////////
		// Class sensor pin hole
		///////////////////////////////////////

		SensorPinHole::SensorPinHole(const robot_ptr_t & _robPtr, filtered_obj_t inFilter) :
			SensorAbstract(_robPtr, inFilter) {
			type("Pin-hole-camera");
			cout << "Created pinhole." << endl;
		}

		SensorPinHole::SensorPinHole(const simulation_t dummy, const robot_ptr_t & _robPtr) :
				SensorAbstract(ObjectAbstract::FOR_SIMULATION, _robPtr) {
			type("Pin-hole-camera");
			cout << "Created pinhole." << endl;
		}

		void SensorPinHole::set_parameters(const jblas::vec2 & imSz, const jblas::vec4 & k, const jblas::vec & d, const jblas::vec & c) {
			imgSize   = imSz;
			intrinsic = k;
			distortion.resize(d.size());
			distortion = d;
			correction.resize(c.size());
			correction = c;
		}

		void SensorPinHole::setup(const size_t _id, const string & _name, const vec7 & _pose, const vec7 & _std, const jblas::vec2 & _s, const vec4 & _k, const vec & _d, const vec & _c){
			SensorAbstract::MapObject::ObjectAbstract::setup(_id, _name);
			pose.x(_pose);
			pose.std(_std);
			set_parameters(_s, _k, _d, _c);
		}

		void SensorPinHole::acquireRaw(){

//			jafarImage_ptr_t i(Image::loadImage("/home/agonzale/workspace/opencv/img_pan01.jpeg",1));
//			jafarImage_ptr_t i(Image::loadImage("pelican.laas.fr:/data1/robots/dala/data/2009-02-27-someLoops/serie01/images/image.r.0000.tiff",1));
//			jafarImage_ptr_t i(Image::loadImage("/home/jsola/imageSample2.ppm",0));
			jafarImage_ptr_t i(Image::loadImage("/home/jeanmarie/imageSample2.ppm",0));

			rawimage_ptr_t imgRawPtr(new RawImage()) ;
			imgRawPtr->setJafarImage(i) ;

			this->rawPtr = imgRawPtr;

			// try to print the image
			// todo remove this test
//			cout << "creating viewer manager..." << endl;
//			jafar::qdisplay::ViewerManager viewerManager;
//			cout << "creating image viewer..." << endl;
//			jafar::qdisplay::ImageView imgView;
//			cout << "creating viewer..." << endl;
//			jafar::qdisplay::Viewer viewer;
//			cout << "after vewer" << endl;

		}


	}
}

