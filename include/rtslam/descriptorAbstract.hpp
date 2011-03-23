/**
 * \file descriptorAbstract.hpp
 *
 * \date 20/04/2010
 * \author jmcodol
 * \ingroup rtslam
 */

#ifndef DESCRIPTORABSTRACT_H_
#define DESCRIPTORABSTRACT_H_

#include <boost/smart_ptr.hpp>

#include "jmath/jblas.hpp"
#include "image/oimstream.hpp"

#include "rtslam/rtSlam.hpp"
//#include "rtslam/appearanceAbstract.hpp"
#include "rtslam/appearanceImage.hpp"

namespace jafar {

	namespace rtslam {

		class DescriptorAbstract {
			
			
			public:

				DescriptorAbstract();
				virtual ~DescriptorAbstract();

				/**
				 * Take an actual observation and use its appearance to improve the descriptor
				 */
				virtual bool addObservation(const observation_ptr_t & obsPtr) = 0;
				/**
				 * Fills in the predicted appearance in an observation
				 */
				virtual bool predictAppearance(const observation_ptr_t & obsPtr) = 0;
				/**
				 * returns true if a valid prediction can be made with this descriptor for this observation
				 * if it returns false, it doesn't mean that no prediction can be made, but that
				 * we can't be sure it will be good
				 */
				virtual bool isPredictionValid(const observation_ptr_t & obsPtr) = 0;
				
				virtual std::string categoryName() const {
					return "DESCRIPTOR";
				}

				virtual void desc_text(std::ostream& os) const {}
				virtual void desc_image(image::oimstream& os) const {}

		};

		
		std::ostream& operator <<(std::ostream & s, DescriptorAbstract const & desc);
		image::oimstream& operator <<(image::oimstream & s, DescriptorAbstract const & desc);
		
		
		class DescriptorFactoryAbstract
		{
			public:
				virtual DescriptorAbstract *createDescriptor() = 0;
		};
		
	}
}

#endif /* DESCRIPTORABSTRACT_H_ */
