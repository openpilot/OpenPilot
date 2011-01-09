# This file is Copyright 2007, 2009, 2010 Dean Hall.
#
# This file is part of the Python-on-a-Chip program.
# Python-on-a-Chip is free software: you can redistribute it and/or modify
# it under the terms of the GNU LESSER GENERAL PUBLIC LICENSE Version 2.1.
#
# Python-on-a-Chip is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
# A copy of the GNU LESSER GENERAL PUBLIC LICENSE Version 2.1
# is seen in the file COPYING in this directory.

## @file
#  @copybrief avr

## @package avr
#  @brief AVR Access Module
#
# Provides generic access to the AVR microcontroller
#
# Note that to save RAM when the module is imported, many of the
# port & ddr methods below are commented out. Uncomment and recompile
# in order to use!
#
# <b>USAGE</b>
#
# \code
# import avr
# avr.ddrA(0) # Set all pins as input
# a = avr.portA()
# avr.ddrA(0xFF) # Set all pins as output
# avr.portA(42)
#
# avr.delay(500) # Half second pause
#
# if avr.digitalRead('A', 3):
#   avr.digitalWrite('D', 0, True)
# \endcode


"""__NATIVE__
#include <avr/io.h>
#include <util/delay.h>

/*
 * Common method for all port register operations
 */
PmReturn_t
_portX(volatile uint8_t *port,
       volatile uint8_t *direction,
       volatile uint8_t *pin)
{
   pPmObj_t pa;
   PmReturn_t retval = PM_RET_OK;

   switch (NATIVE_GET_NUM_ARGS())
   {
      /* If no argument is present, return PIN reg value */
      case 0:

        /* Read port and create a Python integer from its value */
        retval = int_new(*pin, &pa);

        /* Return the integer on the stack */
        NATIVE_SET_TOS(pa);
        break;

      /* If one argument is present, set port to that value */
      case 1:
         pa = NATIVE_GET_LOCAL(0);
         /* If the arg is not an integer, raise TypeError */
         if (OBJ_GET_TYPE(pa) != OBJ_TYPE_INT)
         {
           PM_RAISE(retval, PM_RET_EX_TYPE);
           break;
         }

         NATIVE_SET_TOS(PM_NONE);

         /* Set PORT to the low byte of the integer value */
         *port = ((pPmInt_t)pa)->val;
         break;

      /* If an invalid number of args are present, raise TypeError */
      default:
         PM_RAISE(retval, PM_RET_EX_TYPE);
         break;
    }

    return retval;
}


/*
 * Set a DDR register to the first Python argument
 */
PmReturn_t _ddrX(volatile uint8_t *direction)
{
   PmReturn_t retval = PM_RET_OK;
   pPmObj_t pa;
   if(NATIVE_GET_NUM_ARGS() != 1)
   {
      PM_RAISE(retval, PM_RET_EX_TYPE);
      return retval;
   }

   pa = NATIVE_GET_LOCAL(0);
   if (OBJ_GET_TYPE(pa) != OBJ_TYPE_INT)
   {
      PM_RAISE(retval, PM_RET_EX_TYPE);
      return retval;
   }

   *direction = (uint8_t) ((pPmInt_t)pa)->val;
   NATIVE_SET_TOS(PM_NONE);
   return retval;
}



/*
 * Loads the correct AVR port registers & direction address from the first
 * Python argument, and integer pin number (0-7) from second argument.
 * Port name argument is expected to be a single-character string with the port
 * letter ([a-dA-D])
 *
 * Both port_reg & port_reg arguments are optional.
 *
 * TODO: Look into putting this into a table in PROGMEM instead of a switch
 * statement
 */
PmReturn_t  _get_port_register(volatile uint8_t **pin_reg,
                               volatile uint8_t **port_reg,
                               volatile uint8_t **direction,
                               uint8_t *pin)
{
    pPmObj_t pa;
    pPmObj_t pb;
    PmReturn_t retval = PM_RET_OK;

    pa = NATIVE_GET_LOCAL(0);
    if (OBJ_GET_TYPE(pa) != OBJ_TYPE_STR)
    {
      PM_RAISE(retval, PM_RET_EX_TYPE);
      return retval;
    }

    pb = NATIVE_GET_LOCAL(1);
    if (OBJ_GET_TYPE(pb) != OBJ_TYPE_INT)
    {
      PM_RAISE(retval, PM_RET_EX_TYPE);
      return retval;
    }

    // Only single-character strings for the port number
    if ((((pPmString_t)pa)->length) != 1)
    {
      PM_RAISE(retval, PM_RET_EX_VAL);
      return retval;
    }

    // Find port & direction regs (TODO: Possibly make a PROGMEM lookup table)
    switch(((pPmString_t)pa)->val[0])
    {
      case 'a':
      case 'A':
        if(port_reg) *port_reg = &PORTA;
        if(pin_reg) *pin_reg = &PINA;
        *direction = &DDRA;
        break;
      case 'b':
      case 'B':
        if(port_reg) *port_reg = &PORTB;
        if(pin_reg) *pin_reg = &PINB;
        *direction = &DDRB;
        break;
      case 'c':
      case 'C':
#if defined(PORTC) && defined(PINC) && defined(DDRC)
        if(port_reg) *port_reg = &PORTC;
        if(pin_reg) *pin_reg = &PINC;
        *direction = &DDRC;
#endif
        break;
      case 'd':
      case 'D':
#if defined(PORTD) && defined(PIND) && defined(DDRD)
        if(port_reg) *port_reg = &PORTD;
        if(pin_reg) *pin_reg = &PIND;
        *direction = &DDRD;
#endif
        break;
      case 'e':
      case 'E':
#if defined(PORTE) && defined(PINE) && defined(DDRE)
        if(port_reg) *port_reg = &PORTE;
        if(pin_reg) *pin_reg = &PINE;
        *direction = &DDRE;
#endif
        break;
      case 'f':
      case 'F':
#if defined(PORTF) && defined(PINF) && defined(DDRF)
        if(port_reg) *port_reg = &PORTF;
        if(pin_reg) *pin_reg = &PINF;
        *direction = &DDRF;
#endif
        break;
      default:
        PM_RAISE(retval, PM_RET_EX_VAL);
        return retval;
    }

    // Check pin is in range
    if(((pPmInt_t)pb)->val < 0 || ((pPmInt_t)pb)->val > 7)
    {
        PM_RAISE(retval, PM_RET_EX_VAL);
        return retval;
    }
    *pin = ((pPmInt_t)pb)->val;

    return retval;
}

"""


# Port methods are commented out by default because of the amount of RAM
# used when the module is loaded. Uncomment the ones you need...

def portA(a):
    """__NATIVE__
    return _portX(&PORTA, &DDRA, &PINA);
    """
    pass

# def portB(a):
#     """__NATIVE__
#     return _portX(&PORTB, &DDRB, &PINB);
#     """
#     pass

# def portC(a):
#     """__NATIVE__
#     return _portX(&PORTC, &DDRC, &PINC);
#     """
#     pass

# def portD(a):
#     """__NATIVE__
#     return _portX(&PORTD, &DDRD, &PIND);
#     """
#     pass

# def portE(a):
#     """__NATIVE__
#     return _portX(&PORTE, &DDRE, &PINE);
#     """
#     pass

# def portF(a):
#     """__NATIVE__
#     return _portX(&PORTF, &DDRF, &PINF);
#     """
#     pass

def ddrA(a):
    """__NATIVE__
    return _ddrX(&DDRA);
    """
    pass

# def ddrB(a):
#     """__NATIVE__
#     return _ddrX(&DDRB);
#     """
#     pass

# def ddrC(a):
#     """__NATIVE__
#     return _ddrX(&DDRC);
#     """
#     pass


# def ddrD(a):
#     """__NATIVE__
#     return _ddrX(&DDRD);
#     """
#     pass

# def ddrE(a):
#     """__NATIVE__
#     return _ddrX(&DDRE);
#     """
#     pass

# def ddrF(a):
#     """__NATIVE__
#     return _ddrX(&DDRF);
#     """
#     pass


# Reads a single pin of a particular AVR port
#
# Port is specified as a single-character string, A-F.
# Pin is specified as an integer, 0-7
#
# Return value is boolean True/False, can be treated as 1/0
def digitalRead(port, pin):
    """__NATIVE__
    volatile uint8_t *port;
    volatile uint8_t *direction;
    uint8_t pin;
    PmReturn_t retval = PM_RET_OK;

    if(NATIVE_GET_NUM_ARGS() != 2)
    {
      PM_RAISE(retval, PM_RET_EX_TYPE);
      return retval;
    }

    retval = _get_port_register(&port, NULL, &direction, &pin);
    if(retval != PM_RET_OK)
      return retval;

    *direction &= ~(1<<pin); // Set pin to input
    pPmObj_t pa = (*port & (1<<pin)) ? PM_TRUE : PM_FALSE;
    NATIVE_SET_TOS(pa); // Push our result object onto the stack
    return retval;
    """
    pass


# Writes a single pin of a particular AVR port
#
# Port is specified as a single-character string, A-F.
# Pin is specified as an integer, 0-7
# Value is either boolean True/False or Integer 0 or non-zero.
#
def digitalWrite(port, pin, value):
    """__NATIVE__
    volatile uint8_t *port;
    volatile uint8_t *direction;
    uint8_t pin;
    pPmObj_t pc;
    PmReturn_t retval = PM_RET_OK;

    NATIVE_SET_TOS(PM_NONE);

    if(NATIVE_GET_NUM_ARGS() != 3)
    {
      PM_RAISE(retval, PM_RET_EX_TYPE);
      return retval;
    }

    retval = _get_port_register(NULL, &port, &direction, &pin);
    if(retval != PM_RET_OK)
      return retval;

    pc = NATIVE_GET_LOCAL(2);

    /* If the arg is not an integer, raise TypeError */
    if (OBJ_GET_TYPE(pc) != OBJ_TYPE_INT && OBJ_GET_TYPE(pc) != OBJ_TYPE_BOOL)
    {
      PM_RAISE(retval, PM_RET_EX_TYPE);
      return retval;
    }

    *direction |= (1<<pin); // Set pin to output

    if(((pPmInt_t)pc)->val)
      *port |= 1<<pin;
    else
      *port &= ~(1<<pin);
    return retval;
    """
    pass


def delay(ms):
    """__NATIVE__
    PmReturn_t retval = PM_RET_OK;

    if(NATIVE_GET_NUM_ARGS() != 1)
    {
      PM_RAISE(retval, PM_RET_EX_TYPE);
      return retval;
    }

    pPmObj_t pa = NATIVE_GET_LOCAL(0);
    if (OBJ_GET_TYPE(pa) == OBJ_TYPE_INT)
    {
      _delay_ms((double) ((pPmInt_t)pa)->val);
    }
    else if (OBJ_GET_TYPE(pa) == OBJ_TYPE_FLT)
    {
      _delay_ms((double) ((pPmFloat_t)pa)->val);
    }
    else
    {
      PM_RAISE(retval, PM_RET_EX_TYPE);
    }

    NATIVE_SET_TOS(PM_NONE);
    return retval;
    """
    pass



# :mode=c:
