#include <stdint.h>		/* uint*_t */
#include <stddef.h>		/* NULL */

#include "bl_fsm.h"

#include "pios_opahrs_proto.h"

#include "pios.h"

struct lfsm_context {
	enum lfsm_state curr_state;
	enum opahrs_msg_link_state link_state;
	enum opahrs_msg_type user_payload_type;
	uint32_t user_payload_len;

	uint32_t errors;

	uint8_t * rx;
	uint8_t * tx;

	uint8_t * link_rx;
	uint8_t * link_tx;

	uint8_t * user_rx;
	uint8_t * user_tx;

	struct lfsm_link_stats stats;
};

static struct lfsm_context context = { 0 };

static void lfsm_update_link_tx(struct lfsm_context * context);
static void lfsm_init_rx(struct lfsm_context * context);

static uint32_t PIOS_SPI_OP;
void lfsm_attach(uint32_t spi_id) {
	PIOS_SPI_OP = spi_id;
}

/*
 *
 * Link Finite State Machine
 *
 */

struct lfsm_transition {
	void (*entry_fn)(struct lfsm_context * context);
	enum lfsm_state next_state[LFSM_EVENT_NUM_EVENTS];
};

static void go_faulted(struct lfsm_context * context);
static void go_stopped(struct lfsm_context * context);
static void go_stopping(struct lfsm_context * context);
static void go_inactive(struct lfsm_context * context);
static void go_user_busy(struct lfsm_context * context);
static void go_user_busy_rx_pending(struct lfsm_context * context);
static void go_user_busy_tx_pending(struct lfsm_context * context);
static void go_user_busy_rxtx_pending(struct lfsm_context * context);
static void go_user_rx_pending(struct lfsm_context * context);
static void go_user_tx_pending(struct lfsm_context * context);
static void go_user_rxtx_pending(struct lfsm_context * context);
static void go_user_rx_active(struct lfsm_context * context);
static void go_user_tx_active(struct lfsm_context * context);
static void go_user_rxtx_active(struct lfsm_context * context);

const static struct lfsm_transition lfsm_transitions[LFSM_STATE_NUM_STATES] = {
		[LFSM_STATE_FAULTED] = {
			.entry_fn = go_faulted,
		}, [LFSM_STATE_STOPPED] = {
			.entry_fn = go_stopped,
			.next_state = {
				[LFSM_EVENT_INIT_LINK] = LFSM_STATE_INACTIVE,
				[LFSM_EVENT_RX_UNKNOWN] = LFSM_STATE_STOPPED,
			},
		}, [LFSM_STATE_STOPPING] = {
			.entry_fn = go_stopping,
			.next_state = {
				[LFSM_EVENT_RX_LINK] = LFSM_STATE_STOPPED,
				[LFSM_EVENT_RX_USER] = LFSM_STATE_STOPPED,
				[LFSM_EVENT_RX_UNKNOWN] = LFSM_STATE_STOPPED,
			},
		}, [LFSM_STATE_INACTIVE] = {
			.entry_fn = go_inactive,
			.next_state = {
				[LFSM_EVENT_STOP] = LFSM_STATE_STOPPING,
				[LFSM_EVENT_USER_SET_RX] = LFSM_STATE_USER_BUSY_RX_PENDING,
				[LFSM_EVENT_USER_SET_TX] = LFSM_STATE_USER_BUSY_TX_PENDING,
				[LFSM_EVENT_RX_LINK] = LFSM_STATE_INACTIVE,
				[LFSM_EVENT_RX_USER] = LFSM_STATE_INACTIVE,
				[LFSM_EVENT_RX_UNKNOWN] = LFSM_STATE_INACTIVE,
			},
		}, [LFSM_STATE_USER_BUSY] = {
			.entry_fn = go_user_busy,
			.next_state = {
				[LFSM_EVENT_STOP] = LFSM_STATE_STOPPING,
				[LFSM_EVENT_USER_SET_RX] = LFSM_STATE_USER_BUSY_RX_PENDING,
				[LFSM_EVENT_USER_SET_TX] = LFSM_STATE_USER_BUSY_TX_PENDING,
				[LFSM_EVENT_USER_DONE] = LFSM_STATE_INACTIVE,
				[LFSM_EVENT_RX_LINK] = LFSM_STATE_USER_BUSY,
				[LFSM_EVENT_RX_USER] = LFSM_STATE_USER_BUSY,
				[LFSM_EVENT_RX_UNKNOWN] = LFSM_STATE_USER_BUSY,
			},
		}, [LFSM_STATE_USER_BUSY_RX_PENDING] = {
			.entry_fn = go_user_busy_rx_pending,
			.next_state = {
				[LFSM_EVENT_USER_SET_TX] = LFSM_STATE_USER_BUSY_RXTX_PENDING,
				[LFSM_EVENT_USER_DONE] = LFSM_STATE_USER_RX_PENDING,
				[LFSM_EVENT_RX_LINK] = LFSM_STATE_USER_BUSY_RX_PENDING,
				[LFSM_EVENT_RX_USER] = LFSM_STATE_USER_BUSY_RX_PENDING,
				[LFSM_EVENT_RX_UNKNOWN] = LFSM_STATE_USER_BUSY_RX_PENDING,
			},
		}, [LFSM_STATE_USER_BUSY_TX_PENDING] = {
			.entry_fn = go_user_busy_tx_pending,
			.next_state = {
				[LFSM_EVENT_USER_SET_RX] = LFSM_STATE_USER_BUSY_RXTX_PENDING,
				[LFSM_EVENT_USER_DONE] = LFSM_STATE_USER_TX_PENDING,
				[LFSM_EVENT_RX_LINK] = LFSM_STATE_USER_BUSY_TX_PENDING,
				[LFSM_EVENT_RX_USER] = LFSM_STATE_USER_BUSY_TX_PENDING,
				[LFSM_EVENT_RX_UNKNOWN] = LFSM_STATE_USER_BUSY_TX_PENDING,
			},
		}, [LFSM_STATE_USER_BUSY_RXTX_PENDING] = {
			.entry_fn = go_user_busy_rxtx_pending,
			.next_state = {
				[LFSM_EVENT_USER_DONE] = LFSM_STATE_USER_RXTX_PENDING,
				[LFSM_EVENT_RX_LINK] = LFSM_STATE_USER_BUSY_RXTX_PENDING,
				[LFSM_EVENT_RX_USER] = LFSM_STATE_USER_BUSY_RXTX_PENDING,
				[LFSM_EVENT_RX_UNKNOWN] = LFSM_STATE_USER_BUSY_RXTX_PENDING,
			},
		}, [LFSM_STATE_USER_RX_PENDING] = {
			.entry_fn = go_user_rx_pending,
			.next_state = {
				[LFSM_EVENT_RX_LINK] = LFSM_STATE_USER_RX_ACTIVE,
				[LFSM_EVENT_RX_USER] = LFSM_STATE_USER_RX_ACTIVE,
				[LFSM_EVENT_RX_UNKNOWN] = LFSM_STATE_USER_RX_ACTIVE,
			},
		}, [LFSM_STATE_USER_TX_PENDING] = {
			.entry_fn = go_user_tx_pending,
			.next_state = {
				[LFSM_EVENT_RX_LINK] = LFSM_STATE_USER_TX_ACTIVE,
				[LFSM_EVENT_RX_USER] = LFSM_STATE_USER_TX_ACTIVE,
				[LFSM_EVENT_RX_UNKNOWN] = LFSM_STATE_USER_TX_ACTIVE,
			},
		}, [LFSM_STATE_USER_RXTX_PENDING] = {
			.entry_fn = go_user_rxtx_pending,
			.next_state = {
				[LFSM_EVENT_RX_LINK] = LFSM_STATE_USER_RXTX_ACTIVE,
				[LFSM_EVENT_RX_USER] = LFSM_STATE_USER_RXTX_ACTIVE,
				[LFSM_EVENT_RX_UNKNOWN] = LFSM_STATE_USER_RXTX_ACTIVE,
			},
		}, [LFSM_STATE_USER_RX_ACTIVE] = {
			.entry_fn = go_user_rx_active,
			.next_state = {
				[LFSM_EVENT_RX_LINK] = LFSM_STATE_USER_RX_ACTIVE,
				[LFSM_EVENT_RX_USER] = LFSM_STATE_USER_BUSY,
				[LFSM_EVENT_RX_UNKNOWN] = LFSM_STATE_USER_RX_ACTIVE,
			},
		}, [LFSM_STATE_USER_TX_ACTIVE] = {
			.entry_fn = go_user_tx_active,
			.next_state = {
				[LFSM_EVENT_RX_LINK] = LFSM_STATE_INACTIVE,
				[LFSM_EVENT_RX_USER] = LFSM_STATE_INACTIVE,
				[LFSM_EVENT_RX_UNKNOWN] = LFSM_STATE_INACTIVE,
			},
		}, [LFSM_STATE_USER_RXTX_ACTIVE] = {
			.entry_fn = go_user_rxtx_active,
			.next_state = {
				[LFSM_EVENT_RX_LINK] = LFSM_STATE_USER_RX_ACTIVE,
				[LFSM_EVENT_RX_USER] = LFSM_STATE_USER_BUSY,
				[LFSM_EVENT_RX_UNKNOWN] = LFSM_STATE_USER_RX_ACTIVE,
			},
		}, };

/*
 * FSM State Entry Functions
 */

static void go_faulted(struct lfsm_context * context) {
	PIOS_DEBUG_Assert(0);
}

static void go_stopped(struct lfsm_context * context) {
#if 0
	PIOS_SPI_Stop(PIOS_SPI_OP);
#endif
}

static void go_stopping(struct lfsm_context * context) {
	context->link_tx = NULL;
	context->tx = NULL;
}

static void go_inactive(struct lfsm_context * context) {
	context->link_state = OPAHRS_MSG_LINK_STATE_INACTIVE;
	lfsm_update_link_tx(context);

	context->user_rx = NULL;
	context->user_tx = NULL;

	context->rx = context->link_rx;
	context->tx = context->link_tx;

	lfsm_init_rx(context);
	PIOS_SPI_TransferBlock(PIOS_SPI_OP, context->tx, context->rx,
			context->user_payload_len, lfsm_irq_callback);
}

static void go_user_busy(struct lfsm_context * context) {
	/* Sanity checks */
	PIOS_DEBUG_Assert(context->user_rx);

	context->user_rx = NULL;
	context->user_tx = NULL;

	context->link_state = OPAHRS_MSG_LINK_STATE_BUSY;
	lfsm_update_link_tx(context);

	context->rx = context->link_rx;
	context->tx = context->link_tx;

	lfsm_init_rx(context);
	PIOS_SPI_TransferBlock(PIOS_SPI_OP, context->tx, context->rx,
			context->user_payload_len, lfsm_irq_callback);
}

static void go_user_busy_rx_pending(struct lfsm_context * context) {
	/* Sanity checks */
	PIOS_DEBUG_Assert(context->user_rx);

	context->link_state = OPAHRS_MSG_LINK_STATE_BUSY;
	lfsm_update_link_tx(context);

	context->rx = context->link_rx;
	context->tx = context->link_tx;

	lfsm_init_rx(context);
	PIOS_SPI_TransferBlock(PIOS_SPI_OP, context->tx, context->rx,
			context->user_payload_len, lfsm_irq_callback);
}

static void go_user_busy_tx_pending(struct lfsm_context * context) {
	/* Sanity checks */
	PIOS_DEBUG_Assert(context->user_tx);

	context->link_state = OPAHRS_MSG_LINK_STATE_BUSY;
	lfsm_update_link_tx(context);

	context->rx = context->link_rx;
	context->tx = context->link_tx;

	lfsm_init_rx(context);
	PIOS_SPI_TransferBlock(PIOS_SPI_OP, context->tx, context->rx,
			context->user_payload_len, lfsm_irq_callback);
}

static void go_user_busy_rxtx_pending(struct lfsm_context * context) {
	/* Sanity checks */
	PIOS_DEBUG_Assert(context->user_rx); PIOS_DEBUG_Assert(context->user_tx);

	context->link_state = OPAHRS_MSG_LINK_STATE_BUSY;
	lfsm_update_link_tx(context);

	context->rx = context->link_rx;
	context->tx = context->link_tx;

	lfsm_init_rx(context);
	PIOS_SPI_TransferBlock(PIOS_SPI_OP, context->tx, context->rx,
			context->user_payload_len, lfsm_irq_callback);
}

static void go_user_rx_pending(struct lfsm_context * context) {
	/* Sanity checks */
	PIOS_DEBUG_Assert(context->user_rx);

	context->link_state = OPAHRS_MSG_LINK_STATE_BUSY;
	lfsm_update_link_tx(context);

	context->rx = context->link_rx;
	context->tx = context->link_tx;

	lfsm_init_rx(context);
	PIOS_SPI_TransferBlock(PIOS_SPI_OP, context->tx, context->rx,
			context->user_payload_len, lfsm_irq_callback);
}

static void go_user_tx_pending(struct lfsm_context * context) {
	/* Sanity checks */
	PIOS_DEBUG_Assert(context->user_tx);

	context->link_state = OPAHRS_MSG_LINK_STATE_BUSY;
	lfsm_update_link_tx(context);

	context->rx = context->link_rx;
	context->tx = context->link_tx;

	lfsm_init_rx(context);
	PIOS_SPI_TransferBlock(PIOS_SPI_OP, context->tx, context->rx,
			context->user_payload_len, lfsm_irq_callback);
}

static void go_user_rxtx_pending(struct lfsm_context * context) {
	/* Sanity checks */
	PIOS_DEBUG_Assert(context->user_rx); PIOS_DEBUG_Assert(context->user_tx);

	context->link_state = OPAHRS_MSG_LINK_STATE_BUSY;
	lfsm_update_link_tx(context);

	context->rx = context->link_rx;
	context->tx = context->link_tx;

	lfsm_init_rx(context);
	PIOS_SPI_TransferBlock(PIOS_SPI_OP, context->tx, context->rx,
			context->user_payload_len, lfsm_irq_callback);
}

static void go_user_rx_active(struct lfsm_context * context) {
	/* Sanity checks */
	PIOS_DEBUG_Assert(context->user_rx);

	context->rx = context->user_rx;
	context->tx = context->link_tx;
	context->link_state = OPAHRS_MSG_LINK_STATE_READY;

	lfsm_update_link_tx(context);
	lfsm_init_rx(context);
	PIOS_SPI_TransferBlock(PIOS_SPI_OP, context->tx, context->rx,
			context->user_payload_len, lfsm_irq_callback);
}

static void go_user_tx_active(struct lfsm_context * context) {
	/* Sanity checks */
	PIOS_DEBUG_Assert(context->user_tx);

	context->link_state = OPAHRS_MSG_LINK_STATE_BUSY;
	context->rx = context->link_rx;
	context->tx = context->user_tx;

	lfsm_init_rx(context);
	PIOS_SPI_TransferBlock(PIOS_SPI_OP, context->tx, context->rx,
			context->user_payload_len, lfsm_irq_callback);
}

static void go_user_rxtx_active(struct lfsm_context * context) {
	/* Sanity checks */
	PIOS_DEBUG_Assert(context->user_rx); PIOS_DEBUG_Assert(context->user_tx);

	context->link_state = OPAHRS_MSG_LINK_STATE_READY;
	context->rx = context->user_rx;
	context->tx = context->user_tx;

	lfsm_init_rx(context);
	PIOS_SPI_TransferBlock(PIOS_SPI_OP, context->tx, context->rx,
			context->user_payload_len, lfsm_irq_callback);
}

/*
 *
 * Misc Helper Functions
 *
 */

static void lfsm_update_link_tx_v0(struct opahrs_msg_v0 * msg,
		enum opahrs_msg_link_state state, uint16_t errors) {
	opahrs_msg_v0_init_link_tx(msg, OPAHRS_MSG_LINK_TAG_NOP);

	msg->payload.link.state = state;
	msg->payload.link.errors = errors;
}

static void lfsm_update_link_tx_v1(struct opahrs_msg_v1 * msg,
		enum opahrs_msg_link_state state, uint16_t errors) {
	opahrs_msg_v1_init_link_tx(msg, OPAHRS_MSG_LINK_TAG_NOP);

	msg->payload.link.state = state;
	msg->payload.link.errors = errors;
}

static void lfsm_update_link_tx(struct lfsm_context * context) {
	PIOS_DEBUG_Assert(context->link_tx);

	switch (context->user_payload_type) {
	case OPAHRS_MSG_TYPE_USER_V0:
		lfsm_update_link_tx_v0((struct opahrs_msg_v0 *) context->link_tx,
				context->link_state, context->errors);
		break;
	case OPAHRS_MSG_TYPE_USER_V1:
		lfsm_update_link_tx_v1((struct opahrs_msg_v1 *) context->link_tx,
				context->link_state, context->errors);
		break;
	case OPAHRS_MSG_TYPE_LINK:
		PIOS_DEBUG_Assert(0);
	}
}

static void lfsm_init_rx(struct lfsm_context * context) {
	PIOS_DEBUG_Assert(context->rx);

	switch (context->user_payload_type) {
	case OPAHRS_MSG_TYPE_USER_V0:
		opahrs_msg_v0_init_rx((struct opahrs_msg_v0 *) context->rx);
		break;
	case OPAHRS_MSG_TYPE_USER_V1:
		opahrs_msg_v1_init_rx((struct opahrs_msg_v1 *) context->rx);
		break;
	case OPAHRS_MSG_TYPE_LINK:
		PIOS_DEBUG_Assert(0);
	}
}

/*
 *
 * External API
 *
 */

void lfsm_inject_event(enum lfsm_event event) {
	PIOS_IRQ_Disable();

	/*
	 * Move to the next state
	 *
	 * This is done prior to calling the new state's entry function to
	 * guarantee that the entry function never depends on the previous
	 * state.  This way, it cannot ever know what the previous state was.
	 */
	context.curr_state = lfsm_transitions[context.curr_state].next_state[event];

	/* Call the entry function (if any) for the next state. */
	if (lfsm_transitions[context.curr_state].entry_fn) {
		lfsm_transitions[context.curr_state].entry_fn(&context);
	}
	PIOS_IRQ_Enable();
}

void lfsm_init(void) {
	context.curr_state = LFSM_STATE_STOPPED;
	go_stopped(&context);
}

void lfsm_set_link_proto_v0(struct opahrs_msg_v0 * link_tx,
		struct opahrs_msg_v0 * link_rx) {
	PIOS_DEBUG_Assert(link_tx);

	context.link_tx = (uint8_t *) link_tx;
	context.link_rx = (uint8_t *) link_rx;
	context.user_payload_type = OPAHRS_MSG_TYPE_USER_V0;
	context.user_payload_len = sizeof(*link_tx);

	lfsm_update_link_tx_v0(link_tx, context.link_state, context.errors);

	lfsm_inject_event(LFSM_EVENT_INIT_LINK);
}

void lfsm_set_link_proto_v1(struct opahrs_msg_v1 * link_tx,
		struct opahrs_msg_v1 * link_rx) {
	PIOS_DEBUG_Assert(link_tx);

	context.link_tx = (uint8_t *) link_tx;
	context.link_rx = (uint8_t *) link_rx;
	context.user_payload_type = OPAHRS_MSG_TYPE_USER_V1;
	context.user_payload_len = sizeof(*link_tx);

	lfsm_update_link_tx_v1(link_tx, context.link_state, context.errors);

	lfsm_inject_event(LFSM_EVENT_INIT_LINK);
}

void lfsm_user_set_tx_v0(struct opahrs_msg_v0 * user_tx) {
	PIOS_DEBUG_Assert(user_tx);

	PIOS_DEBUG_Assert(context.user_payload_type == OPAHRS_MSG_TYPE_USER_V0);
	context.user_tx = (uint8_t *) user_tx;

	lfsm_inject_event(LFSM_EVENT_USER_SET_TX);
}

void lfsm_user_set_rx_v0(struct opahrs_msg_v0 * user_rx) {
	PIOS_DEBUG_Assert(user_rx); PIOS_DEBUG_Assert(context.user_payload_type == OPAHRS_MSG_TYPE_USER_V0);

	context.user_rx = (uint8_t *) user_rx;

	lfsm_inject_event(LFSM_EVENT_USER_SET_RX);
}

void lfsm_user_set_tx_v1(struct opahrs_msg_v1 * user_tx) {
	PIOS_DEBUG_Assert(user_tx); PIOS_DEBUG_Assert(context.user_payload_type == OPAHRS_MSG_TYPE_USER_V1);

	context.user_tx = (uint8_t *) user_tx;

	lfsm_inject_event(LFSM_EVENT_USER_SET_TX);
}

void lfsm_user_set_rx_v1(struct opahrs_msg_v1 * user_rx) {
	PIOS_DEBUG_Assert(user_rx); PIOS_DEBUG_Assert(context.user_payload_type == OPAHRS_MSG_TYPE_USER_V1);

	context.user_rx = (uint8_t *) user_rx;

	lfsm_inject_event(LFSM_EVENT_USER_SET_RX);
}

void lfsm_user_done(void) {
	lfsm_inject_event(LFSM_EVENT_USER_DONE);
}

void lfsm_stop(void) {
	lfsm_inject_event(LFSM_EVENT_STOP);
}

void lfsm_get_link_stats(struct lfsm_link_stats * stats) {
	PIOS_DEBUG_Assert(stats);

	*stats = context.stats;
}

enum lfsm_state lfsm_get_state(void) {
	return context.curr_state;
}

/*
 *
 * ISR Callback
 *
 */

void lfsm_irq_callback(uint8_t crc_ok, uint8_t crc_val) {
	if (!crc_ok) {
		context.stats.rx_badcrc++;
		lfsm_inject_event(LFSM_EVENT_RX_UNKNOWN);
		return;
	}

	if (!context.rx) {
		/* No way to know what we just received, assume invalid */
		lfsm_inject_event(LFSM_EVENT_RX_UNKNOWN);
		return;
	}

	/* Recover the head and tail pointers from the message */
	struct opahrs_msg_link_head * head = 0;
	struct opahrs_msg_link_tail * tail = 0;

	switch (context.user_payload_type) {
	case OPAHRS_MSG_TYPE_USER_V0:
		head = &((struct opahrs_msg_v0 *) context.rx)->head;
		tail = &((struct opahrs_msg_v0 *) context.rx)->tail;
		break;
	case OPAHRS_MSG_TYPE_USER_V1:
		head = &((struct opahrs_msg_v1 *) context.rx)->head;
		tail = &((struct opahrs_msg_v1 *) context.rx)->tail;
		break;
	case OPAHRS_MSG_TYPE_LINK:
		/* Should never be rx'ing before the link protocol version is known */
		PIOS_DEBUG_Assert(0);
		break;
	}

	/* Check for bad magic */
	if ((head->magic != OPAHRS_MSG_MAGIC_HEAD) || (tail->magic
			!= OPAHRS_MSG_MAGIC_TAIL)) {
		if (head->magic != OPAHRS_MSG_MAGIC_HEAD) {
			context.stats.rx_badmagic_head++;
		}
		if (tail->magic != OPAHRS_MSG_MAGIC_TAIL) {
			context.stats.rx_badmagic_tail++;
		}
		lfsm_inject_event(LFSM_EVENT_RX_UNKNOWN);
		return;
	}

	/* Good magic, find out what type of payload we've got */
	switch (head->type) {
	case OPAHRS_MSG_TYPE_LINK:
		context.stats.rx_link++;
		lfsm_inject_event(LFSM_EVENT_RX_LINK);
		break;
	case OPAHRS_MSG_TYPE_USER_V0:
	case OPAHRS_MSG_TYPE_USER_V1:
		if (head->type == context.user_payload_type) {
			context.stats.rx_user++;
			lfsm_inject_event(LFSM_EVENT_RX_USER);
		} else {
			/* Mismatched user payload type */
			context.stats.rx_badver++;
			lfsm_inject_event(LFSM_EVENT_RX_UNKNOWN);
		}
		break;
	default:
		/* Unidentifiable payload type */
		context.stats.rx_badtype++;
		lfsm_inject_event(LFSM_EVENT_RX_UNKNOWN);
	}
}
