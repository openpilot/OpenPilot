/**
 ******************************************************************************
 *
 * @file       pios_string.h
 * @author     The OpenPilot Team, http://www.openpilot.org Copyright (C) 2014.
 * @addtogroup PiOS
 * @{
 * @addtogroup PiOS
 * @{
 * @brief PiOS string rountines
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
#ifndef PIOS_STRING_H
#define PIOS_STRING_H


size_t pios_strnlen(const char *str, size_t n);

char *pios_strcpy(char *dst0, const char *src0);

char *pios_strcat(char *s1, const char *s2);

char *pios_strncpy(char *dst0, const char *src0, size_t count);

int pios_strcmp(const char *s1, const char *s2);

int pios_strncmp(const char *s1, const char *s2, size_t n);

char *pios_strndup(const char *s, uint16_t max_string_length);

#endif /* PIOS_STRING_H */
