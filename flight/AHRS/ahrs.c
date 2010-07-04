/**
 ******************************************************************************
 *
 * @file       ahrs.c
 * @author     The OpenPilot Team, http://www.openpilot.org Copyright (C) 2010.
 * @brief      Main AHRS functions
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


/* OpenPilot Includes */
#include "ahrs.h"
#include "pios_opahrs_proto.h"
#include "ahrs_fsm.h"		/* lfsm_state */

/* Global Variables */

/* Local Variables */
struct mag_sensor {
  uint8_t id[4];
  struct {
    int16_t axis[3];
    float   heading;
  } raw;
};

struct accel_sensor {
  struct {
    uint16_t x;
    uint16_t y;
    uint16_t z;
  } raw;
};

struct attitude_solution {
  struct {
    float q1;
    float q2;
    float q3;
    float q4;
  } quaternion;

  struct {
    float roll;
    float pitch;
    float yaw;
  } euler;
};

static struct mag_sensor        mag_data;
static struct accel_sensor      accel_data;
static struct attitude_solution attitude_data = {
  .quaternion = {
    .q1 = 1.011,
    .q2 = 2.022,
    .q3 = 3.033,
    .q4 = 0,
  },
  .euler = {
    .roll  = 4.044,
    .pitch = 5.055,
    .yaw   = 6.066,
  },
};

/* Function Prototypes */
void process_spi_request(void);

/**
* AHRS Main function
*/
int main()
{
  /* Brings up System using CMSIS functions, enables the LEDs. */
  PIOS_SYS_Init();
  
  /* Delay system */
  PIOS_DELAY_Init();
  
  /* Communication system */
  PIOS_COM_Init();
  
  /* ADC system */
  PIOS_ADC_Init();
  
  /* Magnetic sensor system */
  PIOS_I2C_Init();
  PIOS_HMC5843_Init();
  
  /* Setup the Accelerometer FS (Full-Scale) GPIO */
  PIOS_GPIO_Enable(0);
  SET_ACCEL_2G;
  
  /* Configure the HMC5843 Sensor */
  PIOS_HMC5843_ConfigTypeDef HMC5843_InitStructure;
  HMC5843_InitStructure.M_ODR = PIOS_HMC5843_ODR_10;
  HMC5843_InitStructure.Meas_Conf = PIOS_HMC5843_MEASCONF_NORMAL;
  HMC5843_InitStructure.Gain = PIOS_HMC5843_GAIN_2;
  HMC5843_InitStructure.Mode = PIOS_HMC5843_MODE_CONTINUOUS;
  PIOS_HMC5843_Config(&HMC5843_InitStructure);
  
  /* SPI link to master */
  PIOS_SPI_Init();
  
  lfsm_init();
  
  // Main loop
  while (1) {
    uint8_t loop_ctr;

    // Alive signal
    if (loop_ctr++ > 100) {
      PIOS_LED_Toggle(LED1);
      loop_ctr = 0;
    }
    
    // Get 3 ID bytes
    strcpy ((char *)mag_data.id, "ZZZ");
    PIOS_HMC5843_ReadID(mag_data.id);
    
    // Get magnetic readings
    PIOS_HMC5843_ReadMag(mag_data.raw.axis);
    
    // Calculate the heading
    mag_data.raw.heading = atan2((double)(mag_data.raw.axis[0]), (double)(-1 * mag_data.raw.axis[1])) * (180 / M_PI);
    if(mag_data.raw.heading < 0) mag_data.raw.heading += 360;
    
    // Test ADC
    accel_data.raw.x = PIOS_ADC_PinGet(0);
    accel_data.raw.y = PIOS_ADC_PinGet(1);
    accel_data.raw.z = PIOS_ADC_PinGet(2);
    //PIOS_COM_SendFormattedString(PIOS_COM_AUX, "ADC Values: %d,%d,%d,%d,%d,%d\r\n", PIOS_ADC_PinGet(0), PIOS_ADC_PinGet(1), PIOS_ADC_PinGet(2), PIOS_ADC_PinGet(3), PIOS_ADC_PinGet(4), PIOS_ADC_PinGet(5));


    /* Simulate a rotating airframe */
    attitude_data.quaternion.q1 += .001;
    attitude_data.quaternion.q2 += .002;
    attitude_data.quaternion.q3 += .003;
    attitude_data.quaternion.q4 += 1;

    attitude_data.euler.roll  += .004;
    if (attitude_data.euler.roll > 360.0) attitude_data.euler.roll -= 360.0;
    attitude_data.euler.pitch += .005;
    if (attitude_data.euler.pitch > 360.0) attitude_data.euler.pitch -= 360.0;
    attitude_data.euler.yaw   += .006;
    if (attitude_data.euler.yaw > 360.0) attitude_data.euler.yaw -= 360.0;
    
    process_spi_request();
    
    // Delay until next reading
    //PIOS_DELAY_WaitmS(50);
  }
  
  return 0;
}

void dump_spi_message(uint8_t port, const char * prefix, uint8_t * data, uint32_t len)
{




}


static struct opahrs_msg_v1   link_tx_v1;
static struct opahrs_msg_v1   link_rx_v1;
static struct opahrs_msg_v1   user_rx_v1;
static struct opahrs_msg_v1   user_tx_v1;

void process_spi_request(void)
{
  bool msg_to_process = FALSE;

  PIOS_IRQ_Disable();
  /* Figure out if we're in an interesting stable state */
  switch (lfsm_get_state()) {
  case LFSM_STATE_USER_BUSY:
    msg_to_process = TRUE;
    break;
  case LFSM_STATE_INACTIVE:
    /* Queue up a receive buffer */
    lfsm_user_set_rx_v1 (&user_rx_v1);
    lfsm_user_done ();
    break;
  case LFSM_STATE_STOPPED:
    /* Get things going */
    lfsm_set_link_proto_v1 (&link_tx_v1, &link_rx_v1);
    break;
  default:
    /* Not a stable state */
    break;
  }
  PIOS_IRQ_Enable();

  if (!msg_to_process) {
    /* Nothing to do */
    //PIOS_COM_SendFormattedString(PIOS_COM_AUX, ".");
    return;
  }

  if (user_rx_v1.tail.magic != OPAHRS_MSG_MAGIC_TAIL) {
    PIOS_COM_SendFormattedString(PIOS_COM_AUX, "x");
  }

  /* We've got a message to process */
  //dump_spi_message(PIOS_COM_AUX, "+", (uint8_t *)&user_rx_v1, sizeof(user_rx_v1));

  switch (user_rx_v1.payload.user.t) {
  case OPAHRS_MSG_V1_REQ_SYNC:
    opahrs_msg_v1_init_user_tx (&user_tx_v1, OPAHRS_MSG_V1_RSP_SYNC);
    user_tx_v1.payload.user.v.rsp.sync.i_am_a_bootloader = FALSE;
    user_tx_v1.payload.user.v.rsp.sync.hw_version = 1;
    user_tx_v1.payload.user.v.rsp.sync.bl_version = 2;
    user_tx_v1.payload.user.v.rsp.sync.fw_version = 3;
    user_tx_v1.payload.user.v.rsp.sync.cookie     = user_rx_v1.payload.user.v.req.sync.cookie;
    dump_spi_message(PIOS_COM_AUX, "S", (uint8_t *)&user_tx_v1, sizeof(user_tx_v1));
    lfsm_user_set_tx_v1 (&user_tx_v1);
    break;
  case OPAHRS_MSG_V1_REQ_RESET:
    PIOS_DELAY_WaitmS(user_rx_v1.payload.user.v.req.reset.reset_delay_in_ms);
    PIOS_SYS_Reset();
    break;
  case OPAHRS_MSG_V1_REQ_SERIAL:
    opahrs_msg_v1_init_user_tx (&user_tx_v1, OPAHRS_MSG_V1_RSP_SERIAL);
    PIOS_SYS_SerialNumberGet((char *)&(user_tx_v1.payload.user.v.rsp.serial.serial_bcd));
    dump_spi_message(PIOS_COM_AUX, "I", (uint8_t *)&user_tx_v1, sizeof(user_tx_v1));
    lfsm_user_set_tx_v1 (&user_tx_v1);
    break;
  case OPAHRS_MSG_V1_REQ_HEADING:
    opahrs_msg_v1_init_user_tx (&user_tx_v1, OPAHRS_MSG_V1_RSP_HEADING);
    user_tx_v1.payload.user.v.rsp.heading.raw_mag.x     = mag_data.raw.axis[0];
    user_tx_v1.payload.user.v.rsp.heading.raw_mag.y     = mag_data.raw.axis[1];
    user_tx_v1.payload.user.v.rsp.heading.raw_mag.z     = mag_data.raw.axis[2];
    user_tx_v1.payload.user.v.rsp.heading.heading       = mag_data.raw.heading;
    dump_spi_message(PIOS_COM_AUX, "H", (uint8_t *)&user_tx_v1, sizeof(user_tx_v1));
    lfsm_user_set_tx_v1 (&user_tx_v1);
    break;
  case OPAHRS_MSG_V1_REQ_ATTITUDE:
    opahrs_msg_v1_init_user_tx (&user_tx_v1, OPAHRS_MSG_V1_RSP_ATTITUDE);
    user_tx_v1.payload.user.v.rsp.attitude.quaternion.q1 = attitude_data.quaternion.q1;
    user_tx_v1.payload.user.v.rsp.attitude.quaternion.q2 = attitude_data.quaternion.q2;
    user_tx_v1.payload.user.v.rsp.attitude.quaternion.q3 = attitude_data.quaternion.q3;
    user_tx_v1.payload.user.v.rsp.attitude.quaternion.q4 = attitude_data.quaternion.q4;
    user_tx_v1.payload.user.v.rsp.attitude.euler.roll    = attitude_data.euler.roll;
    user_tx_v1.payload.user.v.rsp.attitude.euler.pitch   = attitude_data.euler.pitch;
    user_tx_v1.payload.user.v.rsp.attitude.euler.yaw     = attitude_data.euler.yaw;
    dump_spi_message(PIOS_COM_AUX, "A", (uint8_t *)&user_tx_v1, sizeof(user_tx_v1));
#if 1
    /* DEBUG: Overload q4 as a cycle counter since last read. */
    attitude_data.quaternion.q4 = 0;
#endif
    lfsm_user_set_tx_v1 (&user_tx_v1);
    break;
  default:
    break;
  }

  /* Finished processing the received message, requeue it */
  memset(&user_rx_v1, 0xAA, sizeof(user_rx_v1));
  lfsm_user_set_rx_v1 (&user_rx_v1);
  lfsm_user_done ();
}

