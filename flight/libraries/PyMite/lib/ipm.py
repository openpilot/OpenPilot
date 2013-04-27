# This file is Copyright 2007, 2009, 2010 Dean Hall.
#
# This file is part of the Python-on-a-Chip program.
# Python-on-a-Chip is free software: you can redistribute it and/or modify
# it under the terms of the GNU LESSER GENERAL PUBLIC LICENSE Version 2.1.
#
# Python-on-a-Chip is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
# A copy of the GNU LESSER GENERAL PUBLIC LICENSE Version 2.1
# is seen in the file COPYING in this directory.

## @file
#  @copybrief ipm_target

## @package ipm_target
#  @brief Provides PyMite's interactive interface for the target.


##
# Receives an image over the platform's standard connection.
# Returns the image in a string object
#
def _getImg():
    """__NATIVE__
    PmReturn_t retval;
    uint8_t imgType;
    uint16_t imgSize;
    uint8_t *pchunk;
    pPmCodeImgObj_t pimg;
    uint16_t i;
    uint8_t b;

    /* Get the image type */
    retval = plat_getByte(&imgType);
    PM_RETURN_IF_ERROR(retval);

    /* Quit if a code image type was not received */
    if (imgType != OBJ_TYPE_CIM)
    {
        PM_RAISE(retval, PM_RET_EX_STOP);
        return retval;
    }

    /* Get the image size (little endien) */
    retval = plat_getByte(&b);
    PM_RETURN_IF_ERROR(retval);
    imgSize = b;
    retval = plat_getByte(&b);
    PM_RETURN_IF_ERROR(retval);
    imgSize |= (b << 8);

    /* Get space for CodeImgObj */
    retval = heap_getChunk(sizeof(PmCodeImgObj_t) + imgSize, &pchunk);
    PM_RETURN_IF_ERROR(retval);
    pimg = (pPmCodeImgObj_t)pchunk;
    OBJ_SET_TYPE(pimg, OBJ_TYPE_CIO);

    /* Start the image with the bytes that have already been received */
    i = 0;
    pimg->val[i++] = imgType;
    pimg->val[i++] = imgSize & 0xFF;
    pimg->val[i++] = (imgSize >> 8) & 0xFF;

    /* Get the remaining bytes in the image */
    for(; i < imgSize; i++)
    {
        retval = plat_getByte(&b);
        PM_RETURN_IF_ERROR(retval);

        pimg->val[i] = b;
    }

    /* Return the image as a code image object on the stack */
    NATIVE_SET_TOS((pPmObj_t)pimg);
    return retval;
    """
    pass


def x04():
    """__NATIVE__
    NATIVE_SET_TOS(PM_NONE);
    return plat_putByte(0x04);
    """
    pass


##
# Runs the target device-side interactive session.
#
def ipm(g={}):
    while 1:
        # Wait for a code image, make a code object from it
        # and evaluate the code object.
        # #180: One-liner turned into 3 so that objects get bound to roots
        s = _getImg()
        co = Co(s)
        rv = eval(co, g)
        x04()

    # Execution should never reach here
    # The while loop (above) probably caught a StopIteration, accidentally
    assert False

# :mode=c:
