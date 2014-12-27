/**
 ******************************************************************************
 * @addtogroup PIOS PIOS Core haoftware; you can rnedtt
 * @{
 * @addtogroup PIOS_MPU6000 MPU6000 Functions
 * @brief Deals with the hardware interface to the 3-axis gyro
 * @{
 *
 * @file       pios_mpu000.c
 * @author     The OpenPilot Team, http://www.openpilot.org Copyright (C) 2012.
 * @brief      MPU6000 6-axis gyro and accel chip
 * @see        The GNU Public License (GPL) Version 3
 *
 ******************************************************************************
 */
/*istribu
 * This program is free software; you can rnedtt ad/oe ir modify
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

#include "pios.h"
#include <pios_mpu6000.h>
#ifdef PIOS_INCLUDE_MPU6000
#include <stdint.h>
#include <pios_constants.h>
#include <pios_sensors.h>

/* Global Variables */

enum pios_mpu6000_dev_magic {
    PIOS_MPU6000_DEV_MAGIC = 0x9da9b3ed,
};

// sensor driver interface
bool PIOS_MPU6000_driver_Test(uintptr_t context);
void PIOS_MPU6000_driver_Reset(uintptr_t context);
void PIOS_MPU6000_driver_get_scale(float *scales, uint8_t size, uintptr_t context);
QueueHandle_t PIOS_MPU6000_driver_get_queue(uintptr_t context);

const PIOS_SENSORS_Driver PIOS_MPU6000_Driver = {
    .test      = PIOS_MPU6000_driver_Test,
    .poll      = NULL,
    .fetch     = NULL,
    .reset     = PIOS_MPU6000_driver_Reset,
    .get_queue = PIOS_MPU6000_driver_get_queue,
    .get_scale = PIOS_MPU6000_driver_get_scale,
    .is_polled = false,
};
//


struct mpu6000_dev {
    uint32_t spi_id;
    uint32_t slave_num;
    QueueHandle_t queue;
    const struct pios_mpu6000_cfg *cfg;
    enum pios_mpu6000_range gyro_range;
    enum pios_mpu6000_accel_range accel_range;
    enum pios_mpu6000_filter filter;
    enum pios_mpu6000_dev_magic   magic;
};

#define PIOS_MPU6000_SAMPLES_BYTES    14
#define PIOS_MPU6000_SENSOR_FIRST_REG PIOS_MPU6000_ACCEL_X_OUT_MSB

typedef union {
    uint8_t buffer[1 + PIOS_MPU6000_SAMPLES_BYTES];
    struct {
        uint8_t dummy;
        uint8_t Accel_X_h;
        uint8_t Accel_X_l;
        uint8_t Accel_Y_h;
        uint8_t Accel_Y_l;
        uint8_t Accel_Z_h;
        uint8_t Accel_Z_l;
        uint8_t Temperature_h;
        uint8_t Temperature_l;
        uint8_t Gyro_X_h;
        uint8_t Gyro_X_l;
        uint8_t Gyro_Y_h;
        uint8_t Gyro_Y_l;
        uint8_t Gyro_Z_h;
        uint8_t Gyro_Z_l;
    } data;
} mpu6000_data_t;

#define GET_SENSOR_DATA(mpudataptr, sensor) (mpudataptr.data.sensor##_h << 8 | mpudataptr.data.sensor##_l)

// ! Global structure for this device device
static struct mpu6000_dev *dev;
volatile bool mpu6000_configured = false;
static mpu6000_data_t mpu6000_data;
static PIOS_SENSORS_3Axis_SensorsWithTemp *queue_data = 0;
#define SENSOR_COUNT     2
#define SENSOR_DATA_SIZE (sizeof(PIOS_SENSORS_3Axis_SensorsWithTemp) + sizeof(Vector3i16) * SENSOR_COUNT)
// ! Private functions
static struct mpu6000_dev *PIOS_MPU6000_alloc(const struct pios_mpu6000_cfg *cfg);
static int32_t PIOS_MPU6000_Validate(struct mpu6000_dev *dev);
static void PIOS_MPU6000_Config(struct pios_mpu6000_cfg const *cfg);
static int32_t PIOS_MPU6000_SetReg(uint8_t address, uint8_t buffer);
static int32_t PIOS_MPU6000_GetReg(uint8_t address);
static void PIOS_MPU6000_SetSpeed(const bool fast);
static bool PIOS_MPU6000_HandleData();
static bool PIOS_MPU6000_ReadSensor(bool *woken);

static int32_t PIOS_MPU6000_Test(void);

void PIOS_MPU6000_Register()
{
    PIOS_SENSORS_Register(&PIOS_MPU6000_Driver, PIOS_SENSORS_TYPE_3AXIS_GYRO_ACCEL, 0);
}
/**
 * @brief Allocate a new device
 */
static struct mpu6000_dev *PIOS_MPU6000_alloc(const struct pios_mpu6000_cfg *cfg)
{
    struct mpu6000_dev *mpu6000_dev;

    mpu6000_dev = (struct mpu6000_dev *)pios_malloc(sizeof(*mpu6000_dev));
    PIOS_Assert(mpu6000_dev);

    mpu6000_dev->magic = PIOS_MPU6000_DEV_MAGIC;

    mpu6000_dev->queue = xQueueCreate(cfg->max_downsample + 1, SENSOR_DATA_SIZE);
    PIOS_Assert(mpu6000_dev->queue);

    queue_data = (PIOS_SENSORS_3Axis_SensorsWithTemp *)pios_malloc(SENSOR_DATA_SIZE);
    PIOS_Assert(queue_data);
    queue_data->count = SENSOR_COUNT;
    return mpu6000_dev;
}

/**
 * @brief Validate the handle to the spi device
 * @returns 0 for valid device or -1 otherwise
 */
static int32_t PIOS_MPU6000_Validate(struct mpu6000_dev *vdev)
{
    if (vdev == NULL) {
        return -1;
    }
    if (vdev->magic != PIOS_MPU6000_DEV_MAGIC) {
        return -2;
    }
    if (vdev->spi_id == 0) {
        return -3;
    }
    return 0;
}

/**
 * @brief Initialize the MPU6000 3-axis gyro sensor.
 * @return 0 for success, -1 for failure
 */
int32_t PIOS_MPU6000_Init(uint32_t spi_id, uint32_t slave_num, const struct pios_mpu6000_cfg *cfg)
{
    dev = PIOS_MPU6000_alloc(cfg);
    if (dev == NULL) {
        return -1;
    }

    dev->spi_id    = spi_id;
    dev->slave_num = slave_num;
    dev->cfg = cfg;

    /* Configure the MPU6000 Sensor */
    PIOS_MPU6000_Config(cfg);

    /* Set up EXTI line */
    PIOS_EXTI_Init(cfg->exti_cfg);
    return 0;
}

/**
 * @brief Initialize the MPU6000 3-axis gyro sensor
 * \return none
 * \param[in] PIOS_MPU6000_ConfigTypeDef struct to be used to configure sensor.
 *
 */
static void PIOS_MPU6000_Config(struct pios_mpu6000_cfg const *cfg)
{
    PIOS_MPU6000_Test();

    // Reset chip
    while (PIOS_MPU6000_SetReg(PIOS_MPU6000_PWR_MGMT_REG, PIOS_MPU6000_PWRMGMT_IMU_RST) != 0) {
        ;
    }
    PIOS_DELAY_WaitmS(50);

    // Reset chip and fifo
    while (PIOS_MPU6000_SetReg(PIOS_MPU6000_USER_CTRL_REG,
                               PIOS_MPU6000_USERCTL_GYRO_RST |
                               PIOS_MPU6000_USERCTL_SIG_COND |
                               PIOS_MPU6000_USERCTL_FIFO_RST) != 0) {
        ;
    }

    // Wait for reset to finish
    while (PIOS_MPU6000_GetReg(PIOS_MPU6000_USER_CTRL_REG) &
           (PIOS_MPU6000_USERCTL_GYRO_RST |
            PIOS_MPU6000_USERCTL_SIG_COND |
            PIOS_MPU6000_USERCTL_FIFO_RST)) {
        ;
    }
    PIOS_DELAY_WaitmS(10);
    // Power management configuration
    while (PIOS_MPU6000_SetReg(PIOS_MPU6000_PWR_MGMT_REG, cfg->Pwr_mgmt_clk) != 0) {
        ;
    }

    // Interrupt configuration
    while (PIOS_MPU6000_SetReg(PIOS_MPU6000_INT_CFG_REG, cfg->interrupt_cfg) != 0) {
        ;
    }

    // Interrupt configuration
    while (PIOS_MPU6000_SetReg(PIOS_MPU6000_INT_EN_REG, cfg->interrupt_en) != 0) {
        ;
    }

    // FIFO storage
    while (PIOS_MPU6000_SetReg(PIOS_MPU6000_FIFO_EN_REG, cfg->Fifo_store) != 0) {
        ;
    }
    PIOS_MPU6000_ConfigureRanges(cfg->gyro_range, cfg->accel_range, cfg->filter);
    // Interrupt configuration
    while (PIOS_MPU6000_SetReg(PIOS_MPU6000_USER_CTRL_REG, cfg->User_ctl) != 0) {
        ;
    }

    // Interrupt configuration
    while (PIOS_MPU6000_SetReg(PIOS_MPU6000_PWR_MGMT_REG, cfg->Pwr_mgmt_clk) != 0) {
        ;
    }

    // Interrupt configuration
    while (PIOS_MPU6000_SetReg(PIOS_MPU6000_INT_CFG_REG, cfg->interrupt_cfg) != 0) {
        ;
    }

    // Interrupt configuration
    while (PIOS_MPU6000_SetReg(PIOS_MPU6000_INT_EN_REG, cfg->interrupt_en) != 0) {
        ;
    }
    if ((PIOS_MPU6000_GetReg(PIOS_MPU6000_INT_EN_REG)) != cfg->interrupt_en) {
        return;
    }

    mpu6000_configured = true;
}
/**
 * @brief Configures Gyro, accel and Filter ranges/setings
 * @return 0 if successful, -1 if device has not been initialized
 */
int32_t PIOS_MPU6000_ConfigureRanges(
    enum pios_mpu6000_range gyroRange,
    enum pios_mpu6000_accel_range accelRange,
    enum pios_mpu6000_filter filterSetting)
{
    if (dev == NULL) {
        return -1;
    }

    // update filter settings
    while (PIOS_MPU6000_SetReg(PIOS_MPU6000_DLPF_CFG_REG, filterSetting) != 0) {
        ;
    }

    // Sample rate divider, chosen upon digital filtering settings
    while (PIOS_MPU6000_SetReg(PIOS_MPU6000_SMPLRT_DIV_REG,
                               filterSetting == PIOS_MPU6000_LOWPASS_256_HZ ?
                               dev->cfg->Smpl_rate_div_no_dlp : dev->cfg->Smpl_rate_div_dlp) != 0) {
        ;
    }

    dev->filter = filterSetting;

    // Gyro range
    while (PIOS_MPU6000_SetReg(PIOS_MPU6000_GYRO_CFG_REG, gyroRange) != 0) {
        ;
    }

    dev->gyro_range = gyroRange;
    // Set the accel range
    while (PIOS_MPU6000_SetReg(PIOS_MPU6000_ACCEL_CFG_REG, accelRange) != 0) {
        ;
    }

    dev->accel_range = accelRange;
    return 0;
}

/**
 * @brief Claim the SPI bus for the accel communications and select this chip
 * @return 0 if successful, -1 for invalid device, -2 if unable to claim bus
 */
static int32_t PIOS_MPU6000_ClaimBus(bool fast_spi)
{
    if (PIOS_MPU6000_Validate(dev) != 0) {
        return -1;
    }
    if (PIOS_SPI_ClaimBus(dev->spi_id) != 0) {
        return -2;
    }
    PIOS_MPU6000_SetSpeed(fast_spi);
    PIOS_SPI_RC_PinSet(dev->spi_id, dev->slave_num, 0);
    return 0;
}


static void PIOS_MPU6000_SetSpeed(const bool fast)
{
    if (fast) {
        PIOS_SPI_SetClockSpeed(dev->spi_id, dev->cfg->fast_prescaler);
    } else {
        PIOS_SPI_SetClockSpeed(dev->spi_id, dev->cfg->std_prescaler);
    }
}

/**
 * @brief Claim the SPI bus for the accel communications and select this chip
 * @return 0 if successful, -1 for invalid device, -2 if unable to claim bus
 * @param woken[in,out] If non-NULL, will be set to true if woken was false and a higher priority
 *                      task has is now eligible to run, else unchanged
 */
static int32_t PIOS_MPU6000_ClaimBusISR(bool *woken, bool fast_spi)
{
    if (PIOS_MPU6000_Validate(dev) != 0) {
        return -1;
    }
    if (PIOS_SPI_ClaimBusISR(dev->spi_id, woken) != 0) {
        return -2;
    }
    PIOS_MPU6000_SetSpeed(fast_spi);
    PIOS_SPI_RC_PinSet(dev->spi_id, dev->slave_num, 0);
    return 0;
}

/**
 * @brief Release the SPI bus for the accel communications and end the transaction
 * @return 0 if successful
 */
static int32_t PIOS_MPU6000_ReleaseBus()
{
    if (PIOS_MPU6000_Validate(dev) != 0) {
        return -1;
    }
    PIOS_SPI_RC_PinSet(dev->spi_id, dev->slave_num, 1);
    return PIOS_SPI_ReleaseBus(dev->spi_id);
}

/**
 * @brief Release the SPI bus for the accel communications and end the transaction
 * @return 0 if successful
 * @param woken[in,out] If non-NULL, will be set to true if woken was false and a higher priority
 *                      task has is now eligible to run, else unchanged
 */
static int32_t PIOS_MPU6000_ReleaseBusISR(bool *woken)
{
    if (PIOS_MPU6000_Validate(dev) != 0) {
        return -1;
    }
    PIOS_SPI_RC_PinSet(dev->spi_id, dev->slave_num, 1);
    return PIOS_SPI_ReleaseBusISR(dev->spi_id, woken);
}

/**
 * @brief Read a register from MPU6000
 * @returns The register value or -1 if failure to get bus
 * @param reg[in] Register address to be read
 */
static int32_t PIOS_MPU6000_GetReg(uint8_t reg)
{
    uint8_t data;

    if (PIOS_MPU6000_ClaimBus(false) != 0) {
        return -1;
    }

    PIOS_SPI_TransferByte(dev->spi_id, (0x80 | reg)); // request byte
    data = PIOS_SPI_TransferByte(dev->spi_id, 0); // receive response

    PIOS_MPU6000_ReleaseBus();
    return data;
}

/**
 * @brief Writes one byte to the MPU6000
 * \param[in] reg Register address
 * \param[in] data Byte to write
 * \return 0 if operation was successful
 * \return -1 if unable to claim SPI bus
 * \return -2 if unable to claim i2c device
 */
static int32_t PIOS_MPU6000_SetReg(uint8_t reg, uint8_t data)
{
    if (PIOS_MPU6000_ClaimBus(false) != 0) {
        return -1;
    }

    if (PIOS_SPI_TransferByte(dev->spi_id, 0x7f & reg) != 0) {
        PIOS_MPU6000_ReleaseBus();
        return -2;
    }

    if (PIOS_SPI_TransferByte(dev->spi_id, data) != 0) {
        PIOS_MPU6000_ReleaseBus();
        return -3;
    }

    PIOS_MPU6000_ReleaseBus();

    return 0;
}

/**
 * @brief Perform a dummy read in order to restart interrupt generation
 * \returns 0 if succesful
 */
int32_t PIOS_MPU6000_DummyReadGyros()
{
    // THIS FUNCTION IS DEPRECATED AND DOES NOT PERFORM A ROTATION
    uint8_t buf[7] = { PIOS_MPU6000_GYRO_X_OUT_MSB | 0x80, 0, 0, 0, 0, 0, 0 };
    uint8_t rec[7];

    if (PIOS_MPU6000_ClaimBus(true) != 0) {
        return -1;
    }

    if (PIOS_SPI_TransferBlock(dev->spi_id, &buf[0], &rec[0], sizeof(buf), NULL) < 0) {
        return -2;
    }

    PIOS_MPU6000_ReleaseBus();

    return 0;
}

/*
 * @brief Read the identification bytes from the MPU6000 sensor
 * \return ID read from MPU6000 or -1 if failure
 */
int32_t PIOS_MPU6000_ReadID()
{
    int32_t mpu6000_id = PIOS_MPU6000_GetReg(PIOS_MPU6000_WHOAMI);

    if (mpu6000_id < 0) {
        return -1;
    }
    return mpu6000_id;
}

/**
 * \brief Reads the queue handle
 * \return Handle to the queue or null if invalid device
 */
xQueueHandle PIOS_MPU6000_GetQueue()
{
    if (PIOS_MPU6000_Validate(dev) != 0) {
        return (xQueueHandle)NULL;
    }

    return dev->queue;
}


static float PIOS_MPU6000_GetScale()
{
    switch (dev->gyro_range) {
    case PIOS_MPU6000_SCALE_250_DEG:
        return 1.0f / 131.0f;

    case PIOS_MPU6000_SCALE_500_DEG:
        return 1.0f / 65.5f;

    case PIOS_MPU6000_SCALE_1000_DEG:
        return 1.0f / 32.8f;

    case PIOS_MPU6000_SCALE_2000_DEG:
        return 1.0f / 16.4f;
    }
    return 0;
}

static float PIOS_MPU6000_GetAccelScale()
{
    switch (dev->accel_range) {
    case PIOS_MPU6000_ACCEL_2G:
        return PIOS_CONST_MKS_GRAV_ACCEL_F / 16384.0f;

    case PIOS_MPU6000_ACCEL_4G:
        return PIOS_CONST_MKS_GRAV_ACCEL_F / 8192.0f;

    case PIOS_MPU6000_ACCEL_8G:
        return PIOS_CONST_MKS_GRAV_ACCEL_F / 4096.0f;

    case PIOS_MPU6000_ACCEL_16G:
        return PIOS_CONST_MKS_GRAV_ACCEL_F / 2048.0f;
    }
    return 0;
}

/**
 * @brief Run self-test operation.
 * \return 0 if test succeeded
 * \return non-zero value if test succeeded
 */
static int32_t PIOS_MPU6000_Test(void)
{
    /* Verify that ID matches (MPU6000 ID is 0x69) */
    int32_t mpu6000_id = PIOS_MPU6000_ReadID();

    if (mpu6000_id < 0) {
        return -1;
    }

    if (mpu6000_id != 0x68) {
        return -2;
    }

    return 0;
}

/**
 * @brief EXTI IRQ Handler.  Read all the data from onboard buffer
 * @return a boleoan to the EXTI IRQ Handler wrapper indicating if a
 *         higher priority task is now eligible to run
 */

bool PIOS_MPU6000_IRQHandler(void)
{
    bool woken = false;

    if (!mpu6000_configured) {
        return false;
    }

    bool read_ok = false;
    read_ok = PIOS_MPU6000_ReadSensor(&woken);

    if (read_ok) {
        bool woken2 = PIOS_MPU6000_HandleData();
        woken |= woken2;
    }

    return woken;
}

static bool PIOS_MPU6000_HandleData()
{
    if (!queue_data) {
        return false;
    }

    // Rotate the sensor to OP convention.  The datasheet defines X as towards the right
    // and Y as forward.  OP convention transposes this.  Also the Z is defined negatively
    // to our convention

    // Currently we only support rotations on top so switch X/Y accordingly
    switch (dev->cfg->orientation) {
    case PIOS_MPU6000_TOP_0DEG:
        queue_data->sample[0].y = GET_SENSOR_DATA(mpu6000_data, Accel_X); // chip X
        queue_data->sample[0].x = GET_SENSOR_DATA(mpu6000_data, Accel_Y); // chip Y
        queue_data->sample[1].y = GET_SENSOR_DATA(mpu6000_data, Gyro_X); // chip X
        queue_data->sample[1].x = GET_SENSOR_DATA(mpu6000_data, Gyro_Y); // chip Y
        break;
    case PIOS_MPU6000_TOP_90DEG:
        // -1 to bring it back to -32768 +32767 range
        queue_data->sample[0].y = -1 - (GET_SENSOR_DATA(mpu6000_data, Accel_Y)); // chip Y
        queue_data->sample[0].x = GET_SENSOR_DATA(mpu6000_data, Accel_X); // chip X
        queue_data->sample[1].y = -1 - (GET_SENSOR_DATA(mpu6000_data, Gyro_Y)); // chip Y
        queue_data->sample[1].x = GET_SENSOR_DATA(mpu6000_data, Gyro_X); // chip X
        break;
    case PIOS_MPU6000_TOP_180DEG:
        queue_data->sample[0].y = -1 - (GET_SENSOR_DATA(mpu6000_data, Accel_X)); // chip X
        queue_data->sample[0].x = -1 - (GET_SENSOR_DATA(mpu6000_data, Accel_Y)); // chip Y
        queue_data->sample[1].y = -1 - (GET_SENSOR_DATA(mpu6000_data, Gyro_X)); // chip X
        queue_data->sample[1].x = -1 - (GET_SENSOR_DATA(mpu6000_data, Gyro_Y)); // chip Y
        break;
    case PIOS_MPU6000_TOP_270DEG:
        queue_data->sample[0].y = GET_SENSOR_DATA(mpu6000_data, Accel_Y); // chip Y
        queue_data->sample[0].x = -1 - (GET_SENSOR_DATA(mpu6000_data, Accel_X)); // chip X
        queue_data->sample[1].y = GET_SENSOR_DATA(mpu6000_data, Gyro_Y); // chip Y
        queue_data->sample[1].x = -1 - (GET_SENSOR_DATA(mpu6000_data, Gyro_X)); // chip X
        break;
    }
    queue_data->sample[0].z = -1 - (GET_SENSOR_DATA(mpu6000_data, Accel_Z));
    queue_data->sample[1].z = -1 - (GET_SENSOR_DATA(mpu6000_data, Gyro_Z));
    const int16_t temp = GET_SENSOR_DATA(mpu6000_data, Temperature);
    queue_data->temperature = 3500 + ((float)(temp + 512)) * (1.0f / 3.4f);

    BaseType_t higherPriorityTaskWoken;
    xQueueSendToBackFromISR(dev->queue, (void *)&queue_data, &higherPriorityTaskWoken);
    return higherPriorityTaskWoken == pdTRUE;
}

static bool PIOS_MPU6000_ReadSensor(bool *woken)
{
    const uint8_t mpu6000_send_buf[1 + PIOS_MPU6000_SAMPLES_BYTES] = { PIOS_MPU6000_SENSOR_FIRST_REG | 0x80 };

    if (PIOS_MPU6000_ClaimBusISR(woken, true) != 0) {
        return false;
    }
    if (PIOS_SPI_TransferBlock(dev->spi_id, &mpu6000_send_buf[0], &mpu6000_data.buffer[0], sizeof(mpu6000_data_t), NULL) < 0) {
        PIOS_MPU6000_ReleaseBusISR(woken);
        return false;
    }
    PIOS_MPU6000_ReleaseBusISR(woken);
    return true;
}

// Sensor driver implementation
bool PIOS_MPU6000_driver_Test(__attribute__((unused)) uintptr_t context)
{
    return !PIOS_MPU6000_Test();
}

void PIOS_MPU6000_driver_Reset(__attribute__((unused)) uintptr_t context)
{
    PIOS_MPU6000_DummyReadGyros();
}

void PIOS_MPU6000_driver_get_scale(float *scales, uint8_t size, __attribute__((unused)) uintptr_t contet)
{
    PIOS_Assert(size >= 2);
    scales[0] = PIOS_MPU6000_GetAccelScale();
    scales[1] = PIOS_MPU6000_GetScale();
}

QueueHandle_t PIOS_MPU6000_driver_get_queue(__attribute__((unused)) uintptr_t context)
{
    return dev->queue;
}
#endif /* PIOS_INCLUDE_MPU6000 */

/**
 * @}
 * @}
 */
