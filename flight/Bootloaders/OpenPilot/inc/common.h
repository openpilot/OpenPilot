/*
 * common.h
 *
 *  Created on: 2010/08/18
 *      Author: Programacao
 */

#ifndef COMMON_H_
#define COMMON_H_


/**************************************************/
/* OP_DFU Memory locations                       */
/**************************************************/

#define StartOfUserCode					0x08006000

/**************************************************/
/* OP_DFU Mem Sizes			                      */
/**************************************************/

#define SizeOfHash					60
#define SizeOfDescription			100
#define SizoOfCode					100



/**************************************************/
/* OP_DFU states                       */
/**************************************************/

#define DFUidle					0
#define uploading				1
#define wrong_packet_received	2
#define too_many_packets		3
#define too_few_packets			4
#define Last_operation_Success	5
#define downloading				6
#define idle					7
#define Last_operation_failed	8
#define uploadingStarting		9

/**************************************************/
/* OP_DFU commands                       */
/**************************************************/

#define Reserved						0
#define Req_Capabilities				1
#define Rep_Capabilities				2
#define EnterDFU						3
#define JumpFW							4
#define Reset 							5
#define Abort_Operation					6
#define Upload							7
#define Op_END							8
#define Download_Req					9
#define Download						10
#define Status_Request					11
#define Status_Rep 						12

/**************************************************/
/* OP_DFU transfer types                       */
/**************************************************/

#define FW								0
#define Hash							1
#define Descript						2

#define DownloadDelay					100000
#endif /* COMMON_H_ */
