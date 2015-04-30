/**
 ******************************************************************************
 * @addtogroup OpenPilotModules OpenPilot Modules
 * @{
 * @addtogroup GSPModule GPS Module
 * @brief Process GPS information
 * @{
 *
 * @file       UBX.h
 * @author     The OpenPilot Team, http://www.openpilot.org Copyright (C) 2010.
 * @brief      GPS module, handles GPS and NMEA stream
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

#ifndef UBX_H
#define UBX_H
#include "openpilot.h"
#include "gpspositionsensor.h"
#include "gpsextendedstatus.h"
#ifndef PIOS_GPS_MINIMAL
#include "auxmagsensor.h"
#endif

#include "GPS.h"

#define UBX_HW_VERSION_8 80000
#define UBX_HW_VERSION_7 70000

#define UBX_SYNC1        0xb5 // UBX protocol synchronization characters
#define UBX_SYNC2        0x62

// From u-blox6 receiver protocol specification

// Messages classes
typedef enum {
    UBX_CLASS_NAV     = 0x01,
    UBX_CLASS_ACK     = 0x05,
    UBX_CLASS_CFG     = 0x06,
    UBX_CLASS_MON     = 0x0A,
    UBX_CLASS_OP_CUST = 0x99,
    UBX_CLASS_AID     = 0x0B,
    // unused class IDs, used for disabling them
    UBX_CLASS_RXM     = 0x02,
} ubx_class;

// Message IDs
typedef enum {
    UBX_ID_NAV_POSLLH    = 0x02,
    UBX_ID_NAV_STATUS    = 0x03,
    UBX_ID_NAV_DOP       = 0x04,
    UBX_ID_NAV_SOL       = 0x06,
    UBX_ID_NAV_VELNED    = 0x12,
    UBX_ID_NAV_TIMEUTC   = 0x21,
    UBX_ID_NAV_SVINFO    = 0x30,
    UBX_ID_NAV_PVT       = 0x07,

    UBX_ID_NAV_AOPSTATUS = 0x60,
    UBX_ID_NAV_CLOCK     = 0x22,
    UBX_ID_NAV_DGPS      = 0x31,
    UBX_ID_NAV_POSECEF   = 0x01,
    UBX_ID_NAV_SBAS      = 0x32,
    UBX_ID_NAV_TIMEGPS   = 0x20,
    UBX_ID_NAV_VELECEF   = 0x11
} ubx_class_nav_id;

typedef enum {
    UBX_ID_OP_SYS = 0x01,
    UBX_ID_OP_MAG = 0x02,
} ubx_class_op_id;

typedef enum {
    UBX_ID_MON_VER    = 0x04,
    // unused messages IDs, used for disabling them
    UBX_ID_MON_HW2    = 0x0B,
    UBX_ID_MON_HW     = 0x09,
    UBX_ID_MON_IO     = 0x02,
    UBX_ID_MON_MSGPP  = 0x06,
    UBX_ID_MON_RXBUFF = 0x07,
    UBX_ID_MON_RXR    = 0x21,
    UBX_ID_MON_TXBUF  = 0x08,
} ubx_class_mon_id;

typedef enum {
    UBX_ID_CFG_NAV5 = 0x24,
    UBX_ID_CFG_RATE = 0x08,
    UBX_ID_CFG_MSG  = 0x01,
    UBX_ID_CFG_CFG  = 0x09,
    UBX_ID_CFG_SBAS = 0x16,
    UBX_ID_CFG_GNSS = 0x3E,
    UBX_ID_CFG_PRT  = 0x00
} ubx_class_cfg_id;

typedef enum {
    UBX_ID_ACK_ACK = 0x01,
    UBX_ID_ACK_NAK = 0x00,
} ubx_class_ack_id;

typedef enum {
    UBX_ID_AID_ALM    = 0x0B,
    UBX_ID_AID_ALPSRV = 0x32,
    UBX_ID_AID_ALP    = 0x50,
    UBX_ID_AID_AOP    = 0x33,
    UBX_ID_AID_DATA   = 0x10,
    UBX_ID_AID_EPH    = 0x31,
    UBX_ID_AID_HUI    = 0x02,
    UBX_ID_AID_INI    = 0x01,
    UBX_ID_AID_REQ    = 0x00,
} ubx_class_aid_id;

typedef enum {
    UBX_ID_RXM_ALM  = 0x30,
    UBX_ID_RXM_EPH  = 0x31,
    UBX_ID_RXM_RAW  = 0x10,
    UBX_ID_RXM_SFRB = 0x11,
    UBX_ID_RXM_SVSI = 0x20,
} ubx_class_rxm_id;

typedef enum {
    UBX_GNSS_ID_GPS     = 0,
    UBX_GNSS_ID_SBAS    = 1,
    UBX_GNSS_ID_GALILEO = 2,
    UBX_GNSS_ID_BEIDOU  = 3,
    UBX_GNSS_ID_IMES    = 4,
    UBX_GNSS_ID_QZSS    = 5,
    UBX_GNSS_ID_GLONASS = 6,
    UBX_GNSS_ID_MAX
} ubx_config_gnss_id_t;

// Enumeration options for field UBXDynamicModel
typedef enum {
    UBX_DYNMODEL_PORTABLE   = 0,
    UBX_DYNMODEL_STATIONARY = 2,
    UBX_DYNMODEL_PEDESTRIAN = 3,
    UBX_DYNMODEL_AUTOMOTIVE = 4,
    UBX_DYNMODEL_SEA = 5,
    UBX_DYNMODEL_AIRBORNE1G = 6,
    UBX_DYNMODEL_AIRBORNE2G = 7,
    UBX_DYNMODEL_AIRBORNE4G = 8
} ubx_config_dynamicmodel_t;

typedef enum {
    UBX_SBAS_SATS_AUTOSCAN = 0,
    UBX_SBAS_SATS_WAAS     = 1,
    UBX_SBAS_SATS_EGNOS    = 2,
    UBX_SBAS_SATS_MSAS     = 3,
    UBX_SBAS_SATS_GAGAN    = 4,
    UBX_SBAS_SATS_SDCM     = 5
} ubx_config_sats_t;

// private structures

// Geodetic Position Solution
struct UBX_NAV_POSLLH {
    uint32_t iTOW; // GPS Millisecond Time of Week (ms)
    int32_t  lon;    // Longitude (deg*1e-7)
    int32_t  lat;    // Latitude (deg*1e-7)
    int32_t  height; // Height above Ellipsoid (mm)
    int32_t  hMSL;   // Height above mean sea level (mm)
    uint32_t hAcc; // Horizontal Accuracy Estimate (mm)
    uint32_t vAcc; // Vertical Accuracy Estimate (mm)
} __attribute__((packed));

// Receiver Navigation Status
#define STATUS_GPSFIX_NOFIX    0x00
#define STATUS_GPSFIX_DRONLY   0x01
#define STATUS_GPSFIX_2DFIX    0x02
#define STATUS_GPSFIX_3DFIX    0x03
#define STATUS_GPSFIX_GPSDR    0x04
#define STATUS_GPSFIX_TIMEONLY 0x05

#define STATUS_FLAGS_GPSFIX_OK (1 << 0)
#define STATUS_FLAGS_DIFFSOLN  (1 << 1)
#define STATUS_FLAGS_WKNSET    (1 << 2)
#define STATUS_FLAGS_TOWSET    (1 << 3)

struct UBX_NAV_STATUS {
    uint32_t iTOW; // GPS Millisecond Time of Week (ms)
    uint8_t  gpsFix;  // GPS fix type
    uint8_t  flags;   // Navigation Status Flags
    uint8_t  fixStat; // Fix Status Information
    uint8_t  flags2;  // Additional navigation output information
    uint32_t ttff; // Time to first fix (ms)
    uint32_t msss; // Milliseconds since startup/reset (ms)
} __attribute__((packed));

// Dilution of precision
struct UBX_NAV_DOP {
    uint32_t iTOW; // GPS Millisecond Time of Week (ms)
    uint16_t gDOP; // Geometric DOP
    uint16_t pDOP; // Position DOP
    uint16_t tDOP; // Time DOP
    uint16_t vDOP; // Vertical DOP
    uint16_t hDOP; // Horizontal DOP
    uint16_t nDOP; // Northing DOP
    uint16_t eDOP; // Easting DOP
} __attribute__((packed));

// Navigation solution
struct UBX_NAV_SOL {
    uint32_t iTOW; // GPS Millisecond Time of Week (ms)
    int32_t  fTOW;       // fractional nanoseconds (ns)
    int16_t  week;       // GPS week
    uint8_t  gpsFix;     // GPS fix type
    uint8_t  flags;      // Fix status flags
    int32_t  ecefX;      // ECEF X coordinate (cm)
    int32_t  ecefY;      // ECEF Y coordinate (cm)
    int32_t  ecefZ;      // ECEF Z coordinate (cm)
    uint32_t pAcc; // 3D Position Accuracy Estimate (cm)
    int32_t  ecefVX;     // ECEF X coordinate (cm/s)
    int32_t  ecefVY;     // ECEF Y coordinate (cm/s)
    int32_t  ecefVZ;     // ECEF Z coordinate (cm/s)
    uint32_t sAcc; // Speed Accuracy Estimate
    uint16_t pDOP; // Position DOP
    uint8_t  reserved1;  // Reserved
    uint8_t  numSV;      // Number of SVs used in Nav Solution
    uint32_t reserved2; // Reserved
} __attribute__((packed));

// PVT Navigation Position Velocity Time Solution
#define PVT_VALID_VALIDDATE            0x01
#define PVT_VALID_VALIDTIME            0x02
#define PVT_VALID_FULLYRESOLVED        0x04

#define PVT_FIX_TYPE_NO_FIX            0
#define PVT_FIX_TYPE_DEAD_RECKON       0x01 // Dead Reckoning only
#define PVT_FIX_TYPE_2D                0x02 // 2D-Fix
#define PVT_FIX_TYPE_3D                0x03 // 3D-Fix
#define PVT_FIX_TYPE_GNSS_DEAD_RECKON  0x04 // GNSS + dead reckoning combined
#define PVT_FIX_TYPE_TIME_ONLY         0x05 // Time only fix

#define PVT_FLAGS_GNSSFIX_OK           (1 << 0)
#define PVT_FLAGS_DIFFSOLN             (1 << 1)
#define PVT_FLAGS_PSMSTATE_ENABLED     (1 << 2)
#define PVT_FLAGS_PSMSTATE_ACQUISITION (2 << 2)
#define PVT_FLAGS_PSMSTATE_TRACKING    (3 << 2)
#define PVT_FLAGS_PSMSTATE_PO_TRACKING (4 << 2)
#define PVT_FLAGS_PSMSTATE_INACTIVE    (5 << 2)

struct UBX_NAV_PVT {
    uint32_t iTOW;
    uint16_t year;
    uint8_t  month;
    uint8_t  day;
    uint8_t  hour;
    uint8_t  min;
    uint8_t  sec;
    uint8_t  valid;
    uint32_t tAcc;
    int32_t  nano;
    uint8_t  fixType;
    uint8_t  flags;
    uint8_t  reserved1;
    uint8_t  numSV;
    int32_t  lon;
    int32_t  lat;
    int32_t  height;
    int32_t  hMSL;
    uint32_t hAcc;
    uint32_t vAcc;
    int32_t  velN;
    int32_t  velE;
    int32_t  velD;
    int32_t  gSpeed;
    int32_t  heading;
    uint32_t sAcc;
    uint32_t headingAcc;
    uint16_t pDOP;
    uint16_t reserved2;
    uint32_t reserved3;
} __attribute__((packed));

// North/East/Down velocity
struct UBX_NAV_VELNED {
    uint32_t iTOW; // ms GPS Millisecond Time of Week
    int32_t  velN;     // cm/s NED north velocity
    int32_t  velE;     // cm/s NED east velocity
    int32_t  velD;     // cm/s NED down velocity
    uint32_t speed; // cm/s Speed (3-D)
    uint32_t gSpeed; // cm/s Ground Speed (2-D)
    int32_t  heading;  // 1e-5 *deg Heading of motion 2-D
    uint32_t sAcc; // cm/s Speed Accuracy Estimate
    uint32_t cAcc; // 1e-5 *deg Course / Heading Accuracy Estimate
} __attribute__((packed));

// UTC Time Solution
#define TIMEUTC_VALIDTOW (1 << 0)
#define TIMEUTC_VALIDWKN (1 << 1)
#define TIMEUTC_VALIDUTC (1 << 2)

struct UBX_NAV_TIMEUTC {
    uint32_t iTOW; // GPS Millisecond Time of Week (ms)
    uint32_t tAcc; // Time Accuracy Estimate (ns)
    int32_t  nano;   // Nanoseconds of second
    uint16_t year;
    uint8_t  month;
    uint8_t  day;
    uint8_t  hour;
    uint8_t  min;
    uint8_t  sec;
    uint8_t  valid;  // Validity Flags
} __attribute__((packed));

// Space Vehicle (SV) Information
// Single SV information block
#define SVUSED     (1 << 0) // This SV is used for navigation
#define DIFFCORR   (1 << 1) // Differential correction available
#define ORBITAVAIL (1 << 2) // Orbit information available
#define ORBITEPH   (1 << 3) // Orbit information is Ephemeris
#define UNHEALTHY  (1 << 4) // SV is unhealthy
#define ORBITALM   (1 << 5) // Orbit information is Almanac Plus
#define ORBITAOP   (1 << 6) // Orbit information is AssistNow Autonomous
#define SMOOTHED   (1 << 7) // Carrier smoothed pseudoranges used

struct UBX_NAV_SVINFO_SV {
    uint8_t chn; // Channel number
    uint8_t svid; // Satellite ID
    uint8_t flags; // Misc SV information
    uint8_t quality; // Misc quality indicators
    uint8_t cno; // Carrier to Noise Ratio (dbHz)
    int8_t  elev;     // Elevation (integer degrees)
    int16_t azim; // Azimuth	(integer degrees)
    int32_t prRes; // Pseudo range residual (cm)
} __attribute__((packed));

// SV information message
#define MAX_SVS 32

struct UBX_NAV_SVINFO {
    uint32_t iTOW; // GPS Millisecond Time of Week (ms)
    uint8_t  numCh;        // Number of channels
    uint8_t  globalFlags;  //
    uint16_t reserved2; // Reserved
    struct UBX_NAV_SVINFO_SV sv[MAX_SVS]; // Repeated 'numCh' times
} __attribute__((packed));

// ACK message class
struct UBX_ACK_ACK {
    uint8_t clsID; // ClassID
    uint8_t msgID; // MessageID
} __attribute__((packed));

struct UBX_ACK_NAK {
    uint8_t clsID; // ClassID
    uint8_t msgID; // MessageID
} __attribute__((packed));

// Communication port information
// default cfg_prt packet at 9600,N,8,1 (from u-center)
// 0000  B5 62 06 00 14 00-01 00
// 0008  00 00 D0 08 00 00 80 25
// 0010  00 00 07 00 03 00 00 00
// 0018  00 00-A2 B5
#define UBX_CFG_PRT_PORTID_DDC   0
#define UBX_CFG_PRT_PORTID_UART1 1
#define UBX_CFG_PRT_PORTID_UART2 2
#define UBX_CFG_PRT_PORTID_USB   3
#define UBX_CFG_PRT_PORTID_SPI   4
#define UBX_CFG_PRT_MODE_DATABITS5     0x00
#define UBX_CFG_PRT_MODE_DATABITS6     0x40
#define UBX_CFG_PRT_MODE_DATABITS7     0x80
#define UBX_CFG_PRT_MODE_DATABITS8     0xC0
#define UBX_CFG_PRT_MODE_EVENPARITY   0x000
#define UBX_CFG_PRT_MODE_ODDPARITY    0x200
#define UBX_CFG_PRT_MODE_NOPARITY     0x800
#define UBX_CFG_PRT_MODE_STOPBITS1_0 0x0000
#define UBX_CFG_PRT_MODE_STOPBITS1_5 0x1000
#define UBX_CFG_PRT_MODE_STOPBITS2_0 0x2000
#define UBX_CFG_PRT_MODE_STOPBITS0_5 0x3000
#define UBX_CFG_PRT_MODE_RESERVED      0x10

#define UBX_CFG_PRT_MODE_DEFAULT (UBX_CFG_PRT_MODE_DATABITS8 | UBX_CFG_PRT_MODE_NOPARITY | UBX_CFG_PRT_MODE_STOPBITS1_0 | UBX_CFG_PRT_MODE_RESERVED)

struct UBX_CFG_PRT {
    uint8_t  portID;       // 1 or 2 for UART ports
    uint8_t  res0;         // reserved
    uint16_t res1;         // reserved
    uint32_t mode;         // bit masks for databits, stopbits, parity, and non-zero reserved
    uint32_t baudRate;     // bits per second, 9600 means 9600
    uint16_t inProtoMask;  // bit 0 on = UBX, bit 1 on = NEMA
    uint16_t outProtoMask; // bit 0 on = UBX, bit 1 on = NEMA
    uint16_t flags;        // reserved
    uint16_t pad;          // reserved
} __attribute__((packed));

struct UBX_CFG_MSG {
    uint8_t msgClass;
    uint8_t msgID;
    uint8_t rate;
} __attribute__((packed));

struct UBX_CFG_RATE {
    uint16_t measRate;
    uint16_t navRate;
    uint16_t timeRef;
} __attribute__((packed));

// Mask for "all supported devices": battery backed RAM, Flash, EEPROM, SPI Flash
#define UBX_CFG_CFG_DEVICE_BBR      0x01
#define UBX_CFG_CFG_DEVICE_FLASH    0x02
#define UBX_CFG_CFG_DEVICE_EEPROM   0x04
#define UBX_CFG_CFG_DEVICE_SPIFLASH 0x10
#define UBX_CFG_CFG_DEVICE_ALL (UBX_CFG_CFG_DEVICE_BBR | UBX_CFG_CFG_DEVICE_FLASH | UBX_CFG_CFG_DEVICE_EEPROM | UBX_CFG_CFG_DEVICE_SPIFLASH)
#define UBX_CFG_CFG_SETTINGS_NONE     0x000
#define UBX_CFG_CFG_SETTINGS_IOPORT   0x001
#define UBX_CFG_CFG_SETTINGS_MSGCONF  0x002
#define UBX_CFG_CFG_SETTINGS_INFMSG   0x004
#define UBX_CFG_CFG_SETTINGS_NAVCONF  0x008
#define UBX_CFG_CFG_SETTINGS_TPCONF   0x010
#define UBX_CFG_CFG_SETTINGS_SFDRCONF 0x100
#define UBX_CFG_CFG_SETTINGS_RINVCONF 0x200
#define UBX_CFG_CFG_SETTINGS_ANTCONF  0x400

#define UBX_CFG_CFG_SETTINGS_ALL        (UBX_CFG_CFG_SETTINGS_IOPORT   | \
                                         UBX_CFG_CFG_SETTINGS_MSGCONF  | \
                                         UBX_CFG_CFG_SETTINGS_INFMSG   | \
                                         UBX_CFG_CFG_SETTINGS_NAVCONF  | \
                                         UBX_CFG_CFG_SETTINGS_TPCONF   | \
                                         UBX_CFG_CFG_SETTINGS_SFDRCONF | \
                                         UBX_CFG_CFG_SETTINGS_RINVCONF | \
                                         UBX_CFG_CFG_SETTINGS_ANTCONF)

// Sent messages for configuration support
struct UBX_CFG_CFG {
    uint32_t clearMask;
    uint32_t saveMask;
    uint32_t loadMask;
    uint8_t  deviceMask;
} __attribute__((packed));

#define UBX_CFG_SBAS_MODE_ENABLED    0x01
#define UBX_CFG_SBAS_MODE_TEST       0x02
#define UBX_CFG_SBAS_USAGE_RANGE     0x01
#define UBX_CFG_SBAS_USAGE_DIFFCORR  0x02
#define UBX_CFG_SBAS_USAGE_INTEGRITY 0x04

// SBAS used satellite PNR bitmask (120-151)
// -------------------------------------1---------1---------1---------1
// -------------------------------------5---------4---------3---------2
// ------------------------------------10987654321098765432109876543210
// WAAS 122, 133, 134, 135, 138---------|---------|---------|---------|
#define UBX_CFG_SBAS_SCANMODE1_WAAS  0b00000000000001001110000000000100
// EGNOS 120, 124, 126, 131-------------|---------|---------|---------|
#define UBX_CFG_SBAS_SCANMODE1_EGNOS 0b00000000000000000000100001010001
// MSAS 129, 137------------------------|---------|---------|---------|
#define UBX_CFG_SBAS_SCANMODE1_MSAS  0b00000000000000100000001000000000
// GAGAN 127, 128-----------------------|---------|---------|---------|
#define UBX_CFG_SBAS_SCANMODE1_GAGAN 0b00000000000000000000000110000000
// SDCM 125, 140, 141-------------------|---------|---------|---------|
#define UBX_CFG_SBAS_SCANMODE1_SDCM  0b00000000001100000000000000100000

#define UBX_CFG_SBAS_SCANMODE2       0x00
struct UBX_CFG_SBAS {
    uint8_t  mode;
    uint8_t  usage;
    uint8_t  maxSBAS;
    uint8_t  scanmode2;
    uint32_t scanmode1;
} __attribute__((packed));

#define UBX_CFG_GNSS_FLAGS_ENABLED      0x000001
#define UBX_CFG_GNSS_FLAGS_GPS_L1CA     0x010000
#define UBX_CFG_GNSS_FLAGS_SBAS_L1CA    0x010000
#define UBX_CFG_GNSS_FLAGS_BEIDOU_B1I   0x010000
#define UBX_CFG_GNSS_FLAGS_QZSS_L1CA    0x010000
#define UBX_CFG_GNSS_FLAGS_QZSS_L1SAIF  0x040000
#define UBX_CFG_GNSS_FLAGS_GLONASS_L1OF 0x010000

#define UBX_CFG_GNSS_NUMCH_VER7         22
#define UBX_CFG_GNSS_NUMCH_VER8         32

struct UBX_CFG_GNSS_CFGBLOCK {
    uint8_t  gnssId;
    uint8_t  resTrkCh;
    uint8_t  maxTrkCh;
    uint8_t  resvd;
    uint32_t flags;
} __attribute__((packed));

struct UBX_CFG_GNSS {
    uint8_t msgVer;
    uint8_t numTrkChHw;
    uint8_t numTrkChUse;
    uint8_t numConfigBlocks;
    struct UBX_CFG_GNSS_CFGBLOCK cfgBlocks[UBX_GNSS_ID_MAX];
} __attribute__((packed));

struct UBX_CFG_NAV5 {
    uint16_t mask;
    uint8_t  dynModel;
    uint8_t  fixMode;
    int32_t  fixedAlt;
    uint32_t fixedAltVar;
    int8_t   minElev;
    uint8_t  drLimit;
    uint16_t pDop;
    uint16_t tDop;
    uint16_t pAcc;
    uint16_t tAcc;
    uint8_t  staticHoldThresh;
    uint8_t  dgpsTimeOut;
    uint8_t  cnoThreshNumSVs;
    uint8_t  cnoThresh;
    uint16_t reserved2;
    uint32_t reserved3;
    uint32_t reserved4;
} __attribute__((packed));

// MON message Class
#define UBX_MON_MAX_EXT 5
struct UBX_MON_VER {
    char swVersion[30];
    char hwVersion[10];
#if UBX_MON_MAX_EXT > 0
    char extension[UBX_MON_MAX_EXT][30];
#endif
} __attribute__((packed));

// OP custom messages
struct UBX_OP_SYSINFO {
    uint32_t flightTime;
    uint16_t options;
    uint8_t  board_type;
    uint8_t  board_revision;
    uint8_t  commit_tag_name[26];
    uint8_t  sha1sum[8];
} __attribute__((packed));

// OP custom messages
struct UBX_OP_MAG {
    int16_t  x;
    int16_t  y;
    int16_t  z;
    uint16_t Status;
} __attribute__((packed));

typedef union {
    uint8_t payload[0];
    // Nav Class
    struct UBX_NAV_POSLLH  nav_posllh;
    struct UBX_NAV_STATUS  nav_status;
    struct UBX_NAV_DOP     nav_dop;
    struct UBX_NAV_SOL     nav_sol;
    struct UBX_NAV_VELNED  nav_velned;
#if !defined(PIOS_GPS_MINIMAL)
    struct UBX_NAV_PVT     nav_pvt;
    struct UBX_NAV_TIMEUTC nav_timeutc;
    struct UBX_NAV_SVINFO  nav_svinfo;
    struct UBX_CFG_PRT     cfg_prt;
    // Ack Class
    struct UBX_ACK_ACK     ack_ack;
    struct UBX_ACK_NAK     ack_nak;
    // Mon Class
    struct UBX_MON_VER     mon_ver;
    struct UBX_OP_SYSINFO  op_sysinfo;
    struct UBX_OP_MAG op_mag;
#endif
} UBXPayload;

struct UBXHeader {
    uint8_t  class;
    uint8_t  id;
    uint16_t len;
    uint8_t  ck_a;
    uint8_t  ck_b;
} __attribute__((packed));

struct UBXPacket {
    struct UBXHeader header;
    UBXPayload payload;
} __attribute__((packed));

struct UBXSENTHEADER {
    uint8_t  prolog[2];
    uint8_t  class;
    uint8_t  id;
    uint16_t len;
} __attribute__((packed));

union UBXSENTPACKET {
    uint8_t buffer[0];
    struct {
        struct UBXSENTHEADER header;
        union {
            struct UBX_CFG_CFG  cfg_cfg;
            struct UBX_CFG_MSG  cfg_msg;
            struct UBX_CFG_NAV5 cfg_nav5;
            struct UBX_CFG_PRT  cfg_prt;
            struct UBX_CFG_RATE cfg_rate;
            struct UBX_CFG_SBAS cfg_sbas;
            struct UBX_CFG_GNSS cfg_gnss;
        } payload;
        uint8_t resvd[2]; // added space for checksum bytes
    } message;
} __attribute__((packed));

// Used by AutoConfig code
extern int32_t ubxHwVersion;
extern GPSPositionSensorSensorTypeOptions sensorType;
extern struct UBX_ACK_ACK ubxLastAck;
extern struct UBX_ACK_NAK ubxLastNak;

bool checksum_ubx_message(struct UBXPacket *);
uint32_t parse_ubx_message(struct UBXPacket *, GPSPositionSensorData *);

int parse_ubx_stream(uint8_t *rx, uint16_t len, char *, GPSPositionSensorData *, struct GPS_RX_STATS *);
void load_mag_settings();

#endif /* UBX_H */
