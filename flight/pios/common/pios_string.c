/**
 ******************************************************************************
 *
 * @file       pios_string.c
 * @author     The OpenPilot Team, http://www.openpilot.org Copyright (C) 2014.
 * @addtogroup PiOS
 * @{
 * @addtogroup PiOS
 * @{
 * @brief PiOS string methods
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
#include <pios.h>
#include <pios_mem.h>
#include <pios_string.h>

#include "limits.h"

size_t pios_strnlen(const char *str, size_t n)
{
  const char *start = str;

  while (n-- > 0 && *str)
    str++;

  return str - start;
}

 char *pios_strcpy(char *dst0, const char *src0)
 {
   char *s = dst0;

   while ((*dst0++ = *src0++))
     ;

   return s;
 }

 char *pios_strcat(char *s1, const char *s2)
 {
   char *s = s1;

   while (*s1)
     s1++;

   while ((*s1++ = *s2++))
     ;
   return s;
 }

 char *pios_strncpy(char *dst0, const char *src0, size_t count)
 {
   char *dscan;
   const char *sscan;

   dscan = dst0;
   sscan = src0;
   while (count > 0)
     {
       --count;
       if ((*dscan++ = *sscan++) == '\0')
 	break;
     }
   while (count-- > 0)
     *dscan++ = '\0';

   return dst0;
 }


 int pios_strcmp(const char *s1, const char *s2)
 {
   while (*s1 != '\0' && *s1 == *s2)
     {
       s1++;
       s2++;
     }

   return (*(unsigned char *) s1) - (*(unsigned char *) s2);
 }

 int pios_strncmp(const char *s1, const char *s2, size_t n)
 {
   if (n == 0)
     return 0;

   while (n-- != 0 && *s1 == *s2)
     {
       if (n == 0 || *s1 == '\0')
 	break;
       s1++;
       s2++;
     }

   return (*(unsigned char *) s1) - (*(unsigned char *) s2);
 }

 char *pios_strndup(const char *s, uint16_t max_string_length)
 {
         char *new;

         new = pios_malloc(max_string_length);
         if(new) {
                 pios_strncpy(new, s, max_string_length);
         }

         return new;

 }



