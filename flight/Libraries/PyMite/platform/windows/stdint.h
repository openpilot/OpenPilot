/*
 * Small stdint.h replacement to be used with the MS Visual C++ environment
 */

#ifndef __stdint_h__
#define __stdint_h__

#if defined(_MSC_VER)
    // Visual Studio does not include stdint.h, so we do it inline here
    typedef char int8_t;
    typedef short int16_t;
    typedef int int32_t;
    typedef unsigned char uint8_t;
    typedef unsigned short uint16_t;
    typedef unsigned int uint32_t;

    // change the default VS _DEBUG into PyMite __DEBUG__
    #if defined(_DEBUG)
        #define __DEBUG__ 1
    #endif
#else
    //#include <stdint.h>
#endif


#endif /* __stdint_h__ */