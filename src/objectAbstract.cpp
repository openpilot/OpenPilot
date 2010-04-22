/**
 * \file objectAbstract.cpp
 *
 * \date 18/03/2010
 * \author jsola@laas.fr
 *
 *
 *  ## Add a description here ##
 *
 * \ingroup rtslam
 */

#include <iostream>

#include "rtslam/objectAbstract.hpp"

namespace jafar {
	namespace rtslam {
		using namespace std;


		/*
		 * Operator << for class ObjectAbstract.
		 * It shows different information of the object.
		 */
		std::ostream& operator <<(std::ostream & s, jafar::rtslam::ObjectAbstract & obj) {
			s << obj.categoryName() << " " << obj.id() << ": ";
			if (obj.name().size() > 0)
				s << obj.name() << ", ";
			s << "of type " << obj.type();
			return s;
		}

		ObjectAbstract::ObjectAbstract() :
			id_(0), categoryName_("OBJECT") {
			cout << "Created object." << endl;
		}

	}
}
