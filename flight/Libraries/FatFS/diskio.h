#ifndef _DISKIO
#define _DISKIO

#define _READONLY		0		/* 1: Read-only mode */
#define _USE_IOCTL		1

#include "integer.h"

/* Status of Disk Functions */
typedef uint32_t		DSTATUS;

/* Results of Disk Functions */
typedef enum {
        RES_OK = 0,             /* 0: Successful */
        RES_ERROR,              /* 1: R/W Error */
        RES_WRPRT,              /* 2: Write Protected */
        RES_NOTRDY,             /* 3: Not Ready */
        RES_PARERR              /* 4: Invalid Parameter */
} DRESULT;

/*---------------------------------------*/
/* Prototypes for disk control functions */
/*---------------------------------------*/
DSTATUS disk_initialize(uint8_t Drive);
DSTATUS disk_status(uint8_t Drive);
DRESULT disk_read(uint8_t Drive, uint8_t* Buffer, uint32_t SectorNumber, uint8_t SectorCount);
DRESULT disk_write(uint8_t Drive, const BYTE* Buffer, uint32_t SectorNumber, uint8_t SectorCount);
DRESULT disk_ioctl(uint8_t Drive, uint8_t Command, void* Buffer);

/* Disk Status Bits (DSTATUS) */

#define STA_NOINIT              0x01    /* Drive not initialized */
#define STA_NODISK              0x02    /* No medium in the drive */
#define STA_PROTECT             0x04    /* Write protected */


/* Command code for disk_ioctrl() */

/* Generic command */
#define CTRL_SYNC				0		/* Mandatory for write functions */
#define GET_SECTOR_COUNT		1		/* Mandatory for only f_mkfs() */
#define GET_SECTOR_SIZE         2
#define GET_BLOCK_SIZE          3		/* Mandatory for only f_mkfs() */
#define CTRL_POWER				4
#define CTRL_LOCK				5
#define CTRL_EJECT				6
/* MMC/SDC command */
#define MMC_GET_TYPE			10
#define MMC_GET_CSD				11
#define MMC_GET_CID				12
#define MMC_GET_OCR				13
#define MMC_GET_SDSTAT          14
/* ATA/CF command */
#define ATA_GET_REV				20
#define ATA_GET_MODEL			21
#define ATA_GET_SN				22





typedef struct
{
  uint8_t  CSDStruct;            /* CSD structure */
  uint8_t  SysSpecVersion;       /* System specification version */
  uint8_t  Reserved1;            /* Reserved */
  uint8_t  TAAC;                 /* Data read access-time 1 */
  uint8_t  NSAC;                 /* Data read access-time 2 in CLK cycles */
  uint8_t  MaxBusClkFrec;        /* Max. bus clock frequency */
  uint16_t CardComdClasses;      /* Card command classes */
  uint8_t  RdBlockLen;           /* Max. read data block length */
  uint8_t  PartBlockRead;        /* Partial blocks for read allowed */
  uint8_t  WrBlockMisalign;      /* Write block misalignment */
  uint8_t  RdBlockMisalign;      /* Read block misalignment */
  uint8_t  DSRImpl;              /* DSR implemented */
  uint8_t  Reserved2;            /* Reserved */
  uint16_t DeviceSize;           /* Device Size */
  uint8_t  MaxRdCurrentVDDMin;   /* Max. read current @ VDD min */
  uint8_t  MaxRdCurrentVDDMax;   /* Max. read current @ VDD max */
  uint8_t  MaxWrCurrentVDDMin;   /* Max. write current @ VDD min */
  uint8_t  MaxWrCurrentVDDMax;   /* Max. write current @ VDD max */
  uint8_t  DeviceSizeMul;        /* Device size multiplier */
  uint8_t  EraseGrSize;          /* Erase group size */
  uint8_t  EraseGrMul;           /* Erase group size multiplier */
  uint8_t  WrProtectGrSize;      /* Write protect group size */
  uint8_t  WrProtectGrEnable;    /* Write protect group enable */
  uint8_t  ManDeflECC;           /* Manufacturer default ECC */
  uint8_t  WrSpeedFact;          /* Write speed factor */
  uint8_t  MaxWrBlockLen;        /* Max. write data block length */
  uint8_t  WriteBlockPaPartial;  /* Partial blocks for write allowed */
  uint8_t  Reserved3;            /* Reserded */
  uint8_t  ContentProtectAppli;  /* Content protection application */
  uint8_t  FileFormatGrouop;     /* File format group */
  uint8_t  CopyFlag;             /* Copy flag (OTP) */
  uint8_t  PermWrProtect;        /* Permanent write protection */
  uint8_t  TempWrProtect;        /* Temporary write protection */
  uint8_t  FileFormat;           /* File Format */
  uint8_t  ECC;                  /* ECC code */
  uint8_t  msd_CRC;                  /* CRC */
  uint8_t  Reserved4;            /* always 1*/
} SDCARDCsdTypeDef;

/* Structure taken from Mass Storage Driver example provided by STM */
typedef struct
{
  uint8_t  ManufacturerID;       /* ManufacturerID */
  uint16_t OEM_AppliID;          /* OEM/Application ID */
  char ProdName[6];         /* Product Name */
  uint8_t  ProdRev;              /* Product Revision */
  u32 ProdSN;               /* Product Serial Number */
  uint8_t  Reserved1;            /* Reserved1 */
  uint16_t ManufactDate;         /* Manufacturing Date */
  uint8_t  msd_CRC;              /* CRC */
  uint8_t  Reserved2;            /* always 1*/
} SDCARDCidTypeDef;


/* Prototypes */
extern int32_t SDCARD_Init(void);
extern int32_t SDCARD_PowerOn(void);
extern int32_t SDCARD_PowerOff(void);
extern int32_t SDCARD_CheckAvailable(uint8_t was_available);
extern int32_t SDCARD_SendSDCCmd(uint8_t cmd, uint32_t addr, uint8_t crc);
extern int32_t SDCARD_SectorRead(uint32_t sector, uint8_t *buffer);
extern int32_t SDCARD_SectorWrite(uint32_t sector, uint8_t *buffer);
extern int32_t SDCARD_CIDRead(SDCARDCidTypeDef *cid);
extern int32_t SDCARD_CSDRead(SDCARDCsdTypeDef *csd);


#endif
 
