#ifndef OP_EIGEN_ALIGN_FUNCTION_H
#define OP_EIGEN_ALIGN_FUNCTION_H

/*
 * The purpose of this macro is to force the alignment of the stack to a
 * 16-byte boundary on systems that don't provide it automatically.  At
 * this time, only GCC on Win32 is known to require it.  MSVC on Win32 may
 * as well.
 */

#ifdef __GNUC__
# ifdef WIN32
#  define FORCE_ALIGN_FUNC __attribute__((force_align_arg_pointer))
# else
#  define FORCE_ALIGN_FUNC
# endif
#else
# ifdef __MACOSX__
#  define FORCE_ALIGN_FUNC
# else
#  error Unknown compiler.  You may need to provide a definition of FORCE_ALIGN_FUNC
# endif
#endif

#endif // !defined OP_EIGEN_ALIGN_FUNCTION_H
