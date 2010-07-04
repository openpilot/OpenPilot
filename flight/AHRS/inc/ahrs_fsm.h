#ifndef AHRS_FSM_H
#define AHRS_FSM_H

#include "pios_opahrs_proto.h"

enum lfsm_state {
  LFSM_STATE_FAULTED = 0,	/* Must be zero so undefined transitions land here */
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

  LFSM_STATE_NUM_STATES,	/* Must be last */
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

  LFSM_EVENT_NUM_EVENTS,	/* Must be last */
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

extern void lfsm_init(void);
extern void lfsm_inject_event(enum lfsm_event event);

extern void lfsm_irq_callback(uint8_t crc_ok, uint8_t crc_val);

extern void lfsm_get_link_stats (struct lfsm_link_stats * stats);
extern enum lfsm_state lfsm_get_state (void);

extern void lfsm_set_link_proto_v0 (struct opahrs_msg_v0 * link_tx, struct opahrs_msg_v0 * link_rx);
extern void lfsm_user_set_rx_v0 (struct opahrs_msg_v0 * user_rx);
extern void lfsm_user_set_tx_v0 (struct opahrs_msg_v0 * user_tx);

extern void lfsm_set_link_proto_v1 (struct opahrs_msg_v1 * link_tx, struct opahrs_msg_v1 * link_rx);
extern void lfsm_user_set_rx_v1 (struct opahrs_msg_v1 * user_rx);
extern void lfsm_user_set_tx_v1 (struct opahrs_msg_v1 * user_tx);

extern void lfsm_user_done (void);

#endif	/* AHRS_FSM_H */
