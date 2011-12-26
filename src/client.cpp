

#include <iostream>
#include <unistd.h>

#include <boost/numeric/bindings/ublas/matrix.hpp>
#include <boost/numeric/bindings/ublas/vector.hpp>
#include <boost/numeric/bindings/ublas/symmetric.hpp>
#include <boost/numeric/bindings/lapack/driver/syev.hpp>
#include <boost/numeric/bindings/lapack/driver/gesdd.hpp>

#include "gdhe/client.hpp"
#include "jmath/angle.hpp"

namespace jafar {
namespace gdhe {

	Client::SendToServer Client::sendToServer;

	const ColorRGB colorRed(255,0,0);
	const ColorRGB colorGreen(0,255,0);
	const ColorRGB colorBlue(0,0,255);

	void Object::registerClient(Client *client_) { client = client_; if (label) label->registerClient(client_); }
	
	Object::~Object()
	{
		delete label; label = NULL;
		if (client) client->removeObject(this);
		#if ENABLE_CHILDREN
		for(std::list<Object*>::iterator it = children.begin(); it != children.end(); ++it)
		{
			destruction
			delete *it;
		}
		#endif
	}
	
	void Object::refresh()
	{
		if (!client) JFR_ERROR(GdheException, GdheException::NOT_ADDED_TO_CLIENT, "This object was not added to a client");
		std::ostringstream oss;
		if (attributesModified) oss << construct_string();
		if (poseModified) oss << move_string();
		if (attributesModified || poseModified) client->eval(oss.str());
		attributesModified = false;
		poseModified = false;
		if (label) label->refresh();
	}
	
	void Object::createLabel()
		{ label = new Label(); client->addSubObject(label, this, "-l", false); }
	void Object::updateLabelPose()
		{ if (label) label->setPose(x,y,z,yaw,pitch,roll); }
	void Object::setLabel(std::string text)
	{
		if (!label) createLabel();
		label->setText(text);
		updateLabelPose();
	}
	void Object::setLabelColor(ColorRGB &_color)
		{ if (!label) createLabel(); label->setColor(_color); }
	void Object::setLabelColor(unsigned char R_, unsigned char G_, unsigned char B_)
		{ if (!label) createLabel(); label->setColor(R_,G_,B_); }
	void Object::setLabelShift(double x_, double y_, double z_)
		{ if (!label) createLabel(); label->setShift(x_,y_,z_); }
	
	int Client::connect(bool wait)
	{
		int r;
		do { usleep(500000); }
		while ((r = ::get_connection(const_cast<char*>(host.c_str()))) != 1 && wait);
			
		init();
		return r;
	}
	

	void Client::init()
	{
		*this << "clearColor " << backgroundColor << ";" << sendToServer;
		// FIXME this should apparently be done before gdhe loading, no effect afterwards, but not very important since improvement of mouse gestures
		//eval("set xmin -10 ; set xmax 10 ; set ymin -10 ; set ymax 10 ; set zmin -2 ; set zmax 5");
		eval("set obsX 0.0 ; set obsY 0.0 ; set obsZ 0.0 ; set obsElev 45 ; set obsAzi 90 ; set obsDist 4.0 ; set envDefined 1");
		eval("source $env(JAFAR_DIR)/modules/gdhe/data/camera.tcl");
		eval("source $env(JAFAR_DIR)/modules/gdhe/data/setup.tcl");
	}
	
	void Client::addObject(Object *object, bool visible) 
	{
		object->registerClient(this);
		object->setId(idFactory.getId());
		if (object->label) addSubObject(object->label, object, "-l", visible);
		if (visible) object->refresh();
	}
	void Client::addSubObject(Object *object, Object *parent, std::string suffix, bool visible)
	{
		object->registerClient(this);
		object->setId(parent->id, suffix, false);
		if (visible) object->refresh();
	}
	
	void Client::removeObject(Object *object) 
	{
		eval(object->remove_string());
		if (object->ownId) idFactory.releaseId(object->id);
		object->registerClient(NULL);
	}

	void Client::setCameraTarget(double _x, double _y, double _z)
	{
		std::ostringstream oss;
		oss << "set obsX " << _x << ";set obsY " << _y << ";set obsZ " << _z << ";";
		eval(oss.str());
	}
	
	/**
	@param _yaw in degrees
	@param _pitch in degrees
	@param _dist 
	*/
	void Client::setCameraPos(double _yaw, double _pitch, double _dist)
	{
		std::ostringstream oss;
		oss << "set obsElev " << _pitch << ";set obsAzi " << _yaw << ";set obsDist " << _dist << ";";
		eval(oss.str());
	}

	template<class MatR>
	jblas::vec3 R2e(const MatR & R) {
		jblas::vec3 e;
		e(0) = atan2(R(1,0),R(0,0));
		e(1) = asin(-1.0*R(2,0));
		e(2) = atan2(R(2,1),R(2,2));
		for(int i = 0; i < 3; ++i) e(i) = jmath::radToDeg(e(i));
		return e;
	}

	void Ellipsoid::set(jblas::vec3 _x, jblas::sym_mat33 _xCov, double _scale)
	{
		namespace lapack = boost::numeric::bindings::lapack;
		jblas::vec lambda(3);
//		jblas::mat_column_major A(ublas::project(_xCov, ublas::range(0,3), ublas::range(0,3))); 
		jblas::mat_column_major A(_xCov); 
		// SYEV is buggy to get 3d orientation... using generic GESDD instead to do svd decomposition, without using the fact that xCov is symmetric
//		up_sym_adapt s_A(A);
//		int ierr = lapack::syev( 'V', s_A, lambda, lapack::optimal_workspace() );
		jblas::mat_column_major U(3, 3);
		jblas::mat_column_major VT(3, 3);
		int ierr = lapack::gesdd('A',A,lambda,U,VT);
		A = U;
		
		JFR_POSTCOND(ierr==0, "Ellipsoid:: error in lapack::syev() function, ierr=" << ierr);
		if (!ierr==0) {
			JFR_WARNING("Ellipsoid::Ellipsoid: error in lapack::syev() function, ierr=" << ierr);
		} else {
			jblas::vec3 e = R2e(A);
			dx = lambda(0) < 1e-6 ? 1e-3 : sqrt(lambda(0));
			dy = lambda(1) < 1e-6 ? 1e-3 : sqrt(lambda(1));
			dz = lambda(2) < 1e-6 ? 1e-3 : sqrt(lambda(2));
			dx *= _scale; dy *= _scale; dz *= _scale;
			setPose(_x(0), _x(1), _x(2), e(0), e(1), e(2));
			jblas::vec3 ls;
			if (dx >= dy && dx >= dz) { ls(0)=0 ; ls(1)=dy; ls(2)=dz; } else
			if (dy >= dx && dy >= dz) { ls(0)=dx; ls(1)=0 ; ls(2)=dz; } else
				                        { ls(0)=dx; ls(1)=dy; ls(2)=0 ; }
			ls = ls * 1.3;
//std::cout << "Ellipsoid::set x " << _x << " xCov " << _xCov << std::endl;
//std::cout << "Ellipsoid::set rotMatrix " << A << " euler " << e << " lambdas " << lambda << std::endl;
//std::cout << "Ellipsoid::set dxyz " << dx << "," << dy << "," << dz << " labelShift " << ls << std::endl;
			if (label) label->setShift(ls(0),ls(1),ls(2));
			touch();
		}
	}

	void Ellipsoid::setCompressed(jblas::vec3 _x, jblas::sym_mat33 _xCov, double _scale)
	{
		set(_x, _xCov, _scale);
		double &maxdim = (dx>dy ? (dx>dz ? dx : dz) : (dy>dz ? dy : dz));
		double &mindim = (dx<dy ? (dx<dz ? dx : dz) : (dy<dz ? dy : dz));
		maxdim = mindim;
	}


}}

