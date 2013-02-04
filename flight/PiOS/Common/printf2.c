/*******************************************************************************
 Copyright 2001, 2002 Georges Menie (<URL snipped>)
  This program is free software; you can redistribute it and/or modify
  it under the terms of the GNU Lesser General Public License as published by
  the Free Software Foundation; either version 2 of the License, or
  (at your option) any later version.
  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
  GNU Lesser General Public License for more details.
  You should have received a copy of the GNU Lesser General Public License
  along with this program; if not, write to the Free Software
  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
*/

/*******************************************************************************
 putchar is the only external dependency for this file,
 if you have a working putchar, just remove the following
 define. If the function should be called something else,
 replace outbyte(c) by your own function call.
*/
//*******************************************************************************
//  Updated by Daniel D Miller.  Changes to the original Menie code are
//  Copyright 2009-2012 Daniel D Miller
//  All such changes are distributed under the same license as the original,
//  as described above.
//  11/06/09 - adding floating-point support
//  03/19/10 - pad fractional portion of floating-point number with 0s
//  03/30/10 - document the \% bug
//  07/20/10 - Fix a round-off bug in floating-point conversions
//             ( 0.99 with %.1f did not round to 1.0 )
//  10/25/11 - Add support for %+ format (always show + on positive numbers)
//  01/19/12 - fix handling of %f with no decimal; it was defaulting to 0
//             decimal places, rather than printf's 6.
//*******************************************************************************
//  BUGS
//  If '%' is included in a format string, in the form \% with the intent
//  of including the percent sign in the output string, this function will
//  mis-handle the data entirely!!
//  Essentially, it will just discard the character following the percent sign.
//  This bug is not easy to fix in the existing code;
//  for now, I'll just try to remember to use %% instead of \% ...
//*******************************************************************************

//lint -esym(752, debug_output)
//lint -esym(766, stdio.h)

// #define  TEST_PRINTF    1

#include <pios.h>

static uint use_leading_plus = 0 ;

/* based on a example-code from Keil for CS G++ */

/* for caddr_t (typedef char * caddr_t;) */
#include <sys/types.h>

//* NEWLIB STUBS *//
#include <stdlib.h>
#include <sys/unistd.h>
#include <sys/stat.h>
#include <sys/times.h>
#include <errno.h>

/*==============================================================================
 * Environment variables.
 * A pointer to a list of environment variables and their values. For a minimal
 * environment, this empty list is adequate:
 */
char *__env[1] = { 0 };
char **environ = __env;

/*==============================================================================
 * Close a file.
 */
int _close(int file)
{
    return -1;
}

/*==============================================================================
 * Transfer control to a new process.
 */
int _execve(char *name, char **argv, char **env)
{
    errno = ENOMEM;
    return -1;
}

/*==============================================================================
 * Exit a program without cleaning up files.
 */
void _exit( int code )
{
	/* Should we force a system reset? */
	while( 1 )
	{
		;
	}
}

/*==============================================================================
 * Create a new process.
 */
int _fork(void)
{
    errno = EAGAIN;
    return -1;
}

/*==============================================================================
 * Status of an open file.
 */
int _fstat(int file, struct stat *st)
{
    st->st_mode = S_IFCHR;
    return 0;
}

/*==============================================================================
 * Process-ID
 */
int _getpid(void)
{
    return 1;
}

/*==============================================================================
 * Query whether output stream is a terminal.
 */
int _isatty(int file)
{
    return 1;
}

/*==============================================================================
 * Send a signal.
 */
int _kill(int pid, int sig)
{
    errno = EINVAL;
    return -1;
}

/*==============================================================================
 * Establish a new name for an existing file.
 */
int _link(char *old, char *new)
{
    errno = EMLINK;
    return -1;
}

/*==============================================================================
 * Set position in a file.
 */
int _lseek(int file, int ptr, int dir)
{
    return 0;
}

/*==============================================================================
 * Open a file.
 */
int _open(const char *name, int flags, int mode)
{
    return -1;
}

/*==============================================================================
 * Read from a file.
 */
int _read(int file, char *ptr, int len)
{
    return 0;
}

/*==============================================================================
 * Write to a file. libc subroutines will use this system routine for output to
 * all files, including stdout—so if you need to generate any output, for
 * example to a serial port for debugging, you should make your minimal write
 * capable of doing this.
 */
int _write_r( void * reent, int file, char * ptr, int len )
{
    return 0;
}

/*==============================================================================
 * Increase program data space. As malloc and related functions depend on this,
 * it is useful to have a working implementation. The following suffices for a
 * standalone system; it exploits the symbol _end automatically defined by the
 * GNU linker.
 */
caddr_t _sbrk(int incr)
{
    extern char _end;		/* Defined by the linker */
    static char *heap_end;
    char *prev_heap_end;
    char * stack_ptr;

    if (heap_end == 0)
    {
      heap_end = &_end;
    }

    prev_heap_end = heap_end;
    asm volatile ("MRS %0, msp" : "=r" (stack_ptr) );
    if (heap_end + incr > stack_ptr)
    {
      _write_r ((void *)0, 1, "Heap and stack collision\n", 25);
      _exit (1);
    }

    heap_end += incr;
    return (caddr_t) prev_heap_end;
}

/*==============================================================================
 * Status of a file (by name).
 */
int _stat(char *file, struct stat *st)
{
    st->st_mode = S_IFCHR;
    return 0;
}

/*==============================================================================
 * Timing information for current process.
 */
int _times(struct tms *buf)
{
    return -1;
}

/*==============================================================================
 * Remove a file's directory entry.
 */
int _unlink(char *name)
{
    errno = ENOENT;
    return -1;
}

/*==============================================================================
 * Wait for a child process.
 */
int _wait(int *status)
{
    errno = ECHILD;
    return -1;
}
//* NEWLIB STUBS *//


//****************************************************************************
static void printchar (char **str, int c)
{
	if (str) {
		**str = c;
		++(*str);
	}
#ifdef TEST_PRINTF
   else {
      extern int putchar (int c);
		(void) putchar (c);
   }
#endif
}

//****************************************************************************
static uint my_strlen(char *str)
{
   if (str == 0)
      return 0;
   uint slen = 0 ;
   while (*str != 0) {
      slen++ ;
      str++ ;
   }
   return slen;
}

//****************************************************************************
//  This version returns the length of the output string.
//  It is more useful when implementing a walking-string function.
//****************************************************************************
static const double round_nums[8] = {
   0.5,
   0.05,
   0.005,
   0.0005,
   0.00005,
   0.000005,
   0.0000005,
   0.00000005
} ;

static unsigned dbl2stri(char *outbfr, double dbl, unsigned dec_digits)
{
   static char local_bfr[128] ;
   char *output = (outbfr == 0) ? local_bfr : outbfr ;

   //*******************************************
   //  extract negative info
   //*******************************************
   if (dbl < 0.0) {
      *output++ = '-' ;
      dbl *= -1.0 ;
   } else {
      if (use_leading_plus) {
         *output++ = '+' ;
      }

   }

   //  handling rounding by adding .5LSB to the floating-point data
   if (dec_digits < 8) {
      dbl += round_nums[dec_digits] ;
   }

   //**************************************************************************
   //  construct fractional multiplier for specified number of digits.
   //**************************************************************************
   uint mult = 1 ;
   uint idx ;
   for (idx=0; idx < dec_digits; idx++)
      mult *= 10 ;

   // printf("mult=%u\n", mult) ;
   uint wholeNum = (uint) dbl ;
   uint decimalNum = (uint) ((dbl - wholeNum) * mult);

   //*******************************************
   //  convert integer portion
   //*******************************************
   char tbfr[40] ;
   idx = 0 ;
   while (wholeNum != 0) {
      tbfr[idx++] = '0' + (wholeNum % 10) ;
      wholeNum /= 10 ;
   }
   // printf("%.3f: whole=%s, dec=%d\n", dbl, tbfr, decimalNum) ;
   if (idx == 0) {
      *output++ = '0' ;
   } else {
      while (idx > 0) {
         *output++ = tbfr[idx-1] ;  //lint !e771
         idx-- ;
      }
   }
   if (dec_digits > 0) {
      *output++ = '.' ;

      //*******************************************
      //  convert fractional portion
      //*******************************************
      idx = 0 ;
      while (decimalNum != 0) {
         tbfr[idx++] = '0' + (decimalNum % 10) ;
         decimalNum /= 10 ;
      }
      //  pad the decimal portion with 0s as necessary;
      //  We wouldn't want to report 3.093 as 3.93, would we??
      while (idx < dec_digits) {
         tbfr[idx++] = '0' ;
      }
      // printf("decimal=%s\n", tbfr) ;
      if (idx == 0) {
         *output++ = '0' ;
      } else {
         while (idx > 0) {
            *output++ = tbfr[idx-1] ;
            idx-- ;
         }
      }
   }
   *output = 0 ;

   //  prepare output
   output = (outbfr == 0) ? local_bfr : outbfr ;
   return my_strlen(output) ;
}

//****************************************************************************
#define  PAD_RIGHT   1
#define  PAD_ZERO    2

static int prints (char **out, const char *string, int width, int pad)
{
	register int pc = 0, padchar = ' ';
	if (width > 0) {
      int len = 0;
      const char *ptr;
		for (ptr = string; *ptr; ++ptr)
			++len;
		if (len >= width)
			width = 0;
		else
			width -= len;
		if (pad & PAD_ZERO)
			padchar = '0';
	}
	if (!(pad & PAD_RIGHT)) {
		for (; width > 0; --width) {
			printchar (out, padchar);
			++pc;
		}
	}
	for (; *string; ++string) {
		printchar (out, *string);
		++pc;
	}
	for (; width > 0; --width) {
		printchar (out, padchar);
		++pc;
	}
	return pc;
}

//****************************************************************************
/* the following should be enough for 32 bit int */
#define PRINT_BUF_LEN 12
static int printi (char **out, int i, int b, int sg, int width, int pad, int letbase)
{
	char print_buf[PRINT_BUF_LEN];
   char *s;
   int t, neg = 0, pc = 0;
   unsigned u = (unsigned) i;
	if (i == 0) {
		print_buf[0] = '0';
		print_buf[1] = '\0';
		return prints (out, print_buf, width, pad);
	}
	if (sg && b == 10 && i < 0) {
		neg = 1;
      u = (unsigned) -i;
	}
   //  make sure print_buf is NULL-term
	s = print_buf + PRINT_BUF_LEN - 1;
	*s = '\0';


	while (u) {
      t = u % b;  //lint !e573  Warning 573: Signed-unsigned mix with divide
		if (t >= 10)
			t += letbase - '0' - 10;
		*--s = t + '0';
      u /= b;  //lint !e573  Warning 573: Signed-unsigned mix with divide
	}
	if (neg) {
		if (width && (pad & PAD_ZERO)) {
			printchar (out, '-');
			++pc;
			--width;
		}
		else {
			*--s = '-';
		}
   } else {
      if (use_leading_plus) {
         *--s = '+';
      }
   }
	return pc + prints (out, s, width, pad);
}

//****************************************************************************
static int print (char **out, int *varg)
{
   int post_decimal ;
   int width, pad ;
   unsigned dec_width = 6 ;
   int pc = 0;
   char *format = (char *) (*varg++);
   char scr[2];
   use_leading_plus = 0 ;  //  start out with this clear
	for (; *format != 0; ++format) {
		if (*format == '%') {
         dec_width = 6 ;
			++format;
			width = pad = 0;
			if (*format == '\0')
				break;
			if (*format == '%')
            goto out_lbl;
			if (*format == '-') {
				++format;
				pad = PAD_RIGHT;
			}
         if (*format == '+') {
            ++format;
            use_leading_plus = 1 ;
         }
         while (*format == '0') {
				++format;
				pad |= PAD_ZERO;
			}
         post_decimal = 0 ;
         if (*format == '.'  ||
            (*format >= '0' &&  *format <= '9')) {

            while (1) {
               if (*format == '.') {
                  post_decimal = 1 ;
                  dec_width = 0 ;
                  format++ ;
               } else if ((*format >= '0' &&  *format <= '9')) {
                  if (post_decimal) {
                     dec_width *= 10;
                     dec_width += *format - '0';
                  } else {
                     width *= 10;
                     width += *format - '0';
                  }
                  format++ ;
               } else {
                  break;
               }
            }
         }
         if (*format == 'l')
            ++format;
         switch (*format) {
         case 's':
            {
            // char *s = *((char **) varg++);   //lint !e740
            char *s = (char *) *varg++ ;  //lint !e740 !e826  convert to double pointer
            pc += prints (out, s ? s : "(null)", width, pad);
            use_leading_plus = 0 ;  //  reset this flag after printing one value
            }
            break;
         case 'd':
            pc += printi (out, *varg++, 10, 1, width, pad, 'a');
            use_leading_plus = 0 ;  //  reset this flag after printing one value
            break;
         case 'x':
            pc += printi (out, *varg++, 16, 0, width, pad, 'a');
            use_leading_plus = 0 ;  //  reset this flag after printing one value
            break;
         case 'X':
            pc += printi (out, *varg++, 16, 0, width, pad, 'A');
            use_leading_plus = 0 ;  //  reset this flag after printing one value
            break;
         case 'u':
            pc += printi (out, *varg++, 10, 0, width, pad, 'a');
            use_leading_plus = 0 ;  //  reset this flag after printing one value
            break;
         case 'c':
            /* char are converted to int then pushed on the stack */
            scr[0] = *varg++;
            scr[1] = '\0';
            pc += prints (out, scr, width, pad);
            use_leading_plus = 0 ;  //  reset this flag after printing one value
            break;

         case 'f':
            {
            // http://wiki.debian.org/ArmEabiPort#Structpackingandalignment
            // Stack alignment
            //
            // The ARM EABI requires 8-byte stack alignment at public function entry points,
            // compared to the previous 4-byte alignment.
#ifdef USE_NEWLIB
            char *cptr = (char *) varg ;  //lint !e740 !e826  convert to double pointer
            uint caddr = (uint) cptr ;
            if ((caddr & 0xF) != 0) {
               cptr += 4 ;
            }
            double *dblptr = (double *) cptr ;  //lint !e740 !e826  convert to double pointer
#else
            double *dblptr = (double *) varg ;  //lint !e740 !e826  convert to double pointer
#endif
            double dbl = *dblptr++ ;   //  increment double pointer
            varg = (int *) dblptr ;    //lint !e740  copy updated pointer back to base pointer
            char bfr[81] ;
            // unsigned slen =
            dbl2stri(bfr, dbl, dec_width) ;
            // stuff_talkf("[%s], width=%u, dec_width=%u\n", bfr, width, dec_width) ;
            pc += prints (out, bfr, width, pad);
            use_leading_plus = 0 ;  //  reset this flag after printing one value
            }
            break;

         default:
            printchar (out, '%');
            printchar (out, *format);
            use_leading_plus = 0 ;  //  reset this flag after printing one value
            break;
         }
      } else
      // if (*format == '\\') {
      //
      // } else
      {
out_lbl:
			printchar (out, *format);
			++pc;
		}
   }  //  for each char in format string
   if (out) //lint !e850
		**out = '\0';
	return pc;
}

//****************************************************************************
int stringf (char *out, const char *format, ...)
{
   //  create a pointer into the stack.
   //  Thematically this should be a void*, since the actual usage of the
   //  pointer will vary.  However, int* works fine too.
   //  Either way, the called function will need to re-cast the pointer
   //  for any type which isn't sizeof(int)
   int *varg = (int *) (char *) (&format);
   return print (&out, varg);
}

int printf(const char *format, ...)
{
   int *varg = (int *) (char *) (&format);
   return print (0, varg);
}

int sprintf(char *out, const char *format, ...)
{
   int *varg = (int *) (char *) (&format);
   return print (&out, varg);
}

/**
  * @}
  */
