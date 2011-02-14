/**
 ******************************************************************************
 * @addtogroup PIOS PIOS Core hardware abstraction layer
 * @{
 * @addtogroup PIOS_OPAHRS OPAHRS Functions
 * @brief HAL code to interface to the OpenPilot AHRS module's bootloader
 * @{
 *
 * @file       pios_opahrs_download.c
 * @author     The OpenPilot Team, http://www.openpilot.org Copyright (C) 2010.
 * @brief      Hardware commands to communicate with the AHRS bootloader
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

/* Project Includes */
#include "pios.h"

#if defined(PIOS_INCLUDE_OPAHRS)

#include "pios_opahrs_proto.h"
#include "pios_opahrs.h"

static uint32_t PIOS_OPAHRS_SPI;

void PIOS_OPAHRS_Attach(uint32_t spi_id)
{
	PIOS_OPAHRS_SPI = spi_id;
}

void PIOS_OPAHRS_ForceSlaveSelected(bool selected)
{
	if (selected) {
		PIOS_SPI_RC_PinSet(PIOS_OPAHRS_SPI, 0);
	} else {
		PIOS_SPI_RC_PinSet(PIOS_OPAHRS_SPI, 1);
	}
}

static int32_t opahrs_msg_txrx(const uint8_t * tx, uint8_t * rx, uint32_t len)
{
	int32_t rc;

	PIOS_SPI_RC_PinSet(PIOS_OPAHRS_SPI, 0);
#ifdef PIOS_INCLUDE_FREERTOS
	vTaskDelay(20 / portTICK_RATE_MS);
#else
	PIOS_DELAY_WaitmS(1);
#endif
	rc = PIOS_SPI_TransferBlock(PIOS_OPAHRS_SPI, tx, rx, len, NULL);
	PIOS_SPI_RC_PinSet(PIOS_OPAHRS_SPI, 1);
	return (rc);
}

static enum opahrs_result opahrs_msg_v0_send_req(const struct opahrs_msg_v0 *req)
{
	int32_t rc;
	struct opahrs_msg_v0 link_rx;

	for (uint8_t retries = 0; retries < 20; retries++) {
		struct opahrs_msg_v0 *rsp = &link_rx;

		if ((rc = opahrs_msg_txrx((const uint8_t *)req, (uint8_t *) rsp, sizeof(*rsp))) < 0) {
			return OPAHRS_RESULT_FAILED;
		}

		/* Make sure we got a sane response by checking the magic */
		if ((rsp->head.magic != OPAHRS_MSG_MAGIC_HEAD) || (rsp->tail.magic != OPAHRS_MSG_MAGIC_TAIL)) {
			return OPAHRS_RESULT_FAILED;
		}

		switch (rsp->head.type) {
		case OPAHRS_MSG_TYPE_LINK:
			switch (rsp->payload.link.state) {
			case OPAHRS_MSG_LINK_STATE_BUSY:
			case OPAHRS_MSG_LINK_STATE_INACTIVE:
				/* Wait for a small delay and retry */
#ifdef PIOS_INCLUDE_FREERTOS
				vTaskDelay(20 / portTICK_RATE_MS);
#else
				//PIOS_DELAY_WaitmS(1);
#endif
				continue;
			case OPAHRS_MSG_LINK_STATE_READY:
				/* Peer was ready when we Tx'd so they have now Rx'd our message */
				return OPAHRS_RESULT_OK;
			}
			break;
		case OPAHRS_MSG_TYPE_USER_V0:
		case OPAHRS_MSG_TYPE_USER_V1:
			/* Wait for a small delay and retry */
#ifdef PIOS_INCLUDE_FREERTOS
			vTaskDelay(50 / portTICK_RATE_MS);
#else
			//PIOS_DELAY_WaitmS(1);
#endif
			continue;
		}
	}

	return OPAHRS_RESULT_TIMEOUT;
}

static enum opahrs_result opahrs_msg_v0_recv_rsp(enum opahrs_msg_v0_tag tag, struct opahrs_msg_v0 *rsp)
{
	struct opahrs_msg_v0 link_tx;

	opahrs_msg_v0_init_link_tx(&link_tx, OPAHRS_MSG_LINK_TAG_NOP);

	for (uint8_t retries = 0; retries < 20; retries++) {
		if (opahrs_msg_txrx((const uint8_t *)&link_tx, (uint8_t *) rsp, sizeof(*rsp)) < 0) {
			return OPAHRS_RESULT_FAILED;
		}

		/* Make sure we got a sane response by checking the magic */
		if ((rsp->head.magic != OPAHRS_MSG_MAGIC_HEAD) || (rsp->tail.magic != OPAHRS_MSG_MAGIC_TAIL)) {
			return OPAHRS_RESULT_FAILED;
		}

		switch (rsp->head.type) {
		case OPAHRS_MSG_TYPE_LINK:
			switch (rsp->payload.link.state) {
			case OPAHRS_MSG_LINK_STATE_BUSY:
				/* Wait for a small delay and retry */
#ifdef PIOS_INCLUDE_FREERTOS
				vTaskDelay(20 / portTICK_RATE_MS);
#else
				PIOS_DELAY_WaitmS(1);
#endif
				continue;
			case OPAHRS_MSG_LINK_STATE_INACTIVE:
			case OPAHRS_MSG_LINK_STATE_READY:
				/* somehow, we've missed our response */
				return OPAHRS_RESULT_FAILED;
			}
			break;
		case OPAHRS_MSG_TYPE_USER_V0:
			if (rsp->payload.user.t == tag) {
				return OPAHRS_RESULT_OK;
			} else {
				return OPAHRS_RESULT_FAILED;
			}
			break;
		case OPAHRS_MSG_TYPE_USER_V1:
			/* This isn't the type we expected */
			return OPAHRS_RESULT_FAILED;
			break;
		}
	}

	return OPAHRS_RESULT_TIMEOUT;
}

static enum opahrs_result PIOS_OPAHRS_v0_simple_req(enum opahrs_msg_v0_tag req_type, struct opahrs_msg_v0 *rsp, enum opahrs_msg_v0_tag rsp_type)
{
	struct opahrs_msg_v0 req;
	enum opahrs_result rc;

	/* Make up an empty request */
	opahrs_msg_v0_init_user_tx(&req, req_type);

	/* Send the message until it is received */

	rc = opahrs_msg_v0_send_req(&req);
	if ((rc == OPAHRS_RESULT_OK) && rsp) {
		/* We need a specific kind of reply, go get it */
		return opahrs_msg_v0_recv_rsp(rsp_type, rsp);
	}

	return rc;
}

enum opahrs_result PIOS_OPAHRS_bl_GetVersions(struct opahrs_msg_v0 *rsp)
{
	if (!rsp)
		return OPAHRS_RESULT_FAILED;

	return (PIOS_OPAHRS_v0_simple_req(OPAHRS_MSG_V0_REQ_VERSIONS, rsp, OPAHRS_MSG_V0_RSP_VERSIONS));
}

enum opahrs_result PIOS_OPAHRS_bl_GetMemMap(struct opahrs_msg_v0 *rsp)
{
	if (!rsp)
		return OPAHRS_RESULT_FAILED;

	return (PIOS_OPAHRS_v0_simple_req(OPAHRS_MSG_V0_REQ_MEM_MAP, rsp, OPAHRS_MSG_V0_RSP_MEM_MAP));
}

enum opahrs_result PIOS_OPAHRS_bl_GetSerial(struct opahrs_msg_v0 *rsp)
{
	if (!rsp)
		return OPAHRS_RESULT_FAILED;

	return (PIOS_OPAHRS_v0_simple_req(OPAHRS_MSG_V0_REQ_SERIAL, rsp, OPAHRS_MSG_V0_RSP_SERIAL));
}

enum opahrs_result PIOS_OPAHRS_bl_FwupStart(struct opahrs_msg_v0 *rsp)
{
	if (!rsp)
		return OPAHRS_RESULT_FAILED;
	return (PIOS_OPAHRS_v0_simple_req(OPAHRS_MSG_V0_REQ_FWUP_START, rsp, OPAHRS_MSG_V0_RSP_FWUP_STATUS));
}

enum opahrs_result PIOS_OPAHRS_bl_FwupStatus(struct opahrs_msg_v0 *rsp)
{
	if (!rsp)
		return OPAHRS_RESULT_FAILED;
	return (PIOS_OPAHRS_v0_simple_req(OPAHRS_MSG_V0_REQ_FWUP_STATUS, rsp, OPAHRS_MSG_V0_RSP_FWUP_STATUS));
}

enum opahrs_result PIOS_OPAHRS_bl_FwupData(struct opahrs_msg_v0 *req, struct opahrs_msg_v0 *rsp)
{
	if (!req || !rsp)
		return OPAHRS_RESULT_FAILED;

	/* Make up an attituderaw request */
	opahrs_msg_v0_init_user_tx(req, OPAHRS_MSG_V0_REQ_FWUP_DATA);
	enum opahrs_result rc;
	/* Send the message until it is received */
	rc = opahrs_msg_v0_send_req(req);
	if (rc != OPAHRS_RESULT_OK) {
		/* Failed to send the request, bail out */
		return rc;
	}

	return opahrs_msg_v0_recv_rsp(OPAHRS_MSG_V0_RSP_FWUP_STATUS, rsp);
}

enum opahrs_result PIOS_OPAHRS_bl_FwDlData(struct opahrs_msg_v0 *req, struct opahrs_msg_v0 *rsp)
{
	if (!req || !rsp)
		return OPAHRS_RESULT_FAILED;

	/* Make up an attituderaw request */
	opahrs_msg_v0_init_user_tx(req, OPAHRS_MSG_V0_REQ_FWDN_DATA);
	enum opahrs_result rc;
	/* Send the message until it is received */
	rc = opahrs_msg_v0_send_req(req);
	if (rc != OPAHRS_RESULT_OK) {
		/* Failed to send the request, bail out */
		return rc;
	}

	return opahrs_msg_v0_recv_rsp(OPAHRS_MSG_V0_RSP_FWDN_DATA, rsp);
}

enum opahrs_result PIOS_OPAHRS_bl_FwupVerify(struct opahrs_msg_v0 *rsp)
{
	if (!rsp)
		return OPAHRS_RESULT_FAILED;

	return (PIOS_OPAHRS_v0_simple_req(OPAHRS_MSG_V0_REQ_FWUP_VERIFY, rsp, OPAHRS_MSG_V0_RSP_FWUP_STATUS));
}

enum opahrs_result PIOS_OPAHRS_bl_resync(void)
{
	struct opahrs_msg_v0 req;
	struct opahrs_msg_v0 rsp;

	enum opahrs_result rc = OPAHRS_RESULT_FAILED;

	opahrs_msg_v0_init_link_tx(&req, OPAHRS_MSG_LINK_TAG_NOP);

	PIOS_SPI_RC_PinSet(PIOS_OPAHRS_SPI, 0);
#ifdef PIOS_INCLUDE_FREERTOS
	vTaskDelay(20 / portTICK_RATE_MS);
#else
	//PIOS_DELAY_WaitmS(1);
#endif

	for (uint32_t i = 0; i < sizeof(req); i++) {
		/* Tx a shortened (by one byte) message to walk through all byte positions */
		opahrs_msg_v0_init_rx(&rsp);
		PIOS_SPI_TransferBlock(PIOS_OPAHRS_SPI, (uint8_t *) & req, (uint8_t *) & rsp, sizeof(req) - 1, NULL);

		/* Good magic means we're sync'd */
		if ((rsp.head.magic == OPAHRS_MSG_MAGIC_HEAD) && (rsp.tail.magic == OPAHRS_MSG_MAGIC_TAIL)) {
			/* We need to shift out one more byte to compensate for the short tx */
			PIOS_SPI_TransferByte(PIOS_OPAHRS_SPI, 0x00);
			rc = OPAHRS_RESULT_OK;
			break;
		}
#ifdef PIOS_INCLUDE_FREERTOS
		vTaskDelay(10 / portTICK_RATE_MS);
#else
		PIOS_DELAY_WaitmS(1);
#endif
	}

	PIOS_SPI_RC_PinSet(PIOS_OPAHRS_SPI, 1);
	//vTaskDelay(5 / portTICK_RATE_MS);

	return rc;
}

enum opahrs_result PIOS_OPAHRS_bl_reset(uint32_t delay)
{
	struct opahrs_msg_v0 req;
	/* Make up an attituderaw request */
	opahrs_msg_v0_init_user_tx(&req, OPAHRS_MSG_V0_REQ_RESET);
	req.payload.user.v.req.reset.reset_delay_in_ms = delay;
	/* Send the message until it is received */
	return opahrs_msg_v0_send_req(&req);
}

enum opahrs_result PIOS_OPAHRS_bl_boot(uint32_t delay)
{
	struct opahrs_msg_v0 req;
	/* Make up an attituderaw request */
	opahrs_msg_v0_init_user_tx(&req, OPAHRS_MSG_V0_REQ_BOOT);
	req.payload.user.v.req.boot.boot_delay_in_ms = delay;
	/* Send the message until it is received */
	return opahrs_msg_v0_send_req(&req);
}
#endif /* PIOS_INCLUDE_OPAHRS */
