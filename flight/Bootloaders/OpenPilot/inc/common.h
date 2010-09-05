/*
 * common.h
 *
 *  Created on: 2010/08/18
 *      Author: Programacao
 */

#ifndef COMMON_H_
#define COMMON_H_

#include "board.h"

typedef enum {
	start, keepgoing,
} DownloadAction;

/**************************************************/
/* OP_DFU states                       */
/**************************************************/

typedef enum {
	DFUidle, //0
	uploading, //1
	wrong_packet_received, //2
	too_many_packets, //3
	too_few_packets, //4
	Last_operation_Success, //5
	downloading, //6
	idle, //7
	Last_operation_failed, //8
	uploadingStarting, //9
	outsideDevCapabilities, //10
	test
} DFUStates;
/**************************************************/
/* OP_DFU commands                       */
/**************************************************/
typedef enum {
	Reserved, //0
	Req_Capabilities, //1
	Rep_Capabilities, //2
	EnterDFU, //3
	JumpFW, //4
	Reset, //5
	Abort_Operation, //6
	Upload, //7
	Op_END, //8
	Download_Req, //9
	Download, //10
	Status_Request, //11
	Status_Rep
//12
} DFUCommands;
/**************************************************/
/* OP_DFU transfer types                       */
/**************************************************/
typedef enum {
	FW, //0
	Hash, //1
	Descript
//2
} DFUTransfer;
/**************************************************/
/* OP_DFU programable programable HW types        */
/**************************************************/
typedef enum {
	Self_flash, //0
	Remote_flash_via_spi
//1
} DFUProgType;
/**************************************************/
/* OP_DFU programable sources			          */
/**************************************************/
#define USB	0
#define SPI 1

#define DownloadDelay					100000

#define MAX_DEL_RETRYS					3
#define MAX_WRI_RETRYS					3

#endif /* COMMON_H_ */
