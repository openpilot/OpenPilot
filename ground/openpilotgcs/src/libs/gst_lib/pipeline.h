/*
 * pipeline.h
 *
 *  Created on: 15 déc. 2012
 *      Author: filnet
 */

#ifndef PIPELINE_H_
#define PIPELINE_H_

typedef struct _GstElement GstElement;

//class BusSyncHandler;

class Pipeline {
public:
    enum State {
        VoidPending, Null, Ready, Paused, Playing
    };
    Pipeline();
    virtual ~Pipeline();
private:
    //GstElement * pipeline;
    //BusSyncHandler * handler;
};

#endif /* PIPELINE_H_ */
