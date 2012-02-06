/* $Id$ */

#ifndef RTSLAM_RTSLAM_EXCEPTION_HPP
#define RTSLAM_RTSLAM_EXCEPTION_HPP

#include "kernel/jafarException.hpp"

namespace jafar {
	namespace rtslam {

		/** Base class for all exceptions defined in the module
		 * rtslam.
		 *
		 * @ingroup rtslam
		 */
		class RtslamException: public ::jafar::kernel::Exception {

			public:

				/** This enumeration defines exceptions id for the module
				 * rtslam.
				 */
				enum ExceptionId {
					UNKNOWN_FEATURE_TYPE,
					UNKNOWN_SENSOR_TYPE,
					UNKNOWN_DETECTION_METHOD,
					UNKNOWN_MATCHING_METHOD,
					UNKNOWN_TYPES_FOR_FACTORY,
					BUFFER_OVERFLOW,
					SIMU_ERROR,
					MISSING_DEPENDENCY,
					GENERIC_ERROR
				//        MY_ERROR /**< my error */
				};

				/** Constructor. You should not use this constructor directly,
				 * prefer macros jfrThrowEx or jfrCreateEx which fill for you
				 * parameters \c file_ and \c line_.
				 *
				 * @param id_ exception id
				 * @param message_ message used for debug
				 * @param file_ where the exception was thrown
				 * @param line_ where the exception was thrown
				 */
				RtslamException(ExceptionId id_, const std::string& message_,
				    const std::string& file_, int line_) throw ();

				virtual ~RtslamException() throw ();

				ExceptionId getExceptionId() const throw ();

			protected:

				ExceptionId id;

				static std::string exceptionIdToString(ExceptionId id_) throw ();

		}; // class RtslamException

	} // namespace rtslam
} // namespace jafar

#endif // RTSLAM_RTSLAM_EXCEPTION_HPP
