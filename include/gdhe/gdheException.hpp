/* $Id$ */

#ifndef GDHE_GDHE_EXCEPTION_HPP
#define GDHE_GDHE_EXCEPTION_HPP

#include "kernel/jafarException.hpp"

namespace jafar {

  namespace gdhe {

    /** Base class for all exceptions defined in the module
     * gdhe.
     *
     * @ingroup gdhe
     */
    class GdheException : public ::jafar::kernel::Exception {

    public:

      /** This enumeration defines exceptions id for the module
       * gdhe.
       */
      enum ExceptionId {
				NOT_ADDED_TO_CLIENT
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
      GdheException(ExceptionId id_, 
                            const std::string& message_, 
                            const std::string& file_, int line_) throw();

      virtual ~GdheException() throw();

      ExceptionId getExceptionId() const throw(); 

    protected:

      ExceptionId id;

      static std::string exceptionIdToString(ExceptionId id_) throw();
      
    }; // class GdheException

  } // namespace gdhe
} // namespace jafar

#endif // GDHE_GDHE_EXCEPTION_HPP

