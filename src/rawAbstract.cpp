/**
 * \file rawAbstract.cpp
 * \date 30/03/2010
 * \author jmcodol
 * \ingroup rtslam
 */


#include "rtslam/rawAbstract.hpp"

namespace jafar {
	namespace rtslam {
		using namespace std;

//		RawAbstract::RawAbstract() {
//		}

		RawAbstract::~RawAbstract() {
		}

		// PRINT UTILS
		/*
		 * Operator << for class rawAbstract.
		 * It shows some informations
		 */
		std::ostream& operator <<(std::ostream & s, RawAbstract const & rawA) {
			s << " I am a raw-data abstract" << endl;
			return s;
		}

	}
}
