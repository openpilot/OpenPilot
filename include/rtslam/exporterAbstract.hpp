/**
 * \file exporterAbstract.hpp
 *
 * Header file for exporting state
 *
 * \date 16/03/2011
 * \author croussil
 *
 * \ingroup rtslam
 */

#ifndef EXPORTER_ABSTRACT_HPP
#define EXPORTER_ABSTRACT_HPP

#include "rtslam/robotAbstract.hpp"

namespace jafar {
namespace rtslam {

	
	class ExporterAbstract
	{
		protected:
			robot_ptr_t robPtr;
		public:
			ExporterAbstract(robot_ptr_t robPtr): robPtr(robPtr) {}
			virtual void exportCurrentState() = 0;
			virtual void stop() {}
	};
	
	
	
	
	class ExporterPoster: public ExporterAbstract
	{
		public:
			ExporterPoster(robot_ptr_t robPtr): ExporterAbstract(robPtr)
			{
			
			}
			virtual void exportCurrentState()
			{
				
				
			}
	};
	
	
}}




#endif // EXPORTERABSTRACT_HPP
