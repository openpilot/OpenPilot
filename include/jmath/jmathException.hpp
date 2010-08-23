/* $Id$ */

#ifndef JMATH_JMATH_EXCEPTION_HPP
#define JMATH_JMATH_EXCEPTION_HPP

#include "kernel/jafarException.hpp"

namespace jafar {

  namespace jmath {

    /** Base class for all exceptions defined in the module
     * jmath.
     *
     * @ingroup jmath
     */
    class JmathException : public jafar::kernel::Exception {

    public:

      /** This enumeration defines exceptions id for the module
       * Jmath.
       */
      enum ExceptionId {
				LAPACK_ERROR, /**< Error reported by a lapack routine */
				SYSTEM_FULL  /**< System is already full of data */
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
      JmathException(ExceptionId id_, 
		     const std::string& message_, 
		     const std::string& file_, int line_) throw();

      virtual ~JmathException() throw();

      ExceptionId getExceptionId() const throw(); 

    protected:

      ExceptionId id;

      static std::string exceptionIdToString(ExceptionId id_) throw();
      
    }; // class JmathException

    /** This exception is thrown when a lapack routine fails.
     * Quote from lapack documentation:
     *
     * Error Handling and the Diagnostic Argument INFO 
     *
     *All documented routines have a diagnostic argument INFO that
     *indicates the success or failure of the computation, as follows:
     *
     * - INFO = 0: successful termination 
     * - INFO < 0: illegal value of one *or more arguments -- no
     * computation performed 
     * - INFO > 0: failure in the course of computation 
     *
     * All driver and auxiliary routines check that input arguments
     * such as N or LDA or option arguments of type character have
     * permitted values. If an illegal value of the ith argument is
     * detected, the routine sets INFO = -i, and then calls an
     * error-handling routine XERBLA.  The standard version of XERBLA
     * issues an error message and halts execution, so that no LAPACK
     * routine would ever return to the calling program with INFO <
     * 0. However, this might occur if a non-standard version of
     * XERBLA is used.
     *
     * \ingroup jmath
     */
    class LapackException : public JmathException {

    public:

      int info;

      LapackException(int info_,
		      const std::string& message_, 
		      const std::string& file_, int line_) throw() :
	JmathException(LAPACK_ERROR, message_, file_, line_),
	info(info_)
      {}

      virtual ~LapackException() throw() {}

    };

  } // namespace jmath
} // namespace jafar

#endif // JMATH_JMATH_EXCEPTION_HPP

