/*
 * board.h
 *
 *  Created on: 2010/08/31
 *      Author: Programacao
 */

#ifndef BOARD_H_
#define BOARD_H_

#define deviceID 69

#define board_can_read	1
#define board_can_write	1


/**************************************************/
/* OP_DFU Memory locations                       */
/**************************************************/

#define StartOfUserCode					0x08006000

/**************************************************/
/* OP_DFU Mem Sizes			                      */
/**************************************************/

#define SizeOfHash					20
#define SizeOfDescription			100
#define SizeOfCode					499712-SizeOfHash-SizeOfDescription //488K

#endif /* BOARD_H_ */
