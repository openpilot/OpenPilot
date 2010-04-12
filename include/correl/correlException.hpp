/* $Id$ */

#ifndef CORREL_CORREL_EXCEPTION_HPP
#define CORREL_CORREL_EXCEPTION_HPP

#include "kernel/jafarException.hpp"

namespace jafar {

  namespace correl {

    /** Base class for all exceptions defined in the module
     * correl.
     *
     * @ingroup correl
     */
    class CorrelException : public ::jafar::kernel::Exception {

    public:

      /** This enumeration defines exceptions id for the module
       * correl.
       */
      enum ExceptionId {
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
      CorrelException(ExceptionId id_, 
                            const std::string& message_, 
                            const std::string& file_, int line_) throw();

      virtual ~CorrelException() throw();

      ExceptionId getExceptionId() const throw(); 

    protected:

      ExceptionId id;

      static std::string exceptionIdToString(ExceptionId id_) throw();
      
    }; // class CorrelException

  } // namespace correl
} // namespace jafar

#endif // CORREL_CORREL_EXCEPTION_HPP

