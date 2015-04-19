/*
 * pipeline.h
 *
 *  Created on: 15 déc. 2012
 *      Author: filnet
 */

#ifndef PIPELINE_H_
#define PIPELINE_H_

#include "gst_global.h"

#include <QString>

class GST_LIB_EXPORT Pipeline {
public:
    enum State {
        VoidPending, Null, Ready, Paused, Playing
    };
    Pipeline();
    virtual ~Pipeline();
};

#endif /* PIPELINE_H_ */
