/**
 * \file appearanceAbstract.hpp
 *
 * \date 14/04/2010
 * \author jmcodol
 * \ingroup rtslam
 */

#ifndef APPEARANCEABSTRACT_H_
#define APPEARANCEABSTRACT_H_


namespace jafar {
	namespace rtslam {

		class AppearanceAbstract {
			public:
				AppearanceAbstract();
				virtual ~AppearanceAbstract();
				/**
				 * @return a new object that is a copy (clone) of this
				 */
				virtual AppearanceAbstract* clone() = 0;
		};
	}
}

#endif /* APPEARANCEABSTRACT_H_ */
