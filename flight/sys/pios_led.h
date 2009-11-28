
/**
 * Project: OpenPilot
 *    
 * @author The OpenPilot Team, http://www.openpilot.org, Copyright (C) 2009.
 *    
 * @see The GNU Public License (GPL)
 */
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

#ifndef PIOS_LED_H
#define PIOS_LED_H

/* Type Definitions */
typedef enum {LED1 = 0, LED2 = 1} Led_TypeDef;

/* Exported Functions */
void LED_INIT(void);
void LED_ON(Led_TypeDef LEDNum);
void LED_OFF(Led_TypeDef LEDNum);
void LED_TOGGLE(Led_TypeDef LEDNum);

#endif /* PIOS_LED_H */
