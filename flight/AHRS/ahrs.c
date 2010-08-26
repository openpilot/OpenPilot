/**
 ******************************************************************************
 * @addtogroup AHRS AHRS Control
 * @brief The AHRS Modules perform
 *
 * @{ 
 * @addtogroup AHRS_Main
 * @brief Main function which does the hardware dependent stuff
 * @{ 
 *
 *
 * @file       ahrs.c
 * @author     The OpenPilot Team, http://www.openpilot.org Copyright (C) 2010.
 * @brief      INSGPS Test Program
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
#include "insgps.h"
#include "CoordinateConversions.h"

/**
 * State of AHRS EKF
 * @arg AHRS_IDLE - waiting for data to be available for filtering
 * @arg AHRS_DATA_READY - Data ready for downsampling and processing
 * @arg AHRS_PROCESSING - Performing update on the available data
 */
enum {AHRS_IDLE, AHRS_DATA_READY, AHRS_PROCESSING} ahrs_state;
enum algorithms ahrs_algorithm;

/**
 * @addtogroup AHRS_ADC_Configuration ADC Configuration
 * @{
 * Functions to configure ADC and handle interrupts
 */
void AHRS_ADC_Config(int32_t ekf_rate, int32_t adc_oversample);
void AHRS_ADC_DMA_Handler(void);
void DMA1_Channel1_IRQHandler() __attribute__ ((alias ("AHRS_ADC_DMA_Handler")));
/**
 * @}
 */

// For debugging the raw sensors
//#define DUMP_RAW

/**
 * @addtogroup AHRS_Definitions 
 * @{
 */
// Currently analog acquistion hard coded at 480 Hz
#define ADC_OVERSAMPLE          12
#define EKF_RATE                ((float) 480 / ADC_OVERSAMPLE)
#define ADC_CONTINUOUS_CHANNELS PIOS_ADC_NUM_PINS
#define CORRECTION_COUNT        4

#define VDD            3.3    /* supply voltage for ADC */
#define FULL_RANGE     4096   /* 12 bit ADC */
#define ACCEL_RANGE    2      /* adjustable by FS input */
#define ACCEL_GRAVITY  9.81   /* m s^-1 */
#define ACCEL_SENSITIVITY    ( VDD / 5 )
#define ACCEL_SCALE    ( (VDD / FULL_RANGE) / ACCEL_SENSITIVITY * 2 / ACCEL_RANGE * ACCEL_GRAVITY )
#define ACCEL_OFFSET  -2048  

#define GYRO_SENSITIVITY   ( 2.0 / 1000 ) /* 2 mV / (deg s^-1) */
#define RAD_PER_DEGREE     ( M_PI / 180 )
#define GYRO_SCALE         ( (VDD / FULL_RANGE) / GYRO_SENSITIVITY * RAD_PER_DEGREE )
#define GYRO_OFFSET       -1675 /* From data sheet, zero accel output is 1.35 v */
/**
 * @}
 */

/**
 * @addtogroup AHRS_Local Local Variables 
 * @{
 */
struct mag_sensor {
  uint8_t id[4];
  struct {
    int16_t axis[3];
  } raw;
};

struct accel_sensor {
  struct {
    uint16_t x;
    uint16_t y;
    uint16_t z;
  } raw;
  struct {
    float x;
    float y;
    float z;
  } filtered;
};

struct gyro_sensor {
  struct {
    uint16_t x;
    uint16_t y;
    uint16_t z;
  } raw;
  struct {
    float x;
    float y;
    float z;
  } filtered;  
  struct {
    uint16_t xy;
    uint16_t z;
  } temp;
};

struct attitude_solution {
  struct {
    float q1;
    float q2;
    float q3;
    float q4;
  } quaternion;
};

struct altitude_sensor {
  float altitude;
  float updated;
};

struct gps_sensor {
  float NED[3];
  float heading;
  float groundspeed;
  float quality;
  float updated;
};

static struct mag_sensor        mag_data;
static struct accel_sensor      accel_data;
static struct gyro_sensor       gyro_data;
static struct altitude_sensor   altitude_data;
static struct gps_sensor        gps_data;
static struct attitude_solution attitude_data;

/**
 * @}
 */


/* Function Prototypes */
void process_spi_request(void);
void downsample_data(void);
void calibrate_sensors(void);
void converge_insgps();

/**
 * @addtogroup AHRS_Global_Data AHRS Global Data
 * @{
 * Public data.  Used by both EKF and the sender
 */
//! Accelerometer variance after filter from OP or calibrate_sensors
float accel_var[3] = {1,1,1};
//! Gyro variance after filter from OP or calibrate sensors
float gyro_var[3] = {1,1,1};
//! Accelerometer scale after calibration
float accel_scale[3] = {ACCEL_SCALE, ACCEL_SCALE, ACCEL_SCALE};
//! Gyro scale after calibration
float gyro_scale[3] = {GYRO_SCALE, GYRO_SCALE, GYRO_SCALE};
//! Magnetometer variance from OP or calibrate sensors
float mag_var[3] = {1,1,1};
//! Accelerometer bias from OP or calibrate sensors
int16_t accel_bias[3] = {ACCEL_OFFSET, ACCEL_OFFSET, ACCEL_OFFSET};
//! Gyroscope bias term from OP or calibrate sensors
int16_t gyro_bias[3] = {0,0,0};
//! Magnetometer bias (direction) from OP or calibrate sensors
int16_t mag_bias[3] = {0,0,0};
//! Filter coefficients used in decimation.  Limited order so filter can't run between samples
int16_t fir_coeffs[ADC_OVERSAMPLE+1];          
//! Raw buffer that DMA data is dumped into
int16_t raw_data_buffer[ADC_CONTINUOUS_CHANNELS * ADC_OVERSAMPLE * 2];    // Double buffer that DMA just used
//! Swapped by interrupt handler to achieve double buffering
int16_t * valid_data_buffer;      
//! Counts how many times the EKF wasn't idle when DMA handler called
uint32_t ekf_too_slow = 0;
//! Total number of data blocks converted
uint32_t total_conversion_blocks = 0;
//! Flag to indicate new GPS data available
uint8_t gps_updated = FALSE;
//! Home location in ECEF coordinates
double BaseECEF[3] = {0, 0, 0};
//! Rotation matrix from LLA to Rne
float Rne[3][3];
//! Indicates the communications are requesting a calibration
uint8_t calibration_pending = FALSE;

/**
 * @}
 */


/**
* @brief AHRS Main function
*/
int main()
{
  float gyro[3], accel[3], mag[3];
  float vel[3] = {0,0,0};
  uint32_t loop_ctr = 0;
  
  ahrs_algorithm = INSGPS_Algo;

  /* Brings up System using CMSIS functions, enables the LEDs. */
  PIOS_SYS_Init();
  
  /* Delay system */
  PIOS_DELAY_Init();
  
  /* Communication system */
  PIOS_COM_Init();
  
  /* ADC system */
  AHRS_ADC_Config(EKF_RATE, ADC_OVERSAMPLE);
  
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
 
  // Get 3 ID bytes
  strcpy ((char *)mag_data.id, "ZZZ");
  PIOS_HMC5843_ReadID(mag_data.id);
  
  /* SPI link to master */
  PIOS_SPI_Init();
  
  lfsm_init();
    
  ahrs_state = AHRS_IDLE;;
  /* Use simple averaging filter for now */
  for (int i = 0; i < ADC_OVERSAMPLE; i++)
    fir_coeffs[i] = 1;
  fir_coeffs[ADC_OVERSAMPLE] = ADC_OVERSAMPLE;
  
  if(ahrs_algorithm == INSGPS_Algo) {
    // compute a data point and initialize INS
    downsample_data();
    converge_insgps();
  }
   
#ifdef DUMP_RAW
    while(1) {
        int result;
        uint8_t sync[4] = {7,7,7,7};
        while( ahrs_state != AHRS_DATA_READY );
        ahrs_state = AHRS_PROCESSING;
        
        downsample_data();
        ahrs_state = AHRS_IDLE;;

        // Dump raw buffer
        result = PIOS_COM_SendBuffer(PIOS_COM_AUX, &sync[0], 4); // dump block number
        result += PIOS_COM_SendBuffer(PIOS_COM_AUX, (uint8_t *) &total_conversion_blocks, sizeof(total_conversion_blocks)); // dump block number
        result += PIOS_COM_SendBuffer(PIOS_COM_AUX, (uint8_t *) &valid_data_buffer[0], ADC_OVERSAMPLE * ADC_CONTINUOUS_CHANNELS * sizeof(valid_data_buffer[0]));
        if(result == 0)
            PIOS_LED_Off(LED1);
        else {
            PIOS_LED_On(LED1);
        }
    }
#endif
  
  /******************* Main EKF loop ****************************/
  while (1) {
    // Alive signal
    PIOS_LED_Toggle(LED1);
    
    loop_ctr ++;
    
    if(calibration_pending)
    {
      calibrate_sensors();
      calibration_pending = FALSE;
    }
    
    // Get magnetic readings
    PIOS_HMC5843_ReadMag(mag_data.raw.axis);
    
    // Delay for valid data
    while( ahrs_state != AHRS_DATA_READY );
    ahrs_state = AHRS_PROCESSING;

    downsample_data();
    
    /***************** SEND BACK SOME RAW DATA ************************/
    // Hacky - grab one sample from buffer to populate this.  Need to send back
    // all raw data if it's happening
    accel_data.raw.x = valid_data_buffer[0];
    accel_data.raw.y = valid_data_buffer[2];
    accel_data.raw.z = valid_data_buffer[4];
    
    gyro_data.raw.x = valid_data_buffer[1];
    gyro_data.raw.y = valid_data_buffer[3];
    gyro_data.raw.z = valid_data_buffer[5];
    
    gyro_data.temp.xy = valid_data_buffer[6];
    gyro_data.temp.z = valid_data_buffer[7];            
    
    if(ahrs_algorithm == INSGPS_Algo) 
    {
      /******************** INS ALGORITHM **************************/
      
      // format data for INS algo
      gyro[0] = gyro_data.filtered.x;
      gyro[1] = gyro_data.filtered.y;
      gyro[2] = -gyro_data.filtered.z;
      accel[0] = accel_data.filtered.x,
      accel[1] = accel_data.filtered.y,
      accel[2] = accel_data.filtered.z,
      
      // Note: The magnetometer driver returns registers X,Y,Z from the chip which are 
      // (left, backward, up).  Remapping to (forward, right, down).
      mag[0] = -mag_data.raw.axis[1];
      mag[1] = -mag_data.raw.axis[0];
      mag[2] = -mag_data.raw.axis[2];
            
      INSPrediction(gyro, accel, 1 / (float) EKF_RATE);
      
      if ( gps_data.updated ) 
      {
        // Compute velocity from Heading and groundspeed
        vel[0] = gps_data.groundspeed * cos(gps_data.heading * M_PI / 180);        
        vel[1] = gps_data.groundspeed * sin(gps_data.heading * M_PI / 180);
        vel[2] = 0;
        
        FullCorrection(mag, gps_data.NED, vel, altitude_data.altitude);        
        gps_data.updated = FALSE;
      }
      else
        MagCorrection(mag); 
      
      attitude_data.quaternion.q1 = Nav.q[0];
      attitude_data.quaternion.q2 = Nav.q[1];
      attitude_data.quaternion.q3 = Nav.q[2];
      attitude_data.quaternion.q4 = Nav.q[3];
    }
    else if( ahrs_algorithm == SIMPLE_Algo )
    {    
      float q[4];
      float rpy[3];
      /***************** SIMPLE ATTITUDE FROM NORTH AND ACCEL ************/
      /* Very simple computation of the heading and attitude from accel. */
      rpy[2] = atan2((mag_data.raw.axis[0]), (-1 * mag_data.raw.axis[1])) * 180 / M_PI;
      rpy[1] = atan2(accel_data.filtered.x, accel_data.filtered.z) * 180 / M_PI;
      rpy[0] = atan2(accel_data.filtered.y,accel_data.filtered.z) * 180 / M_PI;
      
      RPY2Quaternion(rpy,q);
      attitude_data.quaternion.q1 = q[0];
      attitude_data.quaternion.q2 = q[1];
      attitude_data.quaternion.q3 = q[2];
      attitude_data.quaternion.q4 = q[3];
    }
    
    ahrs_state = AHRS_IDLE;

    process_spi_request();
    
  }
  
  return 0;
}

/**
 * @brief Downsample the analog data
 * @return none
 *
 * Tried to make as much of the filtering fixed point when possible.  Need to account
 * for offset for each sample before the multiplication if filter not a boxcar.  Could
 * precompute fixed offset as sum[fir_coeffs[i]] * ACCEL_OFFSET.  Puts data into global
 * data structures @ref accel_data and @ref gyro_data.
 * 
 * The accel_data values are converted into a coordinate system where X is forwards along
 * the fuselage, Y is along right the wing, and Z is down.
 */
void downsample_data()
{
  int16_t temp;
  int32_t accel_raw[3], gyro_raw[3];  
  uint16_t i;
  
  // Get the Y data.  Third byte in.  Convert to m/s
  accel_raw[0] = 0;  
  for( i = 0; i < ADC_OVERSAMPLE; i++ )      
  {
    temp = ( valid_data_buffer[0 + i * ADC_CONTINUOUS_CHANNELS] + accel_bias[1] ) * fir_coeffs[i];
    accel_raw[0] += temp;
  }
  accel_data.filtered.y = (float) accel_raw[0] / (float) fir_coeffs[ADC_OVERSAMPLE] * accel_scale[1];

  // Get the X data which projects forward/backwards.  Fifth byte in.  Convert to m/s
  accel_raw[1] = 0;  
  for( i = 0; i < ADC_OVERSAMPLE; i++ )      
    accel_raw[1] += ( valid_data_buffer[2 + i * ADC_CONTINUOUS_CHANNELS] + accel_bias[0] ) * fir_coeffs[i];
  accel_data.filtered.x = (float) accel_raw[1] / (float) fir_coeffs[ADC_OVERSAMPLE]  * accel_scale[0];
  
  // Get the Z data.  Third byte in.  Convert to m/s
  accel_raw[2] = 0;  
  for( i = 0; i < ADC_OVERSAMPLE; i++ )      
    accel_raw[2] += ( valid_data_buffer[4 + i * ADC_CONTINUOUS_CHANNELS] + accel_bias[2] ) * fir_coeffs[i];
  accel_data.filtered.z = -(float) accel_raw[2] / (float) fir_coeffs[ADC_OVERSAMPLE]  * accel_scale[2];
  
  // Get the X gyro data.  Seventh byte in.  Convert to deg/s.
  gyro_raw[0] = 0;
  for( i = 0; i < ADC_OVERSAMPLE; i++ )
    gyro_raw[0] += ( valid_data_buffer[1 + i * ADC_CONTINUOUS_CHANNELS] + gyro_bias[0] ) * fir_coeffs[i];
  gyro_data.filtered.x  = (float) gyro_raw[0] / (float) fir_coeffs[ADC_OVERSAMPLE] * gyro_scale[0];
  
  // Get the Y gyro data.  Second byte in.  Convert to deg/s.
  gyro_raw[1] = 0;
  for( i = 0; i < ADC_OVERSAMPLE; i++ )
    gyro_raw[1] += ( valid_data_buffer[3 + i * ADC_CONTINUOUS_CHANNELS] + gyro_bias[1] ) * fir_coeffs[i];
  gyro_data.filtered.y = (float) gyro_raw[1] / (float) fir_coeffs[ADC_OVERSAMPLE] * gyro_scale[1];
  
  // Get the Z gyro data.  Fifth byte in.  Convert to deg/s.
  gyro_raw[2] = 0;
  for( i = 0; i < ADC_OVERSAMPLE; i++ )
    gyro_raw[2] += ( valid_data_buffer[5 + i * ADC_CONTINUOUS_CHANNELS] + gyro_bias[2] ) * fir_coeffs[i];
  gyro_data.filtered.z = (float) gyro_raw[2] / (float) fir_coeffs[ADC_OVERSAMPLE] * gyro_scale[2];
}

/**
 * @brief Assumes board is not moving computes biases and variances of sensors
 * @returns None
 * 
 * All data is stored in global structures.  This function should be called from OP when
 * aircraft is in stable state and then the data stored to SD card.
 */
void calibrate_sensors() {
  int i;
  int16_t mag_raw[3];
  // local biases for noise analysis
  float accel_bias[3], gyro_bias[3], mag_bias[3];

  // run few loops to get mean
  gyro_bias[0] = gyro_bias[1] = gyro_bias[2] = 0;
  accel_bias[0] = accel_bias[1] = accel_bias[2] = 0;
  mag_bias[0] = mag_bias[1] = mag_bias[2] = 0;
  for(i = 0; i < 50; i++) {
    while( ahrs_state != AHRS_DATA_READY );
    ahrs_state = AHRS_PROCESSING;
    downsample_data();
    gyro_bias[0] += gyro_data.filtered.x;
    gyro_bias[1] += gyro_data.filtered.y;
    gyro_bias[2] += gyro_data.filtered.z;
    accel_bias[0] += accel_data.filtered.x;
    accel_bias[1] += accel_data.filtered.y;
    accel_bias[2] += accel_data.filtered.z;
    PIOS_HMC5843_ReadMag(mag_raw);
    mag_bias[0] += mag_raw[0];
    mag_bias[1] += mag_raw[1];
    mag_bias[2] += mag_raw[2];
    
    ahrs_state = AHRS_IDLE;
    process_spi_request();
  }
  gyro_bias[0] /= i;
  gyro_bias[1] /= i;
  gyro_bias[2] /= i;
  accel_bias[0] /= i;
  accel_bias[1] /= i;
  accel_bias[2] /= i;
  mag_bias[0] /= i;
  mag_bias[1] /= i;
  mag_bias[2] /= i;
  
  // more iterations for variance
  accel_var[0] = accel_var[1] = accel_var[2] = 0;
  gyro_var[0] = gyro_var[1] = gyro_var[2] = 0;
  mag_var[0] = mag_var[1] = mag_var[2] = 0;
  for(i = 0; i < 500; i++) 
  {
    while( ahrs_state != AHRS_DATA_READY );
    ahrs_state = AHRS_PROCESSING;
    downsample_data();
    gyro_var[0] += (gyro_data.filtered.x - gyro_bias[0]) * (gyro_data.filtered.x - gyro_bias[0]);
    gyro_var[1] += (gyro_data.filtered.y - gyro_bias[1]) * (gyro_data.filtered.y - gyro_bias[1]);
    gyro_var[2] += (gyro_data.filtered.z - gyro_bias[2]) * (gyro_data.filtered.z - gyro_bias[2]);
    accel_var[0] += (accel_data.filtered.x - accel_bias[0]) * (accel_data.filtered.x - accel_bias[0]);
    accel_var[1] += (accel_data.filtered.y - accel_bias[1]) * (accel_data.filtered.y - accel_bias[1]);
    accel_var[2] += (accel_data.filtered.z - accel_bias[2]) * (accel_data.filtered.z - accel_bias[2]);
    PIOS_HMC5843_ReadMag(mag_raw);
    mag_var[0] += (mag_raw[0] - mag_bias[0]) * (mag_raw[0] - mag_bias[0]);
    mag_var[1] += (mag_raw[1] - mag_bias[1]) * (mag_raw[1] - mag_bias[1]);
    mag_var[2] += (mag_raw[2] - mag_bias[2]) * (mag_raw[2] - mag_bias[2]);
    ahrs_state = AHRS_IDLE;    
    process_spi_request();
  }
  gyro_var[0] /= i;
  gyro_var[1] /= i;
  gyro_var[2] /= i;
  accel_var[0] /= i;
  accel_var[1] /= i;
  accel_var[2] /= i;
  mag_var[0] /= i;
  mag_var[1] /= i;
  mag_var[2] /= i;   

  float mag_length2 = mag_bias[0] * mag_bias[0] + mag_bias[1] * mag_bias[1] + mag_bias[2] * mag_bias[2];
  mag_var[0] = mag_var[0] / mag_length2;
  mag_var[1] = mag_var[1] / mag_length2;
  mag_var[2] = mag_var[2] / mag_length2;
    
  if(ahrs_algorithm == INSGPS_Algo) 
    converge_insgps();
}

/**
 * @brief Quickly initialize INS assuming stationary and gravity is down
 *
 * Currently this is done iteratively but I'm sure it can be directly computed
 * when I sit down and work it out
 */
void converge_insgps()
{
  float pos[3] = {0,0,0}, vel[3] = {0,0,0}, BaroAlt = 0, mag[3], accel[3], temp_gyro[3] = {0, 0, 0};
  INSGPSInit();
  INSSetAccelVar(accel_var);
  INSSetGyroBias(temp_gyro); // set this to zero - crude bias corrected from downsample_data
  INSSetGyroVar(gyro_var);
  INSSetMagVar(mag_var);         
    
  float temp_var[3] = {10, 10, 10};
  INSSetGyroVar(temp_var); // ignore gyro's

  accel[0] = accel_data.filtered.x;
  accel[1] = accel_data.filtered.y;
  accel[2] = accel_data.filtered.z;
  
  // Iteratively constrain pitch and roll while updating yaw to align magnetic axis.  
  for(int i = 0; i < 50; i++) 
  {
    // This should be done directly but I'm too dumb.
    float rpy[3];
    Quaternion2RPY(Nav.q, rpy);
    rpy[1] = -atan2(accel_data.filtered.x, accel_data.filtered.z) * 180 / M_PI;
    rpy[0] = -atan2(accel_data.filtered.y, accel_data.filtered.z) * 180 / M_PI;    
    // Get magnetic readings
    PIOS_HMC5843_ReadMag(mag_data.raw.axis);
    mag[0] = -mag_data.raw.axis[1];
    mag[1] = -mag_data.raw.axis[0];
    mag[2] = -mag_data.raw.axis[2];
 
    RPY2Quaternion(rpy,Nav.q);
    INSPrediction( temp_gyro, accel, 1 / (float) EKF_RATE );
    FullCorrection(mag,pos,vel,BaroAlt); 
    process_spi_request(); // again we must keep this hear to prevent SPI connection dropping
  } 
  
  INSSetGyroVar(gyro_var);

}

/**
 * @addtogroup AHRS_SPI SPI Messaging 
 * @{
 * @brief SPI protocol handling requests for data from OP mainboard
 */

/**
 * @brief What is this meant to do
 * @param[in] port
 * @param[in] prefix
 * @param[in] data
 * @param[in] len
 * @return None
 */
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
    case OPAHRS_MSG_V1_REQ_ALGORITHM:
      opahrs_msg_v1_init_user_tx (&user_tx_v1, OPAHRS_MSG_V1_RSP_ALGORITHM);
      ahrs_algorithm = user_rx_v1.payload.user.v.req.algorithm.algorithm;
      dump_spi_message(PIOS_COM_AUX, "A", (uint8_t *)&user_rx_v1, sizeof(user_rx_v1));
      lfsm_user_set_tx_v1 (&user_tx_v1);
      break;
    case OPAHRS_MSG_V1_REQ_NORTH:
      opahrs_msg_v1_init_user_tx (&user_tx_v1, OPAHRS_MSG_V1_RSP_NORTH);
      INSSetMagNorth(user_rx_v1.payload.user.v.req.north.Be);
      dump_spi_message(PIOS_COM_AUX, "N", (uint8_t *)&user_rx_v1, sizeof(user_rx_v1));
      lfsm_user_set_tx_v1 (&user_tx_v1);
      break;
    case OPAHRS_MSG_V1_REQ_CALIBRATION:
      if(user_rx_v1.payload.user.v.req.calibration.measure_var)
        calibration_pending = TRUE;
      else {
        accel_var[0] = user_rx_v1.payload.user.v.req.calibration.accel_var[0];
        accel_var[1] = user_rx_v1.payload.user.v.req.calibration.accel_var[1];
        accel_var[2] = user_rx_v1.payload.user.v.req.calibration.accel_var[2];
        gyro_bias[0] = user_rx_v1.payload.user.v.req.calibration.gyro_bias[0];
        gyro_bias[1] = user_rx_v1.payload.user.v.req.calibration.gyro_bias[1];
        gyro_bias[2] = user_rx_v1.payload.user.v.req.calibration.gyro_bias[2];
        gyro_var[0] = user_rx_v1.payload.user.v.req.calibration.gyro_var[0];
        gyro_var[1] = user_rx_v1.payload.user.v.req.calibration.gyro_var[1];
        gyro_var[2] = user_rx_v1.payload.user.v.req.calibration.gyro_var[2];
        mag_var[0] = user_rx_v1.payload.user.v.req.calibration.mag_var[0];
        mag_var[1] = user_rx_v1.payload.user.v.req.calibration.mag_var[1];
        mag_var[2] = user_rx_v1.payload.user.v.req.calibration.mag_var[2];      
        INSSetAccelVar(accel_var);
        float gyro_bias_ins[3] = {0,0,0};
        INSSetGyroBias(gyro_bias_ins);  //gyro bias corrects in preprocessing
        INSSetGyroVar(gyro_var);
        INSSetMagVar(mag_var);             
      }      
      accel_bias[0] = user_rx_v1.payload.user.v.req.calibration.accel_bias[0];
      accel_bias[1] = user_rx_v1.payload.user.v.req.calibration.accel_bias[1];
      accel_bias[2] = user_rx_v1.payload.user.v.req.calibration.accel_bias[2];
      accel_scale[0] = user_rx_v1.payload.user.v.req.calibration.accel_scale[0];
      accel_scale[1] = user_rx_v1.payload.user.v.req.calibration.accel_scale[1];
      accel_scale[2] = user_rx_v1.payload.user.v.req.calibration.accel_scale[2];
      gyro_scale[0] = user_rx_v1.payload.user.v.req.calibration.gyro_scale[0];
      gyro_scale[1] = user_rx_v1.payload.user.v.req.calibration.gyro_scale[1];
      gyro_scale[2] = user_rx_v1.payload.user.v.req.calibration.gyro_scale[2];
      mag_bias[0] = user_rx_v1.payload.user.v.req.calibration.mag_bias[0];
      mag_bias[1] = user_rx_v1.payload.user.v.req.calibration.mag_bias[1];
      mag_bias[2] = user_rx_v1.payload.user.v.req.calibration.mag_bias[2];
      
      // echo back the values used
      opahrs_msg_v1_init_user_tx (&user_tx_v1, OPAHRS_MSG_V1_RSP_CALIBRATION);    
      user_tx_v1.payload.user.v.rsp.calibration.accel_var[0] = accel_var[0];
      user_tx_v1.payload.user.v.rsp.calibration.accel_var[1] = accel_var[1];
      user_tx_v1.payload.user.v.rsp.calibration.accel_var[2] = accel_var[2];
      user_tx_v1.payload.user.v.rsp.calibration.gyro_bias[0] = gyro_bias[0];
      user_tx_v1.payload.user.v.rsp.calibration.gyro_bias[1] = gyro_bias[1];
      user_tx_v1.payload.user.v.rsp.calibration.gyro_bias[2] = gyro_bias[2];
      user_tx_v1.payload.user.v.rsp.calibration.gyro_var[0] = gyro_var[0];
      user_tx_v1.payload.user.v.rsp.calibration.gyro_var[1] = gyro_var[1];
      user_tx_v1.payload.user.v.rsp.calibration.gyro_var[2] = gyro_var[2];
      user_tx_v1.payload.user.v.rsp.calibration.mag_var[0] = mag_var[0];
      user_tx_v1.payload.user.v.rsp.calibration.mag_var[1] = mag_var[1];
      user_tx_v1.payload.user.v.rsp.calibration.mag_var[2] = mag_var[2];
            
      dump_spi_message(PIOS_COM_AUX, "C", (uint8_t *)&user_rx_v1, sizeof(user_rx_v1));
      lfsm_user_set_tx_v1 (&user_tx_v1);
      break;      
    case OPAHRS_MSG_V1_REQ_ATTITUDERAW:
      opahrs_msg_v1_init_user_tx (&user_tx_v1, OPAHRS_MSG_V1_RSP_ATTITUDERAW);
      user_tx_v1.payload.user.v.rsp.attituderaw.mags.x        = mag_data.raw.axis[0];
      user_tx_v1.payload.user.v.rsp.attituderaw.mags.y        = mag_data.raw.axis[1];
      user_tx_v1.payload.user.v.rsp.attituderaw.mags.z        = mag_data.raw.axis[2];
      
      user_tx_v1.payload.user.v.rsp.attituderaw.gyros.x       = gyro_data.raw.x;
      user_tx_v1.payload.user.v.rsp.attituderaw.gyros.y       = gyro_data.raw.y;
      user_tx_v1.payload.user.v.rsp.attituderaw.gyros.z       = gyro_data.raw.z;
      
      user_tx_v1.payload.user.v.rsp.attituderaw.gyros_filtered.x       = gyro_data.filtered.x;
      user_tx_v1.payload.user.v.rsp.attituderaw.gyros_filtered.y       = gyro_data.filtered.y;
      user_tx_v1.payload.user.v.rsp.attituderaw.gyros_filtered.z       = gyro_data.filtered.z;
      
      user_tx_v1.payload.user.v.rsp.attituderaw.gyros.xy_temp = gyro_data.temp.xy;
      user_tx_v1.payload.user.v.rsp.attituderaw.gyros.z_temp  = gyro_data.temp.z;
      
      user_tx_v1.payload.user.v.rsp.attituderaw.accels.x      = accel_data.raw.x;
      user_tx_v1.payload.user.v.rsp.attituderaw.accels.y      = accel_data.raw.y;
      user_tx_v1.payload.user.v.rsp.attituderaw.accels.z      = accel_data.raw.z;
      
      user_tx_v1.payload.user.v.rsp.attituderaw.accels_filtered.x      = accel_data.filtered.x;
      user_tx_v1.payload.user.v.rsp.attituderaw.accels_filtered.y      = accel_data.filtered.y;
      user_tx_v1.payload.user.v.rsp.attituderaw.accels_filtered.z      = accel_data.filtered.z;
      
      dump_spi_message(PIOS_COM_AUX, "R", (uint8_t *)&user_tx_v1, sizeof(user_tx_v1));
      lfsm_user_set_tx_v1 (&user_tx_v1);
      break;
    case OPAHRS_MSG_V1_REQ_UPDATE:
      // process incoming data
      opahrs_msg_v1_init_user_tx (&user_tx_v1, OPAHRS_MSG_V1_RSP_UPDATE);
      if(user_rx_v1.payload.user.v.req.update.barometer.updated)
      {
        altitude_data.altitude    = user_rx_v1.payload.user.v.req.update.barometer.altitude;
        altitude_data.updated     = user_rx_v1.payload.user.v.req.update.barometer.updated;
      }
      if(user_rx_v1.payload.user.v.req.update.gps.updated)
      {
        gps_data.updated = TRUE;
        gps_data.NED[0] = user_rx_v1.payload.user.v.req.update.gps.NED[0];
        gps_data.NED[1] = user_rx_v1.payload.user.v.req.update.gps.NED[1];
        gps_data.NED[2] = user_rx_v1.payload.user.v.req.update.gps.NED[2];
        gps_data.heading = user_rx_v1.payload.user.v.req.update.gps.heading;
        gps_data.groundspeed = user_rx_v1.payload.user.v.req.update.gps.groundspeed;
        gps_data.quality = user_rx_v1.payload.user.v.req.update.gps.quality;
      }
      
      // send out attitude/position estimate
      user_tx_v1.payload.user.v.rsp.update.quaternion.q1 = attitude_data.quaternion.q1;
      user_tx_v1.payload.user.v.rsp.update.quaternion.q2 = attitude_data.quaternion.q2;
      user_tx_v1.payload.user.v.rsp.update.quaternion.q3 = attitude_data.quaternion.q3;
      user_tx_v1.payload.user.v.rsp.update.quaternion.q4 = attitude_data.quaternion.q4;
      
      // TODO: separate this from INSGPS
      user_tx_v1.payload.user.v.rsp.update.NED[0] = Nav.Pos[0];
      user_tx_v1.payload.user.v.rsp.update.NED[1] = Nav.Pos[1];
      user_tx_v1.payload.user.v.rsp.update.NED[2] = Nav.Pos[2];
      user_tx_v1.payload.user.v.rsp.update.Vel[0] = Nav.Vel[0];
      user_tx_v1.payload.user.v.rsp.update.Vel[1] = Nav.Vel[1];
      user_tx_v1.payload.user.v.rsp.update.Vel[2] = Nav.Vel[2];

      user_tx_v1.payload.user.v.rsp.update.quaternion.q2 = attitude_data.quaternion.q2;
      user_tx_v1.payload.user.v.rsp.update.quaternion.q3 = attitude_data.quaternion.q3;
      user_tx_v1.payload.user.v.rsp.update.quaternion.q4 = attitude_data.quaternion.q4;

      dump_spi_message(PIOS_COM_AUX, "U", (uint8_t *)&user_tx_v1, sizeof(user_tx_v1));
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
/**
 * @}
 */

/* Local Variables */
static GPIO_TypeDef* ADC_GPIO_PORT[PIOS_ADC_NUM_PINS] = PIOS_ADC_PORTS;
static const uint32_t ADC_GPIO_PIN[PIOS_ADC_NUM_PINS] = PIOS_ADC_PINS;
static const uint32_t ADC_CHANNEL[PIOS_ADC_NUM_PINS] = PIOS_ADC_CHANNELS;

static ADC_TypeDef* ADC_MAPPING[PIOS_ADC_NUM_PINS] = PIOS_ADC_MAPPING;
static const uint32_t ADC_CHANNEL_MAPPING[PIOS_ADC_NUM_PINS] = PIOS_ADC_CHANNEL_MAPPING;


/**
 * @brief Initialise the ADC Peripheral
 * @param[in] ekf_rate
 * @param[in] adc_oversample
 * 
 * Currently ignores rates and uses hardcoded values.  Need a little logic to
 * map from sampling rates and such to ADC constants.
 */
void AHRS_ADC_Config(int32_t ekf_rate, int32_t adc_oversample)
{
  
	int32_t i;
  
	/* Setup analog pins */
	GPIO_InitTypeDef GPIO_InitStructure;
	GPIO_StructInit(&GPIO_InitStructure);
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_2MHz;
	GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_AIN;
  
	/* Enable each ADC pin in the array */
	for(i = 0; i < PIOS_ADC_NUM_PINS; i++) {
		GPIO_InitStructure.GPIO_Pin = ADC_GPIO_PIN[i];
		GPIO_Init(ADC_GPIO_PORT[i], &GPIO_InitStructure);
	}
  
	/* Enable ADC clocks */
	PIOS_ADC_CLOCK_FUNCTION;
  
	/* Map channels to conversion slots depending on the channel selection mask */
	for(i = 0; i < PIOS_ADC_NUM_PINS; i++) {
		ADC_RegularChannelConfig(ADC_MAPPING[i], ADC_CHANNEL[i], ADC_CHANNEL_MAPPING[i], PIOS_ADC_SAMPLE_TIME);
	}
  
#if (PIOS_ADC_USE_TEMP_SENSOR)
	ADC_TempSensorVrefintCmd(ENABLE);
	ADC_RegularChannelConfig(PIOS_ADC_TEMP_SENSOR_ADC, ADC_Channel_14, PIOS_ADC_TEMP_SENSOR_ADC_CHANNEL, PIOS_ADC_SAMPLE_TIME);
#endif
  
  // TODO: update ADC to continuous sampling, configure the sampling rate
	/* Configure ADCs */
	ADC_InitTypeDef ADC_InitStructure;
	ADC_StructInit(&ADC_InitStructure);
	ADC_InitStructure.ADC_Mode = ADC_Mode_RegSimult;
	ADC_InitStructure.ADC_ScanConvMode = ENABLE;
	ADC_InitStructure.ADC_ContinuousConvMode = ENABLE;
	ADC_InitStructure.ADC_ExternalTrigConv = ADC_ExternalTrigConv_None;
	ADC_InitStructure.ADC_DataAlign = ADC_DataAlign_Right;
	ADC_InitStructure.ADC_NbrOfChannel = 4; //((PIOS_ADC_NUM_CHANNELS + 1) >> 1);
	ADC_Init(ADC1, &ADC_InitStructure);
  
#if (PIOS_ADC_USE_ADC2)
	ADC_Init(ADC2, &ADC_InitStructure);
  
	/* Enable ADC2 external trigger conversion (to synch with ADC1) */
	ADC_ExternalTrigConvCmd(ADC2, ENABLE);
#endif
  
	RCC_ADCCLKConfig(PIOS_ADC_ADCCLK);
  RCC_PCLK2Config(RCC_HCLK_Div16);
  
	/* Enable ADC1->DMA request */
	ADC_DMACmd(ADC1, ENABLE);
  
	/* ADC1 calibration */
	ADC_Cmd(ADC1, ENABLE);
	ADC_ResetCalibration(ADC1);
	while(ADC_GetResetCalibrationStatus(ADC1));
	ADC_StartCalibration(ADC1);
	while(ADC_GetCalibrationStatus(ADC1));
  
#if (PIOS_ADC_USE_ADC2)
	/* ADC2 calibration */
	ADC_Cmd(ADC2, ENABLE);
	ADC_ResetCalibration(ADC2);
	while(ADC_GetResetCalibrationStatus(ADC2));
	ADC_StartCalibration(ADC2);
	while(ADC_GetCalibrationStatus(ADC2));
#endif
  
	/* Enable DMA1 clock */
	RCC_AHBPeriphClockCmd(RCC_AHBPeriph_DMA1, ENABLE);
  
	/* Configure DMA1 channel 1 to fetch data from ADC result register */
	DMA_InitTypeDef DMA_InitStructure;
	DMA_StructInit(&DMA_InitStructure);
	DMA_DeInit(DMA1_Channel1);
	DMA_InitStructure.DMA_PeripheralBaseAddr = (uint32_t)&ADC1->DR;
	DMA_InitStructure.DMA_MemoryBaseAddr = (uint32_t)&raw_data_buffer[0];
	DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralSRC;
  /* We are double buffering half words from the ADC.  Make buffer appropriately sized */
	DMA_InitStructure.DMA_BufferSize = (ADC_CONTINUOUS_CHANNELS * ADC_OVERSAMPLE * 2) >> 1; 
	DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;
	DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;
  /* Note: We read ADC1 and ADC2 in parallel making a word read, also hence the half buffer size */
	DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_Word;
	DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_Word;
	DMA_InitStructure.DMA_Mode = DMA_Mode_Circular;
	DMA_InitStructure.DMA_Priority = DMA_Priority_High;
	DMA_InitStructure.DMA_M2M = DMA_M2M_Disable;
	DMA_Init(DMA1_Channel1, &DMA_InitStructure);
	DMA_Cmd(DMA1_Channel1, ENABLE);
  
	/* Trigger interrupt when for half conversions too to indicate double buffer */
	DMA_ITConfig(DMA1_Channel1, DMA_IT_TC, ENABLE);
	DMA_ITConfig(DMA1_Channel1, DMA_IT_HT, ENABLE);
  
	/* Configure and enable DMA interrupt */
	NVIC_InitTypeDef NVIC_InitStructure;
	NVIC_InitStructure.NVIC_IRQChannel = DMA1_Channel1_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = PIOS_ADC_IRQ_PRIO;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);
  
	/* Finally start initial conversion */
	ADC_SoftwareStartConvCmd(ADC1, ENABLE);
}

/**
 * @brief Interrupt for half and full buffer transfer
 * 
 * This interrupt handler swaps between the two halfs of the double buffer to make
 * sure the ahrs uses the most recent data.  Only swaps data when AHRS is idle, but
 * really this is a pretense of a sanity check since the DMA engine is consantly 
 * running in the background.  Keep an eye on the ekf_too_slow variable to make sure
 * it's keeping up.
 */
void AHRS_ADC_DMA_Handler(void) 
{
  if ( ahrs_state == AHRS_IDLE ) 
  {
    // Ideally this would have a mutex, but I think we can avoid it (and don't have RTOS features)
    
    if( DMA_GetFlagStatus( DMA1_IT_TC1 ) ) // whole double buffer filled
      valid_data_buffer = &raw_data_buffer[ 1 * ADC_CONTINUOUS_CHANNELS * ADC_OVERSAMPLE ];
    else if ( DMA_GetFlagStatus(DMA1_IT_HT1) )
      valid_data_buffer = &raw_data_buffer[ 0 * ADC_CONTINUOUS_CHANNELS * ADC_OVERSAMPLE ];
    else {
      // lets cause a seg fault and catch whatever is going on
      valid_data_buffer = 0;
    }
    
    ahrs_state = AHRS_DATA_READY;
  }
  else {
    // Track how many times an interrupt occurred before EKF finished processing
    ekf_too_slow++;
  }
  
  total_conversion_blocks++;
  
  // Clear all interrupt from DMA 1 - regardless if buffer swapped
  DMA_ClearFlag( DMA1_IT_GL1 );
  
}
