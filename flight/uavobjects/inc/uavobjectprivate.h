/**
 ******************************************************************************
 *
 * @file       uavobjectprivate.h
 * @author     The OpenPilot Team, http://www.openpilot.org Copyright (C) 2014.
 * @brief      Private declarations for uavobject manager and persistence handler.
 *             --
 * @see        The GNU Public License (GPL) Version 3
 *
 *****************************************************************************/
/*
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
 * or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License
 * for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write to the Free Software Foundation, Inc.,
 * 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
 */
#ifndef UAVOBJECTPRIVATE_H_
#define UAVOBJECTPRIVATE_H_


#if (defined(__MACH__) && defined(__APPLE__))
#include <mach-o/getsect.h>
#endif

// Constants

// Private types

// Macros
#define SET_BITS(var, shift, value, mask) var = (var & ~(mask << shift)) | (value << shift);
#define UAVO_PREFIX_SIZE 5
#define UAVO_PREFIX_STRING "UAV"

// Mach-o: dummy segment to calculate ASLR offset in sim_osx
#if (defined(__MACH__) && defined(__APPLE__))
static long _aslr_offset __attribute__((section("__DATA,_aslr")));
#endif

/* Table of UAVO handles */
#if (defined(__MACH__) && defined(__APPLE__))
/* Mach-o format */
static struct UAVOData * *__start__uavo_handles;
static struct UAVOData * *__stop__uavo_handles;
#else
/* ELF format: automagically defined at compile time */
extern struct UAVOData *__start__uavo_handles[] __attribute__((weak));
extern struct UAVOData *__stop__uavo_handles[] __attribute__((weak));
#endif

#define UAVO_LIST_ITERATE(_item) \
    for (struct UAVOData * *_uavo_slot = __start__uavo_handles; \
         _uavo_slot && _uavo_slot < __stop__uavo_handles; \
         _uavo_slot++) { \
        struct UAVOData *_item = *_uavo_slot; \
        if (_item == NULL) { continue; }

/**
 * List of event queues and the eventmask associated with the queue.
 */

/** opaque type for instances **/
typedef void *InstanceHandle;

struct ObjectEventEntry {
    struct ObjectEventEntry *next;
    xQueueHandle queue;
    UAVObjEventCallback     cb;
    uint8_t eventMask;
};

/*
   MetaInstance   == [UAVOBase [UAVObjMetadata]]
   SingleInstance == [UAVOBase [UAVOData [InstanceData]]]
   MultiInstance  == [UAVOBase [UAVOData [NumInstances [InstanceData0 [next]]]]
                                                  ____________________/
   \-->[InstanceData1 [next]]
                                                  _________...________/
   \-->[InstanceDataN [next]]
 */

/*
 * UAVO Base Type
 *   - All Types of UAVObjects are of this base type
 *   - The flags determine what type(s) this object
 */
struct UAVOBase {
    /* Let these objects be added to an event queue */
    struct ObjectEventEntry *next_event;

    /* Describe the type of object that follows this header */
    struct UAVOInfo {
        bool isMeta        : 1;
        bool isSingle      : 1;
        bool isSettings    : 1;
        bool isPriority    : 1;
    } flags;
} __attribute__((packed));

/* Augmented type for Meta UAVO */
struct UAVOMeta {
    struct UAVOBase base;
    UAVObjMetadata  instance0;
} __attribute__((packed));

/* Shared data structure for all data-carrying UAVObjects (UAVOSingle and UAVOMulti) */
struct UAVOData {
    struct UAVOBase base;
    uint32_t id;
    /*
     * Embed the Meta object as another complete UAVO
     * inside the payload for this UAVO.
     */
    struct UAVOMeta metaObj;
    uint16_t instance_size;
} __attribute__((packed, aligned(4)));

/* Augmented type for Single Instance Data UAVO */
struct UAVOSingle {
    struct UAVOData uavo;

    uint8_t instance0[];
    /*
     * Additional space will be malloc'd here to hold the
     * the data for this instance.
     */
} __attribute__((packed));

/* Part of a linked list of instances chained off of a multi instance UAVO. */
struct UAVOMultiInst {
    struct UAVOMultiInst *next;
    uint8_t instance[];
    /*
     * Additional space will be malloc'd here to hold the
     * the data for this instance.
     */
} __attribute__((packed));

/* Augmented type for Multi Instance Data UAVO */
struct UAVOMulti {
    struct UAVOData uavo;
    uint16_t num_instances;
    struct UAVOMultiInst instance0 __attribute__((aligned(4)));
    /*
     * Additional space will be malloc'd here to hold the
     * the data for instance 0.
     */
} __attribute__((packed));

/** all information about a metaobject are hardcoded constants **/
#define MetaNumBytes sizeof(UAVObjMetadata)
#define MetaBaseObjectPtr(obj)           ((struct UAVOData *)((obj) - offsetof(struct UAVOData, metaObj)))
#define MetaObjectPtr(obj)               ((struct UAVODataMeta *)&((obj)->metaObj))
#define MetaDataPtr(obj)                 ((UAVObjMetadata *)&((obj)->instance0))
#define LinkedMetaDataPtr(obj)           ((UAVObjMetadata *)&((obj)->metaObj.instance0))

/** all information about instances are dependant on object type **/
#define ObjSingleInstanceDataOffset(obj) ((void *)(&(((struct UAVOSingle *)obj)->instance0)))
#define InstanceDataOffset(inst)         ((void *)&(((struct UAVOMultiInst *)inst)->instance))
#define InstanceData(instance)           ((void *)instance)

// Private functions
int32_t sendEvent(struct UAVOBase *obj, uint16_t instId, UAVObjEventType event);
InstanceHandle getInstance(struct UAVOData *obj, uint16_t instId);

#endif /* UAVOBJECTPRIVATE_H_ */
