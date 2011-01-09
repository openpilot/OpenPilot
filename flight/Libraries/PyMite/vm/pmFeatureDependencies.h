/*
# This file is Copyright 2010 Dean Hall.
#
# This file is part of the PyMite VM.
# The PyMite VM is free software: you can redistribute it and/or modify
# it under the terms of the GNU GENERAL PUBLIC LICENSE Version 2.
#
# The PyMite VM is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
# A copy of the GNU GENERAL PUBLIC LICENSE Version 2
# is seen in the file COPYING in this directory.
*/


#ifndef __PM_EMPTY_PM_FEATURES_H__
#define __PM_EMPTY_PM_FEATURES_H__


/**
 * \file
 * \brief Platform feature descriptions and dependency checks
 *
 * This file explains the purpose of all of the HAVE_* features
 * and provides logical checks for features that depend on other features.
 *
 * A platform defines the features it wants in src/platform/<plat>/pmfeatures.py
 * A tool generates C HAVE_* definitions in pmfeatures.h from pmfeatures.py.
 *
 *
 * HAVE_PRINT
 * ----------
 *
 * When defined, bytecodes PRINT_ITEM and PRINT_NEWLINE are supported. Along
 * with these, helper routines in the object type are compiled in that allow
 * printing of the object.
 * REQUIRES stdio.h to have snprintf()
 *
 *
 * HAVE_GC
 * -------
 *
 * When defined, the code to perform mark-sweep garbage collection is included
 * in the build and automatic GC is enabled.  When undefined the allocator
 * will distribute memory until none is left, after which a memory exception
 * will occur.
 *
 *
 * HAVE_FLOAT
 * ----------
 *
 * When defined, the code to support floating point objects is included
 * in the build.
 * Issue #148 Create configurable float datatype
 *
 *
 * HAVE_DEL
 * --------
 *
 * When defined, the code to support the keyword del is included in the build.
 * This involves the bytecodes: DELETE_SUBSCR, DELETE_NAME, DELETE_ATTR,
 * DELETE_GLOBAL and DELETE_FAST.
 *
 *
 * HAVE_IMPORTS
 * ------------
 *
 * When defined, the code to support the IMPORT_FROM and IMPORT_STAR styles
 * is included in the build.
 *
 *
 * HAVE_DEFAULTARGS
 * ----------------
 *
 * When defined, the code to support default arguments to functions is included
 * in the build.
 * Issue #157 Support default args
 *
 *
 * HAVE_REPLICATION
 * ----------------
 *
 * When defined, the code to support sequence (list, tuple, string) replcation
 * is included in the build.
 * This feature is required by the builtin function __bi.map().
 * #160 Add support for string and tuple replication
 *
 *
 * HAVE_CLASSES
 * ------------
 *
 * When defined, the code to support classes, instances, methods, etc.
 * is included in the build.
 * Issue #202 Implement classes in the vm
 *
 *
 * HAVE_ASSERT
 * -----------
 *
 * When defined, the code to support the assert statement is included
 * in the build.
 *
 *
 * HAVE_GENERATORS
 * ---------------
 *
 * When defined, the code to support the yield keyword's use for
 * generator-iterators is included in the build.
 * Issue #207 Add support for the yield keyword
 *
 *
 * HAVE_BACKTICK
 * -------------
 *
 * When defined, the code to support the backtick operation (`x`) is included
 * in the build.
 * REQUIRES stdio.h to have snprintf()
 * Issue #244 Add support for the backtick operation (UNARY_CONVERT)
 *
 *
 * HAVE_STRING_FORMAT
 * ------------------
 *
 * When defined, the code to perform string formatting using the binary modulo
 * operator is included in the build.
 * REQUIRES stdio.h to have snprintf()
 * Issue #205 Add support for string format operation
 *
 *
 * HAVE_CLOSURES
 * -------------
 *
 * When defined, the code to support function closures is included in the
 * build.
 * Issue #256 Add support for closures
 *
 *
 * HAVE_BYTEARRAY
 * --------------
 *
 * When defined, the code to support the bytearray type is included in the
 * build.  NOTE: If this is defined, the bytearray class in src/lib/__bi.py
 * must also be uncommented.
 * Issue #289 Create bytearray datatype
 *
 *
 * HAVE_DEBUG_INFO
 * ---------------
 *
 * When defined, the code to support debug information in exception reports
 * is included in the build.
 * Issue #103 Add debug info to exception reports
 */

/* Check for dependencies */

#if defined(HAVE_ASSERT) && !defined(HAVE_CLASSES)
#error HAVE_ASSERT requires HAVE_CLASSES
#endif


#if defined(HAVE_GENERATORS) && !defined(HAVE_CLASSES)
#error HAVE_GENERATORS requires HAVE_CLASSES
#endif


#if defined(HAVE_CLOSURES) && !defined(HAVE_DEFAULTARGS)
#error HAVE_CLOSURES requires HAVE_DEFAULTARGS
#endif


#if defined(HAVE_BYTEARRAY) && !defined(HAVE_CLASSES)
#error HAVE_BYTEARRAY requires HAVE_CLASSES
#endif

#endif /* __PM_EMPTY_PM_FEATURES_H__ */
