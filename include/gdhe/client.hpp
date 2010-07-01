#ifndef CLIENT_HPP
#define CLIENT_HPP

#include <cstdlib>
#include <cstdio>
#include <sstream>

#include <gdhe/GDHE.h>

#include "kernel/IdFactory.hpp"
#include "jmath/misc.hpp"
#include "jmath/jblas.hpp"

#include "gdhe/gdheException.hpp"

#define ENABLE_STREAM 0
#define ENABLE_CHILDREN 0

namespace jafar {
namespace gdhe {
	
	
	/** *************************************************************************/
	/// Client
	/** *************************************************************************/
	
	
	class Object;
	
	typedef kernel::IdFactoryList MyIdFactory;
	
	// FIXME maybe ask Matthieu to add const the parameters when included in c++ ? what's the best solution vs const_cast ?
	class Client
	{
		protected:
			MyIdFactory idFactory;
			std::string host;
		protected:
			void init();
		public:
			Client(): idFactory(), host("localhost") {}
			Client(std::string host_): idFactory(), host(host_) {}
			int connect(bool wait = true);
			int disconnect() { ::disconnect(); }
			int eval(std::string exp) 
			{
				int r;
				r = ::eval_expression(const_cast<char*>(exp.c_str()));
				JFR_VDEBUG("evaluated [" << r << "] '" << exp << "'");
				return r;
			}
			int launch_server() { system("gdhe&"); }
			int dump(std::string const& filename) { /* TODO */ }
			void redraw() { eval("redrawAllWindows"); }
			
			void addObject(Object *object);
			void removeObject(Object *object);
			
			#if ENABLE_STREAM
			/*
			Hum... not sure it is possible
			*/
			struct SendToServer {};
			template<typename T> std::ostream& operator<<(std::ostream& os, T element)
				{ os << element; return os; }
			template<> std::ostream& operator<<(std::ostream& os, SendToServer element)
				{ std::ostringstream oss; oss << os; eval(oss.str()); return os; }
			#endif
	};
	
	
	
	/** *************************************************************************/
	/// Colors Management
	/// Will be moved in image, and used in display.hpp
	/** *************************************************************************/
	
	struct ColorRGB
	{
		unsigned char R, G, B;
		ColorRGB(unsigned char R_, unsigned char G_, unsigned char B_): R(R_), G(G_), B(B_) {}
		ColorRGB(): R(0), G(0), B(0) {}
	};

	static const ColorRGB colorRed(255,0,0);
	static const ColorRGB colorGreen(0,255,0);
	static const ColorRGB colorBlue(0,0,255);

	/** *************************************************************************/
	/// Abstract objects definitions 
	/** *************************************************************************/
	
	/**
	When creating a new object type, you have to inherit from this object,
	optionnally add some attributes with their accessors, 
	call touch() every time an accessor modifies an attribute,
	and implement construction().
	
	*/
	class Object
	{
		protected:
			double x, y, z;
			double yaw, pitch, roll;
			ColorRGB color;
			bool poseModified, attributesModified;
			
		protected:
			MyIdFactory::storage_t id; ///< id of the object
			std::string ids;
			Client *client;
			void registerClient(Client *client_) { client = client_; }
			void setId(MyIdFactory::storage_t id_) { id = id_; ids = jmath::toStr(id); }
			void touch() { attributesModified = true; }
			virtual const std::string construct_string() const = 0;
			const std::string move_string() const
			{
				std::ostringstream oss;
				oss << "set pos(" << ids << ") {" << yaw << " " << pitch << " " << roll << " " << x << " " << y << " " << z << "}";
				return oss.str();
			}
			const std::string remove_string() const
			{
				std::ostringstream oss;
				oss << "unset robots(" << ids << ");unset pos(" << ids << ")";
				return oss.str();
			}
			#if ENABLE_CHILDREN
			std::list<Object*> children;
			#endif
		public:
			Object(): 
				x(0), y(0), z(0), yaw(0), pitch(0), roll(0), color(), poseModified(true), attributesModified(true), id(0), ids(""), client(NULL)
				{}
			Object(double _x, double _y, double _z, double _yaw, double _pitch, double _roll): 
				poseModified(true), attributesModified(true), id(0), ids(""), client(NULL) 
				{ setPose(_x,_y,_z,_yaw,_pitch,_roll); }
			Object(double _x, double _y, double _z, double _yaw, double _pitch, double _roll, unsigned char _R, unsigned char _G, unsigned char _B): 
				poseModified(true), attributesModified(true), id(0), ids(""), client(NULL) 
				{ setPose(_x,_y,_z,_yaw,_pitch,_roll); setColor(_R,_G,_B); }
			virtual ~Object();

			/**
			Immediatly remove the object from display
			*/
			void remove()
			{
				if (!client) JFR_ERROR(GdheException, GdheException::NOT_ADDED_TO_CLIENT, "This object was not added to a client");
				client->removeObject(this);
			}
			/**
			Immediatly hides the object in display
			*/
			void hide()
			{
				if (!client) JFR_ERROR(GdheException, GdheException::NOT_ADDED_TO_CLIENT, "This object was not added to a client");
				std::ostringstream oss; oss << "unset pos(" << ids << ")"; client->eval(oss.str());
				poseModified = true;
			}
			/**
			Immediatly show the object in display
			*/
			void show()
			{
				if (!client) JFR_ERROR(GdheException, GdheException::NOT_ADDED_TO_CLIENT, "This object was not added to a client");
				client->eval(move_string());
				poseModified = false;
			}
			/**
			Immediatly updates the object in display
			*/
			void refresh()
			{
				if (!client) JFR_ERROR(GdheException, GdheException::NOT_ADDED_TO_CLIENT, "This object was not added to a client");
				std::ostringstream oss;
				if (attributesModified) oss << construct_string() << " ; ";
				if (poseModified) oss << move_string();
				if (attributesModified || poseModified) client->eval(oss.str());
				attributesModified = false;
				poseModified = false;
			}
			
			void setPose(double _x, double _y, double _z, double _yaw, double _pitch, double _roll)
				{ x = _x; y = _y; z = _z; yaw = _yaw; pitch = _pitch; roll = _roll; poseModified = true; }
			void setPose(jblas::vec &pose) 
				{ setPose(pose(0),pose(1),pose(2),pose(3),pose(4),pose(5)); }
			void setPose(jblas::vec &position, jblas::vec &euler)
				{ setPose(position(0),position(1),position(2),euler(0),euler(1),euler(2)); }
			
			void setColor(ColorRGB &_color) 
				{ color = _color; touch(); }
			void setColor(unsigned char _R, unsigned char _G, unsigned char _B) 
				{ color.R = _R; color.G = _G; color.B = _B; touch(); }
			
			friend class Client;
	};
	
	
	

	/** *************************************************************************/
	/// Objects implementations
	/** *************************************************************************/

	
	class Robot: public Object
	{
		protected:
			std::string model;
		public:
			Robot(std::string model_, double x_, double y_, double z_, double yaw_, double pitch_, double roll_):
				model(model_)
			{
				setPose(x_, y_, z_, yaw_, pitch_, roll_);
			}
			
			virtual const std::string construct_string() const
			{
				std::ostringstream oss;
				oss << "set robots(" << ids << ") {" << model << "}";
				return oss.str();
			}
			
	};
	
	
	class Sphere: public Object
	{
		protected:
			double radius;
			int facets;
			
		public:
			Sphere(double radius_, double facets_ = 12):
				Object(), radius(radius_), facets(facets_)
				{ }
			Sphere(double x0_, double y0_, double z0_, double radius_, unsigned char R_, unsigned char G_, unsigned char B_, double facets_ = 12):
				Object(x0_, y0_, z0_, 0, 0, 0, R_, G_, B_), radius(radius_), facets(facets_)
				{ }

			void setRadius(double _radius) { radius = _radius; }
			void setFacets(double _facets) { facets = _facets; }

			const std::string construct_string() const
			{
				std::ostringstream oss;
				oss << "set robots(" << ids << ") {";
				oss << "color " << (int)color.R << " " << (int)color.G << " " << (int)color.B << " ; ";
				oss << "sphere " << 0 << " " << 0 << " " << 0 << " " << radius << " " << facets << "}";
				return oss.str();
			}

	};
	
	
	class Grid: public Object
	{
		protected:
			double size;
			double extent;
			
		public:
			Grid(double _extent, double _size):
				Object(), extent(_extent), size(_size)
				{ }

			const std::string construct_string() const
			{
				std::ostringstream oss;
				oss << "set robots(" << ids << ") {";
				oss << "color " << (int)color.R << " " << (int)color.G << " " << (int)color.B << " ; ";
				oss << "grille " << -extent << " " << -extent << " " << extent << " " << extent << " " << size << "}";
				return oss.str();
			}
	};
	
	
#if 0
	
	class Ellipsoid: public Object
	{
		private:
			void init(
		
		public:
			Ellipsoid(jblas::vec3 x, double r, int R, int G, int B)
			{
				
			}
			Ellipsoid(jblas::vec3 x, jblas::sym_mat33 P, )
			{
/*					Jafar::Slam::eval_expression("color 64 255 64")
					#Jafar::Slam::eval_expression("if {[info exists robots(#{featname})]} { unset robots(#{featname}) }")
					Jafar::Slam::eval_expression("set robots(#{featname}) {sphere 0 0 0 0.1}")
					Jafar::Slam::eval_expression("set pos(#{featname}) {#{arr[0]} #{arr[1]} #{arr[2]}}")
					#Jafar::Slam::eval_expression("if {[info exists robots(#{featname}_label)]} { unset robots(#{featname}_label) }")
					Jafar::Slam::eval_expression("set robots(#{featname}label) \"color 64 255 64 ; drawString 0 0 0.15 #{featname}\"")
					Jafar::Slam::eval_expression("set pos(#{featname}label) {#{arr[0]} #{arr[1]} #{arr[2]}}")
*/				
			}
			
		
	};
	
	class Label: public Object
	{
		
	};
	
	class Map: public Object
	{
		
	};

	
#endif
	
	
	
}}

#endif

