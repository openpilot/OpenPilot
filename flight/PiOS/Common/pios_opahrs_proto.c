/**
 * @addtogroup PIOS PIOS Core hardware abstraction layer
 * @{
 * @addtogroup PIOS_OPAHRS OPAHRS Functions
 * @{
 *
 * @file       pios_opahrs_proto.c
 * @author     The OpenPilot Team, http://www.openpilot.org Copyright (C) 2010.
 * @brief      PPM Input functions
 * @see        The GNU Public License (GPL) Version 3
 * 
 */

#include "pios_opahrs_proto.h"
#include <string.h>		/* memset */

void opahrs_msg_v0_init_rx(struct opahrs_msg_v0 *msg)
{
	/* Make sure we start with bad magic in the rx buffer */
	msg->head.magic = 0;
	msg->head.type = 0;
	msg->tail.magic = 0;
}

void opahrs_msg_v0_init_user_tx(struct opahrs_msg_v0 *msg, enum opahrs_msg_v0_tag tag)
{
	msg->head.magic = OPAHRS_MSG_MAGIC_HEAD;
	msg->head.type = OPAHRS_MSG_TYPE_USER_V0;

	msg->payload.user.t = tag;

	msg->tail.magic = OPAHRS_MSG_MAGIC_TAIL;
}

void opahrs_msg_v0_init_link_tx(struct opahrs_msg_v0 *msg, enum opahrs_msg_link_tag tag)
{
	msg->head.magic = OPAHRS_MSG_MAGIC_HEAD;
	msg->head.type = OPAHRS_MSG_TYPE_LINK;

	msg->payload.link.t = tag;

	msg->tail.magic = OPAHRS_MSG_MAGIC_TAIL;
}

void opahrs_msg_v1_init_rx(struct opahrs_msg_v1 *msg)
{
	/* Make sure we start with bad magic in the rx buffer */
	msg->head.magic = 0;
	msg->head.type = 0;
	msg->tail.magic = 0;
}

void opahrs_msg_v1_init_user_tx(struct opahrs_msg_v1 *msg, enum opahrs_msg_v1_tag tag)
{
	msg->head.magic = OPAHRS_MSG_MAGIC_HEAD;
	msg->head.type = OPAHRS_MSG_TYPE_USER_V1;

	msg->payload.user.t = tag;

	msg->tail.magic = OPAHRS_MSG_MAGIC_TAIL;
}

void opahrs_msg_v1_init_link_tx(struct opahrs_msg_v1 *msg, enum opahrs_msg_link_tag tag)
{
	msg->head.magic = OPAHRS_MSG_MAGIC_HEAD;
	msg->head.type = OPAHRS_MSG_TYPE_LINK;

	msg->payload.link.t = tag;

	msg->tail.magic = OPAHRS_MSG_MAGIC_TAIL;
}

/**
  * @}
  * @}
  */
