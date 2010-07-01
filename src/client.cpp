

#include <iostream>
#include <unistd.h>

#include "gdhe/client.hpp"

namespace jafar {
namespace gdhe {


//	ColorRGB colorRed(255,0,0);
//	ColorRGB colorGreen(0,255,0);
//	ColorRGB colorBlue(0,0,255);

	
	Object::~Object()
	{
		if (client) client->removeObject(this);
		#if ENABLE_CHILDREN
		for(std::list<Object*>::iterator it = children.begin(); it != children.end(); ++it)
		{
			destruction
			delete *it;
		}
		#endif
	}
	
	
	int Client::connect(bool wait)
	{
		int r;
		do { usleep(200000); }
		while ((r = ::get_connection(const_cast<char*>(host.c_str()))) != 1 && wait);
			
		init();
		return r;
	}
	

	void Client::init()
	{
		eval("clearColor 224 224 256");
		eval("set xmin -500 ; set xmax 500 ; set ymin -500 ; set ymax 500 ; set zmin -20 ; set zmax 200 ; set distmin 0 ; set distmax 500");
		eval("set obsX 0.0 ; set obsY 0.0 ; set obsZ 0.0 ; set obsElev 45 ; set obsAzi 90 ; set obsDist 10.0 ; set envDefined 1");
	}
	
	void Client::addObject(Object *object) 
	{
		object->registerClient(this);
		object->setId(idFactory.getId());
		object->refresh();
	}
	void Client::removeObject(Object *object) 
	{
		eval(object->remove_string());
		idFactory.releaseId(object->id);
		object->registerClient(NULL);
	}

}}

