/**
 * \file rawAbstract.hpp
 * File defining the abstract raw-data class
 * \author jmcodol
 * \ingroup rtslam
 */

#ifndef RAWABSTRACT_HPP_
#define RAWABSTRACT_HPP_

/* --------------------------------------------------------------------- */
/* --- INCLUDE --------------------------------------------------------- */
/* --------------------------------------------------------------------- */

#include <iostream>

#include "image/roi.hpp"

#include "rtslam/objectAbstract.hpp"
#include "rtslam/measurement.hpp"
#include <opencv/cv.h> // opencv 

namespace jafar {
	namespace rtslam {

		class RawAbstract: public ObjectAbstract {

				/*
				 * Operator << for class rawAbstract.
				 * It shows some informations
				 */
				friend std::ostream& operator <<(std::ostream & s, jafar::rtslam::RawAbstract const & rawA);

			public:

				double timestamp;
				double arrival;
				
				virtual ~RawAbstract();

				virtual std::string categoryName() const {
					return "RAW";
				}

		};
	}
}
#endif /* RAWABSTRACT_HPP_ */



