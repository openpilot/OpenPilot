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
	
	typedef kernel::IdFactorySet MyIdFactory;
	
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
			void setHost(std::string host_) { host = host_; }
			int connect(bool wait = true);
			int disconnect() { return ::disconnect(); }
			int eval(std::string exp) 
			{
				int r;
				r = ::eval_expression(const_cast<char*>(exp.c_str()));
				//JFR_DEBUG("evaluated [" << r << "] '" << exp << "'");
				return r;
			}
			int launch_server() { return system("gdhe&"); }
			int dump(std::string const& filename) { return 0;/* TODO */ }
			void redraw() { eval("redrawAllWindows"); }
			
			void addObject(Object *object, bool visible = true);
			void addSubObject(Object *object, Object *parent, std::string suffix, bool visible = true);
			void removeObject(Object *object);
			void setCameraTarget(double _x, double _y, double _z);
			void setCameraPos(double _yaw, double _pitch, double _dist);
			
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

	extern const ColorRGB colorRed;
	extern const ColorRGB colorGreen;
	extern const ColorRGB colorBlue;

	/** *************************************************************************/
	/// Abstract objects definitions 
	/** *************************************************************************/
	
	class Label;
	
	/**
	When creating a new object type, you have to inherit from this object,
	optionnally add some attributes with their accessors, 
	call touch() every time an accessor modifies an attribute,
	and implement construction().
	
	If the object wants to support a label, it has to set its relative position
	everytime its size is changed and call touch()
	
	*/
	class Object
	{
		protected:
			double x, y, z;
			double yaw, pitch, roll;
			ColorRGB color;
			bool poseModified, attributesModified;
			
			Label *label;
			void createLabel();
			void updateLabelPose();
			
		protected:
			bool ownId;
			MyIdFactory::storage_t id; ///< id of the object
			std::string ids;
			Client *client;
			void registerClient(Client *client_);
			void setId(MyIdFactory::storage_t id_, std::string suffix = "", bool ownId_ = true) 
				{ ownId = ownId_; id = id_; ids = jmath::toStr(id)+suffix; }
			void touch() { attributesModified = true; }
			#if ENABLE_CHILDREN
			std::list<Object*> children;
			#endif
		public:
			Object(): 
				x(0), y(0), z(0), yaw(0), pitch(0), roll(0), color(), poseModified(true), attributesModified(true), label(NULL), ownId(false), id(0), ids(""), client(NULL)
				{ }
			Object(double _x, double _y, double _z, double _yaw, double _pitch, double _roll): 
				poseModified(true), attributesModified(true), label(NULL), ownId(false), id(0), ids(""), client(NULL) 
				{ setPose(_x,_y,_z,_yaw,_pitch,_roll); }
			Object(double _x, double _y, double _z, double _yaw, double _pitch, double _roll, unsigned char _R, unsigned char _G, unsigned char _B): 
				poseModified(true), attributesModified(true), label(NULL), ownId(false), id(0), ids(""), client(NULL) 
				{ setPose(_x,_y,_z,_yaw,_pitch,_roll); setColor(_R,_G,_B); }
			virtual ~Object();

			virtual const std::string construct_string() const = 0;
			virtual const std::string move_string() const
			{
				std::ostringstream oss;
				oss << "set pos(" << ids << ") {" << yaw << " " << pitch << " " << roll << " " << x << " " << y << " " << z << "}";
				return oss.str();
			}
			virtual const std::string remove_string() const
			{
				std::ostringstream oss;
				oss << "unset robots(" << ids << ");unset pos(" << ids << ")";
				return oss.str();
			}
			
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
			void refresh();
			
			void setPose(double _x, double _y, double _z, double _yaw, double _pitch, double _roll)
				{ x = _x; y = _y; z = _z; yaw = _yaw; pitch = _pitch; roll = _roll; poseModified = true; updateLabelPose(); }
			void setPose(jblas::vec &pose) 
				{ setPose(pose(0),pose(1),pose(2),pose(3),pose(4),pose(5)); }
			void setPose(jblas::vec &position, jblas::vec &euler)
				{ setPose(position(0),position(1),position(2),euler(0),euler(1),euler(2)); }
			
			void setColor(ColorRGB &_color) 
				{ color = _color; touch(); }
			void setColor(unsigned char _R, unsigned char _G, unsigned char _B) 
				{ color.R = _R; color.G = _G; color.B = _B; touch(); }
			
			void setLabel(std::string text);
			void setLabelColor(ColorRGB &_color);
			void setLabelColor(unsigned char _R, unsigned char _G, unsigned char _B);
			void setLabelShift(double x_, double y_, double z_);
			
			friend class Client;
	};
	
	
	

	/** *************************************************************************/
	/// Objects implementations
	/** *************************************************************************/

	
	class Label: public Object
	{
		protected:
			std::string text;
			double shiftX, shiftY, shiftZ;
		public:
			Label(): text("") {}
			Label(std::string text_): text(text_) {}
			void setText(std::string text_) { text = text_; touch(); }
			void setShift(double x_, double y_, double z_) { shiftX = x_; shiftY = y_; shiftZ = z_; touch(); }
			virtual const std::string construct_string() const
			{
				std::ostringstream oss;
				oss << "set robots(" << ids << ") {";
				oss << "color " << (int)color.R << " " << (int)color.G << " " << (int)color.B << " ; ";
				oss << "drawString " << shiftX << " " << shiftY << " " << shiftZ << " \"" << text << "\"}";
				return oss.str();
			}
	};
	
	class Robot: public Object
	{
		protected:
			std::string model;
		public:
			Robot(std::string model_):
				model(model_) {}
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

			void setRadius(double _radius) { radius = _radius; touch(); if (label) label->setShift(0,0,_radius*1.3); }
			void setFacets(double _facets) { facets = _facets; touch(); }

			const std::string construct_string() const
			{
				std::ostringstream oss;
				oss << "set robots(" << ids << ") {";
				oss << "color " << (int)color.R << " " << (int)color.G << " " << (int)color.B << " ; ";
				oss << "sphere " << 0 << " " << 0 << " " << 0 << " " << radius << " " << facets << "}";
				return oss.str();
			}
	};
	

	class Ellipsoid: public Object
	{
		protected:
			double dx, dy, dz;
			int facets;
			
		public:
			Ellipsoid(double facets_ = 12):
				Object(), dx(0), dy(0), dz(0), facets(facets_)
				{ }
			Ellipsoid(double _dx, double _dy, double _dz, double facets_ = 12):
				Object(), dx(_dx), dy(_dy), dz(_dz), facets(facets_)
				{ }
			Ellipsoid(double x0_, double y0_, double z0_, double _dx, double _dy, double _dz, unsigned char R_, unsigned char G_, unsigned char B_, double facets_ = 12):
				Object(x0_, y0_, z0_, 0, 0, 0, R_, G_, B_), dx(_dx), dy(_dy), dz(_dz), facets(facets_)
				{ }
			Ellipsoid(jblas::vec3 _x, jblas::sym_mat33 _xCov, double _scale = 1)
				{ set(_x, _xCov, _scale); }
			void set(jblas::vec3 _x, jblas::sym_mat33 _xCov, double _scale = 1);
			void setCompressed(jblas::vec3 _x, jblas::sym_mat33 _xCov, double _scale = 1);

			void setRads(double _dx, double _dy, double _dz) { dx = _dx; dy = _dy; dz = _dz; touch(); if (label) label->setShift(0,0,_dz*1.3); }
			void setFacets(double _facets) { facets = _facets; touch(); }

			const std::string construct_string() const
			{
				std::ostringstream oss;
				oss << "set robots(" << ids << ") {";
				oss << "color " << (int)color.R << " " << (int)color.G << " " << (int)color.B << " ; ";
				oss << "ellipsoid " << 0 << " " << 0 << " " << 0 << " " << dx << " " << dy << " " << dz << " " << facets << "}";
				return oss.str();
			}
	};

	
	class Grid: public Object
	{
		protected:
			double extent;
			double size;
			
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
	
	struct Point3D
	{
		double x, y, z;
		Point3D(double x_, double y_, double z_): x(x_), y(y_), z(z_) {}
	};
	
	class Polyline: public Object
	{
		protected:
			std::vector<Point3D> line;
			
		public:
			Polyline(): Object() { }
			Polyline(double x0, double y0, double z0, double x1, double y1, double z1):
				Object()
			{
				line.push_back(Point3D(x0,y0,z0));
				line.push_back(Point3D(x1,y1,z1));
			}

			void addPoint(const Point3D &point) { line.push_back(point); touch(); }
			void addPoint(jblas::vec point) { addPoint(Point3D(point(0),point(1),point(2))); }
			void addPoint(double x, double y, double z) { addPoint(Point3D(x,y,z)); }
			void clear() { line.clear(); touch(); }
			Point3D const& back() { return line.back(); }
			int size() { return line.size(); }

			const std::string construct_string() const
			{
				std::ostringstream oss;
				oss << "set robots(" << ids << ") {";
				oss << "color " << (int)color.R << " " << (int)color.G << " " << (int)color.B << " ; ";
				oss << "polyline " << line.size();
				for(std::vector<Point3D>::const_iterator it = line.begin(); it != line.end(); ++it)
					oss << " " << it->x << " " << it->y << " " << it->z;
				oss << "}";
				return oss.str();
			}
	};
	
	// TODO need to write a real object type Trajectory that optimizes refreshing
	//typedef Polyline Trajectory;
	
	// TODO maybe compress a little bit, we'll lose detail but it's really big à 60Hz...
	class Trajectory: public Object
	{
		protected:
			std::vector<Polyline*> traj;
			int poly_size;
			Polyline *last_poly;
		public:
			Trajectory(): Object(), poly_size(200), last_poly(NULL) {}
			~Trajectory()
			{
				for(std::vector<Polyline*>::iterator it = traj.begin(); it != traj.end(); ++it)
					delete *it;
			}
			void addPoint(const Point3D &point)
			{
				if (last_poly == NULL || last_poly->size() >= poly_size)
				{
					Polyline *prev_last_poly = last_poly;
					traj.push_back(new Polyline());
					last_poly = traj.back();
					last_poly->setColor(color);
					std::ostringstream oss; oss << "-" << traj.size();
					client->addSubObject(last_poly, this, oss.str(), false);
					if (prev_last_poly)
						last_poly->addPoint(prev_last_poly->back());
				}
				last_poly->addPoint(point);
				touch();
			}
			void addPoint(jblas::vec point) { addPoint(Point3D(point(0),point(1),point(2))); }
			void addPoint(double x, double y, double z) { addPoint(Point3D(x,y,z)); }
			void clear() { traj.clear(); touch(); }
		
			const std::string move_string() const
				{ if (last_poly) return last_poly->move_string(); else return ""; }
			const std::string remove_string() const
				{ if (last_poly) return last_poly->remove_string(); else return ""; }
			const std::string construct_string() const
				{ if (last_poly) return last_poly->construct_string(); else return ""; }
	};
	
	
	class Frame: public Object
	{
		protected:
			double length;
		public:
			Frame(): Object() { }
			Frame(double length_):
				Object(), length(length_) {}
			
			const std::string construct_string() const
			{
				std::ostringstream oss;
				oss << "set robots(" << ids << ") {";
				oss << "color " << (int)color.R << " " << (int)color.G << " " << (int)color.B << " ; ";
				oss << "repere " << length << "}";
				return oss.str();
			}
	};
	
	
}}

#endif

