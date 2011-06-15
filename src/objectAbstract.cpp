/**
 * \file objectAbstract.cpp
 * \date 18/03/2010
 * \author jsola
 * \ingroup rtslam
 */

#include <iostream>

#include "rtslam/objectAbstract.hpp"
#include "rtslam/display.hpp"

namespace jafar {
	namespace rtslam {
		using namespace std;


		/*
		 * Operator << for class ObjectAbstract.
		 * It shows different information of the object.
		 */
		std::ostream& operator <<(std::ostream & s, ObjectAbstract const & obj) {
			s << obj.categoryName() << " " << obj.id() << ": ";
			if (obj.name().size() > 0)
				s << obj.name() << ", ";
			return s;
		}

		ObjectAbstract::ObjectAbstract() :
			id_(0), category(OBJECT) {
		}
		
		void ObjectAbstract::destroyDisplay()
		{
			for(std::vector<display::DisplayDataAbstract*>::iterator it = displayData.begin(); it != displayData.end(); ++it)
				{ delete *it; *it = NULL; }
		}
		
		ObjectAbstract::~ObjectAbstract() {
			destroyDisplay();
		}
		

	}
}
