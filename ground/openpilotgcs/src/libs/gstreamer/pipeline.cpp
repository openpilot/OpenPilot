/*
 * pipeline.cpp
 *
 *  Created on: 15 déc. 2012
 *      Author: filnet
 */

#include "pipeline.h"

#include <gst/gst.h>
#include <gst/interfaces/xoverlay.h>

Pipeline::Pipeline()
{
    // initialize gstreamer
    gst::init(NULL, NULL);
}

Pipeline::~Pipeline()
{
}

