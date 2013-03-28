/**
 ******************************************************************************
 * @addtogroup PIOS PIOS Core hardware abstraction layer
 * @{
 * @addtogroup PIOS_WAVPLAY Code for wave audio generator
 * @brief Wave audio generator
 * @{
 *
 * @file       pios_wavplay.c
 * @author     The OpenPilot Team, http://www.openpilot.org Copyright (C) 2012.
 * @brief      audio generator
 * @see        The GNU Public License (GPL) Version 3
 *
 ******************************************************************************
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

#include "pios.h"

#ifdef PIOS_INCLUDE_WAVE

static const struct pios_dac_cfg * dev_cfg;

typedef enum
{
  LittleEndian,
  BigEndian
}Endianness;
/**
  * @}
  */

/** @defgroup WAVEPLAYER_Private_Defines
  * @{
  */
#define  CHUNK_ID                            0x52494646  /* correspond to the letters 'RIFF' */
#define  FILE_FORMAT                         0x57415645  /* correspond to the letters 'WAVE' */
#define  FORMAT_ID                           0x666D7420  /* correspond to the letters 'fmt ' */
#define  DATA_ID                             0x64617461  /* correspond to the letters 'data' */
#define  FACT_ID                             0x66616374  /* correspond to the letters 'fact' */
#define  WAVE_FORMAT_PCM                     0x01
#define  FORMAT_CHNUK_SIZE                   0x10
#define  CHANNEL_MONO                        0x01
#define  SAMPLE_RATE_8000                    8000
#define  SAMPLE_RATE_11025                   11025
#define  SAMPLE_RATE_22050                   22050
#define  SAMPLE_RATE_44100                   44100
#define  BITS_PER_SAMPLE_8                   8
#define  WAVE_DUMMY_BYTE                     0xA5
#define  DAC_DHLCD_REG_8LCD_REG_1_ADDRESS    0x40007410

typedef struct
{
  uint32_t  RIFFchunksize;
  uint16_t  FormatTag;
  uint16_t  NumChannels;
  uint32_t  SampleRate;
  uint32_t  ByteRate;
  uint16_t  BlockAlign;
  uint16_t  BitsPerSample;
  uint32_t  DataSize;
}
WAVE_FormatTypeDef;
typedef enum
{
  Valid_WAVE_File = 0,
  Unvalid_RIFF_ID,
  Unvalid_WAVE_Format,
  Unvalid_FormatChunk_ID,
  Unsupporetd_FormatTag,
  Unsupporetd_Number_Of_Channel,
  Unsupporetd_Sample_Rate,
  Unsupporetd_Bits_Per_Sample,
  Unvalid_DataChunk_ID,
  Unsupporetd_ExtraFormatBytes,
  Unvalid_FactChunk_ID
} ErrorCode;

/**
  * @}
  */

/** @defgroup WAVEPLAYER_Exported_Constants
  * @{
  */
#define SpeechReadAddr         0x0  /* Speech wave start read address */


/* Audio Play STATUS */
#define AudioPlayStatus_STOPPED       0
#define AudioPlayStatus_PLAYING	      1
#define AudioPlayStatus_PAUSED        2

#define MAX_WAVE_FILES       25


/**
  * @}
  */

/** @defgroup WAVEPLAYER_Private_Macros
  * @{
  */
/**
  * @}
  */

/** @defgroup WAVEPLAYER_Private_Variables
  * @{
  */
static WAVE_FormatTypeDef WAVE_Format;
static ErrorCode WaveFileStatus = Unvalid_RIFF_ID;
static uint16_t TIM6ARRValue = 1088;
uint32_t WaveDataLength = 0;
static uint32_t SpeechDataOffset = 0x00;
static uint32_t wavelen = 0;
FILEINFO fiwave;
FILEINFO file;

static uint8_t buffer1[BUFFERSIZE], buffer2[BUFFERSIZE]={0};	//Two cycling buffers which contain the WAV data.
uint32_t wavecounter;

typedef struct
{
	unsigned int format;
	unsigned int sample_rate;
	unsigned int bits_per_sample;
}wave_format;
wave_format wave_info;

/**
  * @brief  Decrements the played wave data length.
  * @param  None
  * @retval Current value of  WaveDataLength variable.
  */
uint32_t Decrement_WaveDataLength(void)
{
  if (WaveDataLength != 0x00)
  {
    WaveDataLength--;
  }
  return (WaveDataLength);
}


/**
  * @brief  Decrements the played wave data length.
  * @param  None
  * @retval Current value of  WaveDataLength variable.
  */
void Set_WaveDataLength(uint32_t value)
{
  WaveDataLength = value;
}

/**
  * @brief  Reads a number of bytes from the SPI Flash and reorder them in Big
  *         or little endian.
  * @param  NbrOfBytes: number of bytes to read.
  *         This parameter must be a number between 1 and 4.
  * @param  ReadAddr: external memory address to read from.
  * @param  Endians: specifies the bytes endianness.
  *         This parameter can be one of the following values:
  *             - LittleEndian
  *             - BigEndian
  * @retval Bytes read from the SPI Flash.
  */
static uint32_t ReadUnit(uint8_t *buffer, uint8_t idx, uint8_t NbrOfBytes, Endianness BytesFormat)
{
  uint32_t index = 0;
  uint32_t Temp = 0;

  for (index = 0; index < NbrOfBytes; index++)
  {
    Temp |= buffer[idx + index] << (index * 8);
  }

  if (BytesFormat == BigEndian)
  {
    Temp = __REV(Temp);
  }
  return Temp;
}


/**
  * @brief  Checks the format of the .WAV file and gets information about
  *   the audio format. This is done by reading the value of a
  *   number of parameters stored in the file header and comparing
  *   these to the values expected authenticates the format of a
  *   standard .WAV  file (44 bytes will be read). If  it is a valid
  *   .WAV file format, it continues reading the header to determine
  *   the  audio format such as the sample rate and the sampled data
  *   size. If the audio format is supported by this application,
  *   it retrieves the audio format in WAVE_Format structure and
  *   returns a zero value. Otherwise the function fails and the
  *   return value is nonzero.In this case, the return value specifies
  *   the cause of  the function fails. The error codes that can be
  *   returned by this function are declared in the header file.
  * @param  None
  * @retval Zero value if the function succeed, otherwise it return
  *         a nonzero value which specifies the error code.
  */
static ErrorCode WavePlayer_WaveParsing(uint8_t *DirName, uint8_t *FileName, uint32_t *FileLen)
{
  uint32_t Temp = 0x00;
  uint32_t ExtraFormatBytes = 0;
  __IO uint32_t err = 0;
  uint32_t number_of_clusters;
  uint32_t i;

  /* Directory enumeration test */
  if (PIOS_FOPEN_READ(FileName, file))
  {
    err = 1;
  }
  else
  {
    *FileLen = file.filelen;
    number_of_clusters = file.filelen / 512;
    if ((file.filelen % SECTOR_SIZE) > 0)
    {
      number_of_clusters ++;
    }
  }
  PIOS_FREAD(&file, buffer1, 44, &i);
  //DFS_ReadFile(&file, sector, buffer1, &i, SECTOR_SIZE);

  /* Read chunkID, must be 'RIFF'  ----------------------------------------------*/
  Temp = ReadUnit(buffer1, 0, 4, BigEndian);
  if (Temp != CHUNK_ID)
  {
    return(Unvalid_RIFF_ID);
  }

  /* Read the file length ----------------------------------------------------*/
  WAVE_Format.RIFFchunksize = ReadUnit(buffer1, 4, 4, LittleEndian);

  /* Read the file format, must be 'WAVE' ------------------------------------*/
  Temp = ReadUnit(buffer1, 8, 4, BigEndian);
  if (Temp != FILE_FORMAT)
  {
    return(Unvalid_WAVE_Format);
  }

  /* Read the format chunk, must be'fmt ' --------------------------------------*/
  Temp = ReadUnit(buffer1, 12, 4, BigEndian);
  if (Temp != FORMAT_ID)
  {
    return(Unvalid_FormatChunk_ID);
  }
  /* Read the length of the 'fmt' data, must be 0x10 -------------------------*/
  Temp = ReadUnit(buffer1, 16, 4, LittleEndian);
  if (Temp != 0x10)
  {
    ExtraFormatBytes = 1;
  }
  /* Read the audio format, must be 0x01 (PCM) -------------------------------*/
  WAVE_Format.FormatTag = ReadUnit(buffer1, 20, 2, LittleEndian);
  if (WAVE_Format.FormatTag != WAVE_FORMAT_PCM)
  {
    return(Unsupporetd_FormatTag);
  }

  /* Read the number of channels, must be 0x01 (Mono) ------------------------*/
  WAVE_Format.NumChannels = ReadUnit(buffer1, 22, 2, LittleEndian);
  if (WAVE_Format.NumChannels != CHANNEL_MONO)
  {
    return(Unsupporetd_Number_Of_Channel);
  }

  /* Read the Sample Rate ----------------------------------------------------*/
  WAVE_Format.SampleRate = ReadUnit(buffer1, 24, 4, LittleEndian);
  /* Update the OCA value according to the .WAV file Sample Rate */
  switch (WAVE_Format.SampleRate)
  {
    case SAMPLE_RATE_8000 :
      TIM6ARRValue = (PIOS_PERIPHERAL_APB1_CLOCK)/8000;
      break; /* 8KHz = 24MHz / 3000 */
    case SAMPLE_RATE_11025:
      TIM6ARRValue = (PIOS_PERIPHERAL_APB1_CLOCK)/11025;
      break; /* 11.025KHz = 24MHz / 2176 */
    case SAMPLE_RATE_22050:
      TIM6ARRValue = (PIOS_PERIPHERAL_APB1_CLOCK)/22050;
      break; /* 22.05KHz = 24MHz / 1088 */
    case SAMPLE_RATE_44100:
      TIM6ARRValue = (PIOS_PERIPHERAL_APB1_CLOCK)/44100;
      break; /* 44.1KHz = 24MHz / 544 */
    default:
      return(Unsupporetd_Sample_Rate);
  }

  /* Read the Byte Rate ------------------------------------------------------*/
  WAVE_Format.ByteRate = ReadUnit(buffer1, 28, 4, LittleEndian);

  /* Read the block alignment ------------------------------------------------*/
  WAVE_Format.BlockAlign = ReadUnit(buffer1, 32, 2, LittleEndian);

  /* Read the number of bits per sample --------------------------------------*/
  WAVE_Format.BitsPerSample = ReadUnit(buffer1, 34, 2, LittleEndian);
  if (WAVE_Format.BitsPerSample != BITS_PER_SAMPLE_8)
  {
    return(Unsupporetd_Bits_Per_Sample);
  }
  SpeechDataOffset = 36;
  /* If there is Extra format bytes, these bytes will be defined in "Fact Chunk" */
  if (ExtraFormatBytes == 1)
  {
    /* Read th Extra format bytes, must be 0x00 ------------------------------*/
    Temp = ReadUnit(buffer1, 36, 2, LittleEndian);
    if (Temp != 0x00)
    {
      return(Unsupporetd_ExtraFormatBytes);
    }
    /* Read the Fact chunk, must be 'fact' -----------------------------------*/
    Temp = ReadUnit(buffer1, 38, 4, BigEndian);
    if (Temp != FACT_ID)
    {
      return(Unvalid_FactChunk_ID);
    }
    /* Read Fact chunk data Size ---------------------------------------------*/
    Temp = ReadUnit(buffer1, 42, 4, LittleEndian);

    SpeechDataOffset += 10 + Temp;
  }
  /* Read the Data chunk, must be 'data' ---------------------------------------*/
  Temp = ReadUnit(buffer1, SpeechDataOffset, 4, BigEndian);
  SpeechDataOffset += 4;
  if (Temp != DATA_ID)
  {
    return(Unvalid_DataChunk_ID);
  }

  /* Read the number of sample data ------------------------------------------*/
  WAVE_Format.DataSize = ReadUnit(buffer1, SpeechDataOffset, 4, LittleEndian);
  SpeechDataOffset += 4;
  wavecounter =  SpeechDataOffset;
  PIOS_FREAD(&file, buffer1, SECTOR_SIZE, &i);
  PIOS_FREAD(&file, buffer2, SECTOR_SIZE, &i);
  return(Valid_WAVE_File);
}


/**
  * @brief  Start wave playing
  * @param  None
  * @retval None
  */
int wavfile=0;
const uint8_t table[5][20] = {"openpilo.wav","uav.wav","beepsoun.wav", "warning.wav", "lowaltit.wav"};

uint8_t WavePlayer_Start(void)
{
	// Check for file system availability
	if (PIOS_SDCARD_IsMounted() == 0) {
		return -1;
	}

	/* Read the Speech wave file status */

	if(wavfile<5)
	{
		WaveFileStatus = WavePlayer_WaveParsing(" ", table[wavfile++], &wavelen);
		if(wavfile>4)
		{
			wavfile=5;
		}
		//TIM_ITConfig(TIM6, TIM_IT_Update, ENABLE);
		//WaveDataLength = WAVE_Format.DataSize;
		//TIM_Cmd(TIM6, ENABLE);
		if (WaveFileStatus == Valid_WAVE_File) /* the .WAV file is valid */
		{
			/* Set WaveDataLenght to the Speech wave length */
			WaveDataLength = WAVE_Format.DataSize;

			TIM_Cmd(dev_cfg->timer, DISABLE);
			TIM_SetAutoreload(dev_cfg->timer, TIM6ARRValue);
			/* Start TIM6 */
			TIM_Cmd(dev_cfg->timer, ENABLE);
		}
		else
		{
			return -1;
		}
	}
	return 0;
}

#define TIM6_PERIOD (PIOS_PERIPHERAL_APB1_CLOCK)/44100
void PIOS_WavPlay_Init(const struct pios_dac_cfg * cfg){

	dev_cfg = cfg; // store config before enabling interrupt
#if 0
	GPIO_InitTypeDef GPIO_InitStructure;
	TIM_TimeBaseInitTypeDef    TIM_TimeBaseStructure;
	DMA_InitTypeDef DMA_InitStructure;
	NVIC_InitTypeDef NVIC_InitStructure;
	DAC_InitTypeDef  DAC_InitStructure;

	/* DAC channel 1 & 2 (DAC_OUT1 = PA.4)(DAC_OUT2 = PA.5) configuration */
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_4;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AN;
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
	GPIO_Init(GPIOA, &GPIO_InitStructure);


	NVIC_InitStructure.NVIC_IRQChannel = TIM6_DAC_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = PIOS_IRQ_PRIO_LOW;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);

	TIM_DeInit(TIM6);

	/* Time base configuration */
	TIM_TimeBaseStructInit(&TIM_TimeBaseStructure);
	TIM_TimeBaseStructure.TIM_Period = TIM6_PERIOD;
	TIM_TimeBaseStructure.TIM_Prescaler = 0;
	TIM_TimeBaseStructure.TIM_ClockDivision = 0;
	TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;
	TIM_TimeBaseInit(TIM6, &TIM_TimeBaseStructure);

	/* TIM6 TRGO selection */
	TIM_SelectOutputTrigger(TIM6, TIM_TRGOSource_Update);

	NVIC_InitStructure.NVIC_IRQChannel                   = DMA1_Stream5_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = PIOS_IRQ_PRIO_LOW;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority        = 0;
	NVIC_InitStructure.NVIC_IRQChannelCmd                = ENABLE;
	NVIC_Init(&NVIC_InitStructure);

	/* DMA1_Stream5 channel7 configuration **************************************/
	DMA_DeInit(DMA1_Stream5);
	DMA_InitStructure.DMA_Channel = DMA_Channel_7;
	DMA_InitStructure.DMA_PeripheralBaseAddr = (uint32_t)&DAC->DHR8R1;
	DMA_InitStructure.DMA_Memory0BaseAddr = (uint32_t)&buffer1;
	DMA_InitStructure.DMA_DIR = DMA_DIR_MemoryToPeripheral;
	DMA_InitStructure.DMA_BufferSize = BUFFERSIZE;
	DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;
	DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;
	DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_Byte;
	DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_Byte;
	DMA_InitStructure.DMA_Mode = DMA_Mode_Circular;
	DMA_InitStructure.DMA_Priority = DMA_Priority_High;
	DMA_InitStructure.DMA_FIFOMode           = DMA_FIFOMode_Disable;
	DMA_InitStructure.DMA_FIFOThreshold      = DMA_FIFOThreshold_Full;
	DMA_InitStructure.DMA_MemoryBurst        = DMA_MemoryBurst_Single;
	DMA_InitStructure.DMA_PeripheralBurst    = DMA_PeripheralBurst_Single;
	DMA_Init(DMA1_Stream5, &DMA_InitStructure);
	/* Configure double buffering */
	DMA_DoubleBufferModeConfig(DMA1_Stream5,(uint32_t)&buffer2,DMA_Memory_0);
	DMA_DoubleBufferModeCmd(DMA1_Stream5,ENABLE);

	/* Enable double buffering */
	DMA_Cmd(DMA1_Stream5, ENABLE);
	DMA_ITConfig(DMA1_Stream5, DMA_IT_TC, ENABLE);

	/* DAC channel1 Configuration */
	DAC_DeInit();
	DAC_StructInit(&DAC_InitStructure);

	DAC_InitStructure.DAC_Trigger = DAC_Trigger_T6_TRGO;
	DAC_InitStructure.DAC_WaveGeneration = DAC_WaveGeneration_None;
	DAC_InitStructure.DAC_OutputBuffer = DAC_OutputBuffer_Enable;
	DAC_Init(DAC_Channel_1, &DAC_InitStructure);

	DAC_Cmd(DAC_Channel_1, ENABLE);
	DAC_DMACmd(DAC_Channel_1, ENABLE);
#endif
#if 1

	GPIO_Init(cfg->dac_io.gpio, (GPIO_InitTypeDef*)&(cfg->dac_io.init));

	/* Configure the dividers for this timer */
	TIM_TimeBaseInit(cfg->timer, &cfg->time_base_init);

	/* Enable Interrupts */
	NVIC_Init(&cfg->irq.init);

	TIM_SelectOutputTrigger(cfg->timer, TIM_TRGOSource_Update);

	NVIC_Init(&cfg->dma.irq.init);


	DMA_Cmd(cfg->dma.tx.channel, DISABLE);
	DMA_Init(cfg->dma.tx.channel, (DMA_InitTypeDef*)&(cfg->dma.tx.init));

	/* Enable double buffering */
	DMA_MemoryTargetConfig(cfg->dma.tx.channel,(uint32_t)&buffer1,DMA_Memory_0);
	DMA_DoubleBufferModeConfig(cfg->dma.tx.channel,(uint32_t)&buffer2,DMA_Memory_0);
	DMA_DoubleBufferModeCmd(cfg->dma.tx.channel,ENABLE);

	DMA_Cmd(cfg->dma.tx.channel, ENABLE);
	DMA_ITConfig(cfg->dma.tx.channel, DMA_IT_TC, ENABLE);

	DAC_Init(cfg->channel, (DAC_InitTypeDef*)&(cfg->dac_init));

	DAC_Cmd(cfg->channel, ENABLE);
	DAC_DMACmd(cfg->channel, ENABLE);
#endif
	//WavePlayer_Start();
}


void WavePlayer_Stop(void)
{
  /* Disable TIM6 update interrupt */
  TIM_ITConfig(dev_cfg->timer, TIM_IT_Update, DISABLE);
  /* Disable TIM6 */
  TIM_Cmd(dev_cfg->timer, DISABLE);
}


void DAC_TIM_Handler(void);
void TIM6_DAC_IRQHandler(void) __attribute__ ((alias("DAC_TIM_Handler")));

/**
  * @brief  This function handles TIM6 global interrupt request.
  * @param  None
  * @retval None
  */
void DAC_TIM_Handler(void)
{
  if (TIM_GetITStatus(dev_cfg->timer, TIM_IT_Update) != RESET)
  {
    /* Clear TIM6 update interrupt */
    TIM_ClearITPendingBit(dev_cfg->timer, TIM_IT_Update);
  }
}


void DAC_DMA_Handler(void);
void DMA1_Stream5_IRQHandler(void) __attribute__ ((alias("DAC_DMA_Handler")));

/**
 * @brief Interrupt for half and full buffer transfer
 *
 * This interrupt handler swaps between the two halfs of the double buffer to make
 * sure the ahrs uses the most recent data.  Only swaps data when AHRS is idle, but
 * really this is a pretense of a sanity check since the DMA engine is consantly
 * running in the background.  Keep an eye on the ekf_too_slow variable to make sure
 * it's keeping up.
 */
void DAC_DMA_Handler(void)
{
	uint8_t status=0;
	uint32_t bytesRead=0;
	if (DMA_GetFlagStatus(dev_cfg->dma.tx.channel,DMA_FLAG_TCIF5)) {	// whole double buffer filled
	    if (WaveDataLength)
	    {
			if(DMA_GetCurrentMemoryTarget(dev_cfg->dma.tx.channel) == 0)
			{
				//DMA_MemoryTargetConfig(DMA1_Stream5,(uint32_t)&buffer2,DMA_Memory_1);
				PIOS_FREAD(&file, buffer2, BUFFERSIZE, &bytesRead);
				if (bytesRead != BUFFERSIZE) {
					status=2;
				}
			}
			else
			{
				//DMA_MemoryTargetConfig(DMA1_Stream5,(uint32_t)&buffer1,DMA_Memory_0);
				PIOS_FREAD(&file, buffer1, BUFFERSIZE, &bytesRead);
				if (bytesRead != BUFFERSIZE) {
					status=1;
				}
			}
			if(status)
			{
				// STOP DMA, master first
				/*DMA_Cmd(DMA1_Stream5, DISABLE);*/
				//PIOS_FCLOSE(file);
				//LoadWav();

			}
			WaveDataLength -= 512;
	    }
	    if (WaveDataLength < 512) WaveDataLength = 0;
	    /* If we reach the WaveDataLength of the wave to play */
	    if (WaveDataLength == 0)
	    {
	      /* Stop wave playing */
	      WavePlayer_Stop();
	      PIOS_FCLOSE(file);
	      WavePlayer_Start();
	    }
		DMA_ClearFlag(dev_cfg->dma.tx.channel,DMA_FLAG_TCIF5);
	}
	else if (DMA_GetFlagStatus(dev_cfg->dma.tx.channel,DMA_FLAG_HTIF5)) {
		DMA_ClearFlag(dev_cfg->dma.tx.channel,DMA_FLAG_HTIF5);
	}
	else {

	}
}

#endif /* PIOS_INCLUDE_WAVE */
