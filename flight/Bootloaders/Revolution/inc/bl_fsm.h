/**
 ******************************************************************************
 *
 * @file       ahrs_fsm.h
 * @author     The OpenPilot Team, http://www.openpilot.org Copyright (C) 2010.
 * @brief     
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

#ifndef BL_FSM_H
#define BL_FSM_H

#include "pios_opahrs_proto.h"

enum lfsm_state {
	LFSM_STATE_FAULTED = 0, /* Must be zero so undefined transitions land here */
	LFSM_STATE_STOPPED,
	LFSM_STATE_STOPPING,
	LFSM_STATE_INACTIVE,
	LFSM_STATE_USER_BUSY,
	LFSM_STATE_USER_BUSY_RX_PENDING,
	LFSM_STATE_USER_BUSY_TX_PENDING,
	LFSM_STATE_USER_BUSY_RXTX_PENDING,
	LFSM_STATE_USER_RX_PENDING,
	LFSM_STATE_USER_TX_PENDING,
	LFSM_STATE_USER_RXTX_PENDING,
	LFSM_STATE_USER_RX_ACTIVE,
	LFSM_STATE_USER_TX_ACTIVE,
	LFSM_STATE_USER_RXTX_ACTIVE,

	LFSM_STATE_NUM_STATES
/* Must be last */
};

enum lfsm_event {
	LFSM_EVENT_INIT_LINK,
	LFSM_EVENT_STOP,
	LFSM_EVENT_USER_SET_RX,
	LFSM_EVENT_USER_SET_TX,
	LFSM_EVENT_USER_DONE,
	LFSM_EVENT_RX_LINK,
	LFSM_EVENT_RX_USER,
	LFSM_EVENT_RX_UNKNOWN,

	LFSM_EVENT_NUM_EVENTS
/* Must be last */
};

struct lfsm_link_stats {
	uint32_t rx_badcrc;
	uint32_t rx_badmagic_head;
	uint32_t rx_badmagic_tail;
	uint32_t rx_link;
	uint32_t rx_user;
	uint32_t tx_user;
	uint32_t rx_badtype;
	uint32_t rx_badver;
};

extern void lfsm_attach(uint32_t spi_id);
extern void lfsm_init(void);
extern void lfsm_inject_event(enum lfsm_event event);

extern void lfsm_irq_callback(uint8_t crc_ok, uint8_t crc_val);

extern void lfsm_get_link_stats(struct lfsm_link_stats * stats);
extern enum lfsm_state lfsm_get_state(void);

extern void lfsm_set_link_proto_v0(struct opahrs_msg_v0 * link_tx,
		struct opahrs_msg_v0 * link_rx);
extern void lfsm_user_set_rx_v0(struct opahrs_msg_v0 * user_rx);
extern void lfsm_user_set_tx_v0(struct opahrs_msg_v0 * user_tx);

extern void lfsm_set_link_proto_v1(struct opahrs_msg_v1 * link_tx,
		struct opahrs_msg_v1 * link_rx);
extern void lfsm_user_set_rx_v1(struct opahrs_msg_v1 * user_rx);
extern void lfsm_user_set_tx_v1(struct opahrs_msg_v1 * user_tx);

extern void lfsm_user_done(void);

#endif	/* BL_FSM_H */
