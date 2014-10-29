/**
 ******************************************************************************
 * @addtogroup PIOS PIOS Core hardware abstraction layer
 * @{
 * @addtogroup   PIOS_RFM22B Radio Functions
 * @brief PIOS interface for RFM22B Radio
 * @{
 *
 * @file       pios_rfm22b_priv.h
 * @author     The OpenPilot Team, http://www.openpilot.org Copyright (C) 2012.
 * @brief      RFM22B private definitions.
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

#ifndef PIOS_RFM22B_PRIV_H
#define PIOS_RFM22B_PRIV_H

#include <pios.h>
#include <fifo_buffer.h>
#include <uavobjectmanager.h>
#include <oplinkstatus.h>
#include "pios_rfm22b.h"

// ************************************

#define RFM22B_MAX_PACKET_LEN                     64
#define RFM22B_NUM_CHANNELS                       250

// ************************************

#define RFM22_DEVICE_VERSION_V2                   0x02
#define RFM22_DEVICE_VERSION_A0                   0x04
#define RFM22_DEVICE_VERSION_B1                   0x06

// ************************************

#define BIT0                                      (1u << 0)
#define BIT1                                      (1u << 1)
#define BIT2                                      (1u << 2)
#define BIT3                                      (1u << 3)
#define BIT4                                      (1u << 4)
#define BIT5                                      (1u << 5)
#define BIT6                                      (1u << 6)
#define BIT7                                      (1u << 7)

// ************************************

#define RFM22_DEVICE_TYPE                         0x00    // R
#define RFM22_DT_MASK                             0x1F

#define RFM22_DEVICE_VERSION                      0x01    // R
#define RFM22_DV_MASK                             0x1F

#define RFM22_device_status                       0x02    // R
#define RFM22_ds_cps_mask                         0x03            // Chip Power State mask
#define RFM22_ds_cps_idle                         0x00            // IDLE Chip Power State
#define RFM22_ds_cps_rx                           0x01            // RX Chip Power State
#define RFM22_ds_cps_tx                           0x02            // TX Chip Power State
// #define RFM22_ds_lockdet						0x04		//
// #define RFM22_ds_freqerr						0x08		//
#define RFM22_ds_headerr                          0x10            // Header Error Status. Indicates if the received packet has a header check error
#define RFM22_ds_rxffem                           0x20            // RX FIFO Empty Status
#define RFM22_ds_ffunfl                           0x40            // RX/TX FIFO Underflow Status
#define RFM22_ds_ffovfl                           0x80            // RX/TX FIFO Overflow Status

#define RFM22_interrupt_status1                   0x03    // R
#define RFM22_is1_icrerror                        BIT0            // CRC Error. When set to 1 the cyclic redundancy check is failed.
#define RFM22_is1_ipkvalid                        BIT1            // Valid Packet Received.When set to 1 a valid packet has been received.
#define RFM22_is1_ipksent                         BIT2            // Packet Sent Interrupt. When set to1 a valid packet has been transmitted.
#define RFM22_is1_iext                            BIT3            // External Interrupt. When set to 1 an interrupt occurred on one of the GPIO�s if it is programmed so. The status can be checked in register 0Eh. See GPIOx Configuration section for the details.
#define RFM22_is1_irxffafull                      BIT4            // RX FIFO Almost Full.When set to 1 the RX FIFO has met its almost full threshold and needs to be read by the microcontroller.
#define RFM22_is1_ixtffaem                        BIT5            // TX FIFO Almost Empty. When set to 1 the TX FIFO is almost empty and needs to be filled.
#define RFM22_is1_itxffafull                      BIT6            // TX FIFO Almost Full. When set to 1 the TX FIFO has met its almost full threshold and needs to be transmitted.
#define RFM22_is1_ifferr                          BIT7            // FIFO Underflow/Overflow Error. When set to 1 the TX or RX FIFO has overflowed or underflowed.

#define RFM22_interrupt_status2                   0x04    // R
#define RFM22_is2_ipor                            BIT0            // Power-on-Reset (POR). When the chip detects a Power on Reset above the desired setting this bit will be set to 1.
#define RFM22_is2_ichiprdy                        BIT1            // Chip Ready (XTAL). When a chip ready event has been detected this bit will be set to 1.
#define RFM22_is2_ilbd                            BIT2            // Low Battery Detect. When a low battery event is been detected this bit will be set to 1. This interrupt event is saved even if it is not enabled by the mask register bit and causes an interrupt after it is enabled.
#define RFM22_is2_iwut                            BIT3            // Wake-Up-Timer. On the expiration of programmed wake-up timer this bit will be set to 1.
#define RFM22_is2_irssi                           BIT4            // RSSI. When RSSI level exceeds the programmed threshold this bit will be set to 1.
#define RFM22_is2_ipreainval                      BIT5            // Invalid Preamble Detected. When the preamble is not found within a period of time set by the invalid preamble detection threshold in Register 54h, this bit will be set to 1.
#define RFM22_is2_ipreaval                        BIT6            // Valid Preamble Detected. When a preamble is detected this bit will be set to 1.
#define RFM22_is2_iswdet                          BIT7            // Sync Word Detected. When a sync word is detected this bit will be set to 1.

#define RFM22_interrupt_enable1                   0x05    // R/W
#define RFM22_ie1_encrcerror                      BIT0            // Enable CRC Error. When set to 1 the CRC Error interrupt will be enabled.
#define RFM22_ie1_enpkvalid                       BIT1            // Enable Valid Packet Received. When ipkvalid = 1 the Valid Packet Received Interrupt will be enabled.
#define RFM22_ie1_enpksent                        BIT2            // Enable Packet Sent. When ipksent =1 the Packet Sense Interrupt will be enabled.
#define RFM22_ie1_enext                           BIT3            // Enable External Interrupt. When set to 1 the External Interrupt will be enabled.
#define RFM22_ie1_enrxffafull                     BIT4            // Enable RX FIFO Almost Full. When set to 1 the RX FIFO Almost Full interrupt will be enabled.
#define RFM22_ie1_entxffaem                       BIT5            // Enable TX FIFO Almost Empty. When set to 1 the TX FIFO Almost Empty interrupt will be enabled.
#define RFM22_ie1_entxffafull                     BIT6            // Enable TX FIFO Almost Full. When set to 1 the TX FIFO Almost Full interrupt will be enabled.
#define RFM22_ie1_enfferr                         BIT7            // Enable FIFO Underflow/Overflow. When set to 1 the FIFO Underflow/Overflow interrupt will be enabled.

#define RFM22_interrupt_enable2                   0x06    // R/W
#define RFM22_ie2_enpor                           BIT0            // Enable POR. When set to 1 the POR interrupt will be enabled.
#define RFM22_ie2_enchiprdy                       BIT1            // Enable Chip Ready (XTAL). When set to 1 the Chip Ready interrupt will be enabled.
#define RFM22_ie2_enlbd                           BIT2            // Enable Low Battery Detect. When set to 1 the Low Battery Detect interrupt will be enabled.
#define RFM22_ie2_enwut                           BIT3            // Enable Wake-Up Timer. When set to 1 the Wake-Up Timer interrupt will be enabled.
#define RFM22_ie2_enrssi                          BIT4            // Enable RSSI. When set to 1 the RSSI Interrupt will be enabled.
#define RFM22_ie2_enpreainval                     BIT5            // Enable Invalid Preamble Detected. When mpreadet =1 the Invalid Preamble Detected Interrupt will be enabled.
#define RFM22_ie2_enpreaval                       BIT6            // Enable Valid Preamble Detected. When mpreadet =1 the Valid Preamble Detected Interrupt will be enabled.
#define RFM22_ie2_enswdet                         BIT7            // Enable Sync Word Detected. When mpreadet =1 the Preamble Detected Interrupt will be enabled.

#define RFM22_op_and_func_ctrl1                   0x07    // R/W
#define RFM22_opfc1_xton                          0x01            // READY Mode (Xtal is ON).
#define RFM22_opfc1_pllon                         0x02            // TUNE Mode (PLL is ON). When pllon = 1 the PLL will remain enabled in Idle State. This will for faster turn-around time at the cost of increased current consumption in Idle State.
#define RFM22_opfc1_rxon                          0x04            // RX on in Manual Receiver Mode. Automatically cleared if Multiple Packets config. is disabled and a valid packet received.
#define RFM22_opfc1_txon                          0x08            // TX on in Manual Transmit Mode. Automatically cleared in FIFO mode once the packet is sent. Transmission can be aborted during packet transmission, however, when no data has been sent yet, transmission can only be aborted after the device is programmed to �unmodulated carrier� ("Register 71h. Modulation Mode Control 2").
#define RFM22_opfc1_x32ksel                       0x10            // 32,768 kHz Crystal Oscillator Select.   0: RC oscillator    1: 32 kHz crystal
#define RFM22_opfc1_enwt                          0x20            // Enable Wake-Up-Timer. Enabled when enwt = 1. If the Wake-up-Timer function is enabled it will operate in any mode and notify the microcontroller through the GPIO interrupt when the timer expires.
#define RFM22_opfc1_enlbd                         0x40            // Enable Low Battery Detect. When this bit is set to 1 the Low Battery Detector circuit and threshold comparison will be enabled.
#define RFM22_opfc1_swres                         0x80            // Software Register Reset Bit. This bit may be used to reset all registers simultaneously to a DEFAULT state, without the need for sequentially writing to each individual register. The RESET is accomplished by setting swres = 1. This bit will be automatically cleared.

#define RFM22_op_and_func_ctrl2                   0x08    // R/W
#define RFM22_opfc2_ffclrtx                       0x01            // TX FIFO Reset/Clear. This has to be a two writes operation: Setting ffclrtx =1 followed by ffclrtx = 0 will clear the contents of the TX FIFO.
#define RFM22_opfc2_ffclrrx                       0x02            // RX FIFO Reset/Clear. This has to be a two writes operation: Setting ffclrrx =1 followed by ffclrrx = 0 will clear the contents of the RX FIFO.
#define RFM22_opfc2_enldm                         0x04            // Enable Low Duty Cycle Mode. If this bit is set to 1 then the chip turns on the RX regularly. The frequency should be set in the Wake-Up Timer Period register, while the minimum ON time should be set in the Low-Duty Cycle Mode Duration register. The FIFO mode should be enabled also.
#define RFM22_opfc2_autotx                        0x08            // Automatic Transmission. When autotx = 1 the transceiver will enter automatically TX State when the FIFO is almost full. When the FIFO is empty it will automatically return to the Idle State.
#define RFM22_opfc2_rxmpk                         0x10            // RX Multi Packet. When the chip is selected to use FIFO Mode (dtmod[1:0]) and RX Packet Handling (enpacrx) then it will fill up the FIFO with multiple valid packets if this bit is set, otherwise the transceiver will automatically leave the RX State after the first valid packet has been received.
#define RFM22_opfc2_antdiv_mask                   0xE0            // Enable Antenna Diversity. The GPIO must be configured for Antenna Diversity for the algorithm to work properly.

#define RFM22_xtal_osc_load_cap                   0x09    // R/W
#define RFM22_xolc_xlc_mask                       0x7F            // Tuning Capacitance for the 30 MHz XTAL.
#define RFM22_xolc_xtalshft                       0x80            // Additional capacitance to course shift the frequency if xlc[6:0] is not sufficient. Not binary with xlc[6:0].

#define RFM22_cpu_output_clk                      0x0A    // R/W
#define RFM22_coc_30MHz                           0x00
#define RFM22_coc_15MHz                           0x01
#define RFM22_coc_10MHz                           0x02
#define RFM22_coc_4MHz                            0x03
#define RFM22_coc_3MHz                            0x04
#define RFM22_coc_2MHz                            0x05
#define RFM22_coc_1MHz                            0x06
#define RFM22_coc_32768Hz                         0x07
#define RFM22_coc_enlfc                           0x08
#define RFM22_coc_0cycle                          0x00
#define RFM22_coc_128cycles                       0x10
#define RFM22_coc_256cycles                       0x20
#define RFM22_coc_512cycles                       0x30

#define RFM22_gpio0_config                        0x0B    // R/W
#define RFM22_gpio0_config_por                    0x00            // Power-On-Reset (output)
#define RFM22_gpio0_config_wut                    0x01            // Wake-Up Timer: 1 when WUT has expired (output)
#define RFM22_gpio0_config_lbd                    0x02            // Low Battery Detect: 1 when battery is below threshold setting (output)
#define RFM22_gpio0_config_ddi                    0x03            // Direct Digital Input
#define RFM22_gpio0_config_eife                   0x04            // External Interrupt, falling edge (input)
#define RFM22_gpio0_config_eire                   0x05            // External Interrupt, rising edge (input)
#define RFM22_gpio0_config_eisc                   0x06            // External Interrupt, state change (input)
#define RFM22_gpio0_config_ai                     0x07            // ADC Analog Input
#define RFM22_gpio0_config_atni                   0x08            // Reserved (Analog Test N Input)
#define RFM22_gpio0_config_atpi                   0x09            // Reserved (Analog Test P Input)
#define RFM22_gpio0_config_ddo                    0x0A            // Direct Digital Output
#define RFM22_gpio0_config_dto                    0x0B            // Reserved (Digital Test Output)
#define RFM22_gpio0_config_atno                   0x0C            // Reserved (Analog Test N Output)
#define RFM22_gpio0_config_atpo                   0x0D            // Reserved (Analog Test P Output)
#define RFM22_gpio0_config_rv                     0xOE            // Reference Voltage (output)
#define RFM22_gpio0_config_dclk                   0x0F            // TX/RX Data CLK output to be used in conjunction with TX/RX Data pin (output)
#define RFM22_gpio0_config_txd                    0x10            // TX Data input for direct modulation (input)
#define RFM22_gpio0_config_err                    0x11            // External Retransmission Request (input)
#define RFM22_gpio0_config_txstate                0x12            // TX State (output)
#define RFM22_gpio0_config_txfifoaf               0x13            // TX FIFO Almost Full (output)
#define RFM22_gpio0_config_rxd                    0x14            // RX Data (output)
#define RFM22_gpio0_config_rxstate                0x15            // RX State (output)
#define RFM22_gpio0_config_rxfifoaf               0x16            // RX FIFO Almost Full (output)
#define RFM22_gpio0_config_antswt1                0x17            // Antenna 1 Switch used for antenna diversity (output)
#define RFM22_gpio0_config_antswt2                0x18            // Antenna 2 Switch used for antenna diversity (output)
#define RFM22_gpio0_config_vpd                    0x19            // Valid Preamble Detected (output)
#define RFM22_gpio0_config_ipd                    0x1A            // Invalid Preamble Detected (output)
#define RFM22_gpio0_config_swd                    0x1B            // Sync Word Detected (output)
#define RFM22_gpio0_config_cca                    0x1C            // Clear Channel Assessment (output)
#define RFM22_gpio0_config_vdd                    0x1D            // VDD
#define RFM22_gpio0_config_pup                    0x20
#define RFM22_gpio0_config_drv0                   0x00            // output drive level
#define RFM22_gpio0_config_drv1                   0x40            // output drive level
#define RFM22_gpio0_config_drv2                   0x80            // output drive level
#define RFM22_gpio0_config_drv3                   0xC0            // output drive level

#define RFM22_gpio1_config                        0x0C    // R/W
#define RFM22_gpio1_config_ipor                   0x00            // Inverted Power-On-Reset (output)
#define RFM22_gpio1_config_wut                    0x01            // Wake-Up Timer: 1 when WUT has expired (output)
#define RFM22_gpio1_config_lbd                    0x02            // Low Battery Detect: 1 when battery is below threshold setting (output)
#define RFM22_gpio1_config_ddi                    0x03            // Direct Digital Input
#define RFM22_gpio1_config_eife                   0x04            // External Interrupt, falling edge (input)
#define RFM22_gpio1_config_eire                   0x05            // External Interrupt, rising edge (input)
#define RFM22_gpio1_config_eisc                   0x06            // External Interrupt, state change (input)
#define RFM22_gpio1_config_ai                     0x07            // ADC Analog Input
#define RFM22_gpio1_config_atni                   0x08            // Reserved (Analog Test N Input)
#define RFM22_gpio1_config_atpi                   0x09            // Reserved (Analog Test P Input)
#define RFM22_gpio1_config_ddo                    0x0A            // Direct Digital Output
#define RFM22_gpio1_config_dto                    0x0B            // Reserved (Digital Test Output)
#define RFM22_gpio1_config_atno                   0x0C            // Reserved (Analog Test N Output)
#define RFM22_gpio1_config_atpo                   0x0D            // Reserved (Analog Test P Output)
#define RFM22_gpio1_config_rv                     0xOE            // Reference Voltage (output)
#define RFM22_gpio1_config_dclk                   0x0F            // TX/RX Data CLK output to be used in conjunction with TX/RX Data pin (output)
#define RFM22_gpio1_config_txd                    0x10            // TX Data input for direct modulation (input)
#define RFM22_gpio1_config_err                    0x11            // External Retransmission Request (input)
#define RFM22_gpio1_config_txstate                0x12            // TX State (output)
#define RFM22_gpio1_config_txfifoaf               0x13            // TX FIFO Almost Full (output)
#define RFM22_gpio1_config_rxd                    0x14            // RX Data (output)
#define RFM22_gpio1_config_rxstate                0x15            // RX State (output)
#define RFM22_gpio1_config_rxfifoaf               0x16            // RX FIFO Almost Full (output)
#define RFM22_gpio1_config_antswt1                0x17            // Antenna 1 Switch used for antenna diversity (output)
#define RFM22_gpio1_config_antswt2                0x18            // Antenna 2 Switch used for antenna diversity (output)
#define RFM22_gpio1_config_vpd                    0x19            // Valid Preamble Detected (output)
#define RFM22_gpio1_config_ipd                    0x1A            // Invalid Preamble Detected (output)
#define RFM22_gpio1_config_swd                    0x1B            // Sync Word Detected (output)
#define RFM22_gpio1_config_cca                    0x1C            // Clear Channel Assessment (output)
#define RFM22_gpio1_config_vdd                    0x1D            // VDD
#define RFM22_gpio1_config_pup                    0x20
#define RFM22_gpio1_config_drv0                   0x00            // output drive level
#define RFM22_gpio1_config_drv1                   0x40            // output drive level
#define RFM22_gpio1_config_drv2                   0x80            // output drive level
#define RFM22_gpio1_config_drv3                   0xC0            // output drive level

#define RFM22_gpio2_config                        0x0D    // R/W
#define RFM22_gpio2_config_mc                     0x00            // Microcontroller Clock (output)
#define RFM22_gpio2_config_wut                    0x01            // Wake-Up Timer: 1 when WUT has expired (output)
#define RFM22_gpio2_config_lbd                    0x02            // Low Battery Detect: 1 when battery is below threshold setting (output)
#define RFM22_gpio2_config_ddi                    0x03            // Direct Digital Input
#define RFM22_gpio2_config_eife                   0x04            // External Interrupt, falling edge (input)
#define RFM22_gpio2_config_eire                   0x05            // External Interrupt, rising edge (input)
#define RFM22_gpio2_config_eisc                   0x06            // External Interrupt, state change (input)
#define RFM22_gpio2_config_ai                     0x07            // ADC Analog Input
#define RFM22_gpio2_config_atni                   0x08            // Reserved (Analog Test N Input)
#define RFM22_gpio2_config_atpi                   0x09            // Reserved (Analog Test P Input)
#define RFM22_gpio2_config_ddo                    0x0A            // Direct Digital Output
#define RFM22_gpio2_config_dto                    0x0B            // Reserved (Digital Test Output)
#define RFM22_gpio2_config_atno                   0x0C            // Reserved (Analog Test N Output)
#define RFM22_gpio2_config_atpo                   0x0D            // Reserved (Analog Test P Output)
#define RFM22_gpio2_config_rv                     0xOE            // Reference Voltage (output)
#define RFM22_gpio2_config_dclk                   0x0F            // TX/RX Data CLK output to be used in conjunction with TX/RX Data pin (output)
#define RFM22_gpio2_config_txd                    0x10            // TX Data input for direct modulation (input)
#define RFM22_gpio2_config_err                    0x11            // External Retransmission Request (input)
#define RFM22_gpio2_config_txstate                0x12            // TX State (output)
#define RFM22_gpio2_config_txfifoaf               0x13            // TX FIFO Almost Full (output)
#define RFM22_gpio2_config_rxd                    0x14            // RX Data (output)
#define RFM22_gpio2_config_rxstate                0x15            // RX State (output)
#define RFM22_gpio2_config_rxfifoaf               0x16            // RX FIFO Almost Full (output)
#define RFM22_gpio2_config_antswt1                0x17            // Antenna 1 Switch used for antenna diversity (output)
#define RFM22_gpio2_config_antswt2                0x18            // Antenna 2 Switch used for antenna diversity (output)
#define RFM22_gpio2_config_vpd                    0x19            // Valid Preamble Detected (output)
#define RFM22_gpio2_config_ipd                    0x1A            // Invalid Preamble Detected (output)
#define RFM22_gpio2_config_swd                    0x1B            // Sync Word Detected (output)
#define RFM22_gpio2_config_cca                    0x1C            // Clear Channel Assessment (output)
#define RFM22_gpio2_config_vdd                    0x1D            // VDD
#define RFM22_gpio2_config_pup                    0x20
#define RFM22_gpio2_config_drv0                   0x00            // output drive level
#define RFM22_gpio2_config_drv1                   0x40            // output drive level
#define RFM22_gpio2_config_drv2                   0x80            // output drive level
#define RFM22_gpio2_config_drv3                   0xC0            // output drive level

#define RFM22_io_port_config                      0x0E    // R/W
#define RFM22_io_port_extitst2                    0x40            // External Interrupt Status. If the GPIO2 is programmed to be external interrupt sources then the status can be read here.
#define RFM22_io_port_extitst1                    0x20            // External Interrupt Status. If the GPIO1 is programmed to be external interrupt sources then the status can be read here.
#define RFM22_io_port_extitst0                    0x10            // External Interrupt Status. If the GPIO0 is programmed to be external interrupt sources then the status can be read here.
#define RFM22_io_port_itsdo                       0x08            // Interrupt Request Output on the SDO Pin. nIRQ output is present on the SDO pin if this bit is set and the nSEL input is inactive (high).
#define RFM22_io_port_dio2                        0x04            // Direct I/O for GPIO2. If the GPIO2 is configured to be a direct output then the value on the GPIO pin can be set here. If the GPIO2 is configured to be a direct input then the value of the pin can be read here.
#define RFM22_io_port_dio1                        0x02            // Direct I/O for GPIO1. If the GPIO1 is configured to be a direct output then the value on the GPIO pin can be set here. If the GPIO1 is configured to be a direct input then the value of the pin can be read here.
#define RFM22_io_port_dio0                        0x01            // Direct I/O for GPIO0. If the GPIO0 is configured to be a direct output then the value on the GPIO pin can be set here. If the GPIO0 is configured to be a direct input then the value of the pin can be read here.
#define RFM22_io_port_default                     0x00            // GPIO pins are default

#define RFM22_adc_config                          0x0F    // R/W
#define RFM22_ac_adcgain0                         0x00
#define RFM22_ac_adcgain1                         0x01
#define RFM22_ac_adcgain2                         0x02
#define RFM22_ac_adcgain3                         0x03
#define RFM22_ac_adcref_bg                        0x00
#define RFM22_ac_adcref_vdd3                      0x08
#define RFM22_ac_adcref_vdd2                      0x0C
#define RFM22_ac_adcsel_temp_sensor               0x00
#define RFM22_ac_adcsel_gpio0                     0x10
#define RFM22_ac_adcsel_gpio1                     0x20
#define RFM22_ac_adcsel_gpio2                     0x30
#define RFM22_ac_adcsel_gpio01                    0x40
#define RFM22_ac_adcsel_gpio12                    0x50
#define RFM22_ac_adcsel_gpio02                    0x60
#define RFM22_ac_adcsel_gpio_gnd                  0x70
#define RFM22_ac_adcstartbusy                     0x80

#define RFM22_adc_sensor_amp_offset               0x10    // R/W
#define RFM22_asao_adcoffs_mask                   0x0F            // ADC Sensor Amplifier Offset. The offset can be calculated as Offset = adcoffs[2:0] x VDD/1000; MSB = adcoffs[3] = Sign bit.

#define RFM22_adc_value                           0x11    // R .. Internal 8 bit ADC Output Value.

#define RFM22_temp_sensor_calib                   0x12    // R/W
#define RFM22_tsc_tstrim_mask                     0x0F            // Temperature Sensor Trim Value.
#define RFM22_tsc_entstrim                        0x10            // Temperature Sensor Trim Enable.
#define RFM22_tsc_entsoffs                        0x20            // Temperature Sensor Offset to Convert from K to �C.
#define RFM22_tsc_tsrange0                        0x00            // Temperature Sensor Range Selection. �64C to +64C 0.5C resolution
#define RFM22_tsc_tsrange1                        0x40            // -40 to +85C with 1.0C resolution
#define RFM22_tsc_tsrange2                        0x80            // 0C to 85C with 0.5C resolution
#define RFM22_tsc_tsrange3                        0xC0            // -40F to 216F with 1.0F resolution

#define RFM22_temp_value_offset                   0x13    // R/W

#define RFM22_wakeup_timer_period1                0x14    // R/W
#define RFM22_wakeup_timer_period2                0x15    // R/W
#define RFM22_wakeup_timer_period3                0x16    // R/W

#define RFM22_wakeup_timer_value1                 0x17    // R
#define RFM22_wakeup_timer_value2                 0x18    // R

#define RFM22_low_dutycycle_mode_duration         0x19    // R/W
#define RFM22_low_battery_detector_threshold      0x1A    // R/W

#define RFM22_battery_volateg_level               0x1B    // R

#define RFM22_if_filter_bandwidth                 0x1C    // R/W
#define RFM22_iffbw_filset_mask                   0x0F
#define RFM22_iffbw_ndec_exp_mask                 0x70
#define RFM22_iffbw_dwn3_bypass                   0x80

#define RFM22_afc_loop_gearshift_override         0x1D    // R/W
#define RFM22_afc_lp_gs_ovrd_afcgearl_mask        0x07            // AFC Low Gear Setting.
#define RFM22_afc_lp_gs_ovrd_afcgearh_mask        0x38            // AFC High Gear Setting.
#define RFM22_afc_lp_gs_ovrd_enafc                0x40            // AFC Enable.
#define RFM22_afc_lp_gs_ovrd_afcbd                0x80            // If set, the tolerated AFC frequency error will be halved.

#define RFM22_afc_timing_control                  0x1E    // R/W

#define RFM22_clk_recovery_gearshift_override     0x1F    // R/W
#define RFM22_clk_recovery_oversampling_ratio     0x20    // R/W
#define RFM22_clk_recovery_offset2                0x21    // R/W
#define RFM22_clk_recovery_offset1                0x22    // R/W
#define RFM22_clk_recovery_offset0                0x23    // R/W
#define RFM22_clk_recovery_timing_loop_gain1      0x24    // R/W
#define RFM22_clk_recovery_timing_loop_gain0      0x25    // R/W

#define RFM22_rssi                                0x26    // R
#define RFM22_rssi_threshold_clear_chan_indicator 0x27 // R/W

#define RFM22_antenna_diversity_register1         0x28    // R
#define RFM22_antenna_diversity_register2         0x29    // R

#define RFM22_afc_limiter                         0x2A    // R/W .. AFC_pull_in_range = �AFCLimiter[7:0] x (hbsel+1) x 625 Hz

#define RFM22_afc_correction_read                 0x2B    // R

#define RFM22_ook_counter_value1                  0x2C    // R/W
#define RFM22_ook_counter_value2                  0x2D    // R/W

#define RFM22_slicer_peak_hold                    0x2E    // R/W

#define RFM22_data_access_control                 0x30    // R/W
#define RFM22_dac_crc_ccitt                       0x00            //
#define RFM22_dac_crc_crc16                       0x01            //
#define RFM22_dac_crc_iec16                       0x02            //
#define RFM22_dac_crc_biacheva                    0x03            //
#define RFM22_dac_encrc                           0x04            // CRC Enable. Cyclic Redundancy Check generation is enabled if this bit is set.
#define RFM22_dac_enpactx                         0x08            // Enable Packet TX Handling. If FIFO Mode (dtmod = 10) is being used automatic packet handling may be enabled. Setting enpactx = 1 will enable automatic packet handling in the TX path. Register 30�4D allow for various configurations of the packet structure. Setting enpactx = 0 will not do any packet handling in the TX path. It will only transmit what is loaded to the FIFO.
#define RFM22_dac_skip2ph                         0x10            // Skip 2nd Phase of Preamble Detection. If set, we skip the second phase of the preamble detection (under certain conditions) if antenna diversity is enabled.
#define RFM22_dac_crcdonly                        0x20            // CRC Data Only Enable. When this bit is set to 1 the CRC is calculated on and checked against the packet data fields only.
#define RFM22_dac_lsbfrst                         0x40            // LSB First Enable. The LSB of the data will be transmitted/received first if this bit is set.
#define RFM22_dac_enpacrx                         0x80            // Enable Packet RX Handling. If FIFO Mode (dtmod = 10) is being used automatic packet handling may be enabled. Setting enpacrx = 1 will enable automatic packet handling in the RX path. Register 30�4D allow for various configurations of the packet structure. Setting enpacrx = 0 will not do any packet handling in the RX path. It will only receive everything after the sync word and fill up the RX FIFO.

#define RFM22_ezmac_status                        0x31    // R
#define RFM22_ezmac_status_pksent                 0x01            // Packet Sent. A 1 a packet has been sent by the radio. (Same bit as in register 03, but reading it does not reset the IRQ)
#define RFM22_ezmac_status_pktx                   0x02            // Packet Transmitting. When 1 the radio is currently transmitting a packet.
#define RFM22_ezmac_status_crcerror               0x04            // CRC Error. When 1 a Cyclic Redundancy Check error has been detected. (Same bit as in register 03, but reading it does not reset the IRQ)
#define RFM22_ezmac_status_pkvalid                0x08            // Valid Packet Received. When a 1 a valid packet has been received by the receiver. (Same bit as in register 03, but reading it does not reset the IRQ)
#define RFM22_ezmac_status_pkrx                   0x10            // Packet Receiving. When 1 the radio is currently receiving a valid packet.
#define RFM22_ezmac_status_pksrch                 0x20            // Packet Searching. When 1 the radio is searching for a valid packet.
#define RFM22_ezmac_status_rxcrc1                 0x40            // If high, it indicates the last CRC received is all one�s. May indicated Transmitter underflow in case of CRC error.

#define RFM22_header_control1                     0x32    // R/W
#define RFM22_header_cntl1_bcen_none              0x00            // No broadcast address enable.
#define RFM22_header_cntl1_bcen_0                 0x10            // Broadcast address enable for header byte 0.
#define RFM22_header_cntl1_bcen_1                 0x20            // Broadcast address enable for header byte 1.
#define RFM22_header_cntl1_bcen_2                 0x40            // Broadcast address enable for header byte 2.
#define RFM22_header_cntl1_bcen_3                 0x80            // Broadcast address enable for header byte 3.
#define RFM22_header_cntl1_hdch_none              0x00            // No Received Header check
#define RFM22_header_cntl1_hdch_0                 0x01            // Received Header check for byte 0.
#define RFM22_header_cntl1_hdch_1                 0x02            // Received Header check for byte 1.
#define RFM22_header_cntl1_hdch_2                 0x04            // Received Header check for byte 2.
#define RFM22_header_cntl1_hdch_3                 0x08            // Received Header check for byte 3.

#define RFM22_header_control2                     0x33    // R/W
#define RFM22_header_cntl2_prealen                0x01            // MSB of Preamble Length. See register Preamble Length.
#define RFM22_header_cntl2_synclen_3              0x00            // Synchronization Word 3
#define RFM22_header_cntl2_synclen_32             0x02            // Synchronization Word 3 followed by 2
#define RFM22_header_cntl2_synclen_321            0x04            // Synchronization Word 3 followed by 2 followed by 1
#define RFM22_header_cntl2_synclen_3210           0x06            // Synchronization Word 3 followed by 2 followed by 1 followed by 0
#define RFM22_header_cntl2_fixpklen               0x08            // Fix Packet Length. When fixpklen = 1 the packet length (pklen[7:0]) is not included in the header. When fixpklen = 0 the packet length is included in the header.
#define RFM22_header_cntl2_hdlen_none             0x00            // no header
#define RFM22_header_cntl2_hdlen_3                0x10            // header 3
#define RFM22_header_cntl2_hdlen_32               0x20            // header 3 and 2
#define RFM22_header_cntl2_hdlen_321              0x30            // header 3 and 2 and 1
#define RFM22_header_cntl2_hdlen_3210             0x40            // header 3 and 2 and 1 and 0
#define RFM22_header_cntl2_skipsyn                0x80            // If high, the system will ignore the syncword search timeout reset. The chip will not return to searching for Preamble, but instead will remain searching for Sync word.

#define RFM22_preamble_length                     0x34    // R/W

#define RFM22_preamble_detection_ctrl1            0x35    // R/W
#define RFM22_pre_det_ctrl1_preath_mask           0xF8            // Number of nibbles processed during detection.
#define RFM22_pre_det_ctrl1_rssi_offset_mask      0x07            // Value added as offset to RSSI calculation. Every increment in this register results in an increment of +4 dB in the RSSI.

#define RFM22_sync_word3                          0x36    // R/W
#define RFM22_sync_word2                          0x37    // R/W
#define RFM22_sync_word1                          0x38    // R/W
#define RFM22_sync_word0                          0x39    // R/W

#define RFM22_transmit_header3                    0x3A    // R/W
#define RFM22_transmit_header2                    0x3B    // R/W
#define RFM22_transmit_header1                    0x3C    // R/W
#define RFM22_transmit_header0                    0x3D    // R/W

#define RFM22_transmit_packet_length              0x3E    // R/W

#define RFM22_check_header3                       0x3F    // R/W
#define RFM22_check_header2                       0x40    // R/W
#define RFM22_check_header1                       0x41    // R/W
#define RFM22_check_header0                       0x42    // R/W

#define RFM22_header_enable3                      0x43    // R/W
#define RFM22_header_enable2                      0x44    // R/W
#define RFM22_header_enable1                      0x45    // R/W
#define RFM22_header_enable0                      0x46    // R/W

#define RFM22_received_header3                    0x47    // R
#define RFM22_received_header2                    0x48    // R
#define RFM22_received_header1                    0x49    // R
#define RFM22_received_header0                    0x4A    // R

#define RFM22_received_packet_length              0x4B    // R

#define RFM22_adc8_control                        0x4F    // R/W

#define RFM22_channel_filter_coeff_addr           0x60    // R/W
#define RFM22_ch_fil_coeff_ad_inv_pre_th_mask     0xF0            //
#define RFM22_ch_fil_coeff_ad_chfiladd_mask       0x0F            // Channel Filter Coefficient Look-up Table Address. The address for channel filter coefficients used in the RX path.

#define RFM22_xtal_osc_por_ctrl                   0x62    // R/W
#define RFM22_xtal_osc_por_ctrl_pwst_mask         0xE0            // Internal Power States of the Chip.
#define RFM22_xtal_osc_por_ctrl_clkhyst           0x10            // Clock Hysteresis Setting.
#define RFM22_xtal_osc_por_ctrl_enbias2x          0x08            // 2 Times Higher Bias Current Enable.
#define RFM22_xtal_osc_por_ctrl_enamp2x           0x04            // 2 Times Higher Amplification Enable.
#define RFM22_xtal_osc_por_ctrl_bufovr            0x02            // Output Buffer Enable Override.
#define RFM22_xtal_osc_por_ctrl_enbuf             0x01            // Output Buffer Enable.

#define RFM22_agc_override1                       0x69    // R/W
#define RFM22_agc_ovr1_sgi                        0x40            // AGC Loop, Set Gain Increase. If set to 0 then gain increasing will not be allowed. If set to 1 then gain increasing is allowed, default is 0.
#define RFM22_agc_ovr1_agcen                      0x20            // Automatic Gain Control Enable. When this bit is set then the result of the control can be read out from bits [4:0], otherwise the gain can be controlled manually by writing into bits [4:0].
#define RFM22_agc_ovr1_lnagain                    0x10            // LNA Gain Select. 0 = min gain = 5dB, 1 = max gain = 25 dB.
#define RFM22_agc_ovr1_pga_mask                   0x0F            // PGA Gain Override Value.

#define RFM22_tx_power                            0x6D    // R/W
#define RFM22_tx_pwr_lna_sw                       0x08            // LNA Switch Controller. If set, lna_sw control from the digital will go high during TX modes, and low during other times. If reset, the digital control signal is low at all times.

#define RFM22_tx_data_rate1                       0x6E    // R/W
#define RFM22_tx_data_rate0                       0x6F    // R/W

#define RFM22_modulation_mode_control1            0x70    // R/W
#define RFM22_mmc1_enwhite                        0x01            // Data Whitening is Enabled if this bit is set.
#define RFM22_mmc1_enmanch                        0x02            // Manchester Coding is Enabled if this bit is set.
#define RFM22_mmc1_enmaninv                       0x04            // Manchester Data Inversion is Enabled if this bit is set.
#define RFM22_mmc1_manppol                        0x08            // Manchester Preamble Polarity (will transmit a series of 1 if set, or series of 0 if reset).
#define RFM22_mmc1_enphpwdn                       0x10            // If set, the Packet Handler will be powered down when chip is in low power mode.
#define RFM22_mmc1_txdtrtscale                    0x20            // This bit should be set for Data Rates below 30 kbps.

#define RFM22_modulation_mode_control2            0x71    // R/W
#define RFM22_mmc2_modtyp_mask                    0x03            // Modulation type.
#define RFM22_mmc2_modtyp_none                    0x00            //
#define RFM22_mmc2_modtyp_ook                     0x01            //
#define RFM22_mmc2_modtyp_fsk                     0x02            //
#define RFM22_mmc2_modtyp_gfsk                    0x03            //
#define RFM22_mmc2_fd                             0x04            // MSB of Frequency Deviation Setting, see "Register 72h. Frequency Deviation".
#define RFM22_mmc2_eninv                          0x08            // Invert TX and RX Data.
#define RFM22_mmc2_dtmod_mask                     0x30            // Modulation source.
#define RFM22_mmc2_dtmod_dm_gpio                  0x00            //
#define RFM22_mmc2_dtmod_dm_sdi                   0x10            //
#define RFM22_mmc2_dtmod_fifo                     0x20            //
#define RFM22_mmc2_dtmod_pn9                      0x30            //
#define RFM22_mmc2_trclk_mask                     0xC0            // TX Data Clock Configuration.
#define RFM22_mmc2_trclk_clk_none                 0x00            //
#define RFM22_mmc2_trclk_clk_gpio                 0x40            //
#define RFM22_mmc2_trclk_clk_sdo                  0x80            //
#define RFM22_mmc2_trclk_clk_nirq                 0xC0            //

#define RFM22_frequency_deviation                 0x72    // R/W

#define RFM22_frequency_offset1                   0x73    // R/W
#define RFM22_frequency_offset2                   0x74    // R/W

#define RFM22_frequency_band_select               0x75    // R/W
#define RFM22_fb_mask                             0x1F
#define RFM22_fbs_hbsel                           0x20
#define RFM22_fbs_sbse                            0x40

#define RFM22_nominal_carrier_frequency1          0x76    // R/W
#define RFM22_nominal_carrier_frequency0          0x77    // R/W

#define RFM22_frequency_hopping_channel_select    0x79    // R/W
#define RFM22_frequency_hopping_step_size         0x7A    // R/W

#define RFM22_tx_fifo_control1                    0x7C    // R/W .. TX FIFO Almost Full Threshold (0 - 63)
#define RFM22_tx_fifo_control1_mask               0x3F

#define RFM22_tx_fifo_control2                    0x7D    // R/W .. TX FIFO Almost Empty Threshold (0 - 63)
#define RFM22_tx_fifo_control2_mask               0x3F

#define RFM22_rx_fifo_control                     0x7E    // R/W .. RX FIFO Almost Full Threshold (0 - 63)
#define RFM22_rx_fifo_control_mask                0x3F

#define RFM22_fifo_access                         0x7F    // R/W


// External type definitions

typedef int16_t (*t_rfm22_TxDataByteCallback)(void);
typedef bool (*t_rfm22_RxDataCallback)(void *data, uint8_t len);
enum pios_rfm22b_dev_magic {
    PIOS_RFM22B_DEV_MAGIC = 0x68e971b6,
};

enum pios_radio_state {
    RADIO_STATE_UNINITIALIZED,
    RADIO_STATE_INITIALIZING,
    RADIO_STATE_RX_MODE,
    RADIO_STATE_RX_DATA,
    RADIO_STATE_RX_FAILURE,
    RADIO_STATE_TX_START,
    RADIO_STATE_TX_DATA,
    RADIO_STATE_TX_FAILURE,
    RADIO_STATE_TIMEOUT,
    RADIO_STATE_ERROR,
    RADIO_STATE_FATAL_ERROR,

    RADIO_STATE_NUM_STATES // Must be last
};

enum pios_radio_event {
    RADIO_EVENT_DEFAULT,
    RADIO_EVENT_INT_RECEIVED,
    RADIO_EVENT_INITIALIZE,
    RADIO_EVENT_INITIALIZED,
    RADIO_EVENT_RX_MODE,
    RADIO_EVENT_RX_COMPLETE,
    RADIO_EVENT_TX_START,
    RADIO_EVENT_TIMEOUT,
    RADIO_EVENT_ERROR,
    RADIO_EVENT_FATAL_ERROR,

    RADIO_EVENT_NUM_EVENTS // Must be last
};

enum pios_rfm22b_state {
    RFM22B_STATE_INITIALIZING,
    RFM22B_STATE_TRANSITION,
    RFM22B_STATE_RX_WAIT,
    RFM22B_STATE_RX_WAIT_SYNC,
    RFM22B_STATE_RX_MODE,
    RFM22B_STATE_TX_MODE,
    RFM22B_STATE_TRANSMITTING,

    RFM22B_STATE_NUM_STATES // Must be last
};

#define RFM22B_RX_PACKET_STATS_LEN 4
enum pios_rfm22b_rx_packet_status {
    RADIO_GOOD_RX_PACKET      = 0x00,
    RADIO_CORRECTED_RX_PACKET = 0x01,
    RADIO_ERROR_RX_PACKET     = 0x2,
    RADIO_FAILURE_RX_PACKET    = 0x3
};

typedef struct {
    uint32_t pairID;
    int8_t   rssi;
    int8_t   afc_correction;
    uint8_t  lastContact;
} rfm22b_pair_stats;

enum pios_rfm22b_chip_power_state {
    RFM22B_IDLE_STATE    = 0x00,
    RFM22B_RX_STATE      = 0x01,
    RFM22B_TX_STATE      = 0x10,
    RFM22B_INVALID_STATE = 0x11
};

// Device Status
typedef union {
    struct {
        uint8_t state : 2;
        bool    frequency_error : 1;
        bool    header_error : 1;
        bool    rx_fifo_empty : 1;
        bool    fifo_underflow : 1;
        bool    fifo_overflow : 1;
    };
    uint8_t raw;
} rfm22b_device_status_reg;

// EzMAC Status
typedef union {
    struct {
        bool packet_sent : 1;
        bool packet_transmitting : 1;
        bool crc_error : 1;
        bool valid_packet_received : 1;
        bool packet_receiving : 1;
        bool packet_searching : 1;
        bool crc_is_all_ones : 1;
        bool reserved;
    };
    uint8_t raw;
} rfm22b_ezmac_status_reg;

// Interrrupt Status Register 1
typedef union {
    struct {
        bool crc_error : 1;
        bool valid_packet_received : 1;
        bool packet_sent_interrupt : 1;
        bool external_interrupt : 1;
        bool rx_fifo_almost_full : 1;
        bool tx_fifo_almost_empty : 1;
        bool tx_fifo_almost_full : 1;
        bool fifo_underoverflow_error : 1;
    };
    uint8_t raw;
} rfm22b_int_status_1;

// Interrupt Status Register 2
typedef union {
    struct {
        bool poweron_reset : 1;
        bool chip_ready : 1;
        bool low_battery_detect : 1;
        bool wakeup_timer : 1;
        bool rssi_above_threshold : 1;
        bool invalid_preamble_detected : 1;
        bool valid_preamble_detected : 1;
        bool sync_word_detected : 1;
    };
    uint8_t raw;
} rfm22b_int_status_2;

typedef struct {
    rfm22b_device_status_reg device_status;
    rfm22b_device_status_reg ezmac_status;
    rfm22b_int_status_1 int_status_1;
    rfm22b_int_status_2 int_status_2;
} rfm22b_device_status;

struct pios_rfm22b_dev {
    enum pios_rfm22b_dev_magic magic;
    struct pios_rfm22b_cfg     cfg;

    // The SPI bus information
    uint32_t    spi_id;
    uint32_t    slave_num;

    // Should this modem ack as a coordinator.
    bool        coordinator;

    // The device ID
    uint32_t    deviceID;

    // The coodinator ID (0 if this modem is a coordinator).
    uint32_t    coordinatorID;

    // The task handle
    xTaskHandle taskHandle;

    // The potential paired statistics
    rfm22b_pair_stats pair_stats[OPLINKSTATUS_PAIRIDS_NUMELEM];

    // ISR pending semaphore
    xSemaphoreHandle  isrPending;

    // The COM callback functions.
    pios_com_callback rx_in_cb;
    uint32_t rx_in_context;
    pios_com_callback tx_out_cb;
    uint32_t tx_out_context;

    // the transmit power to use for data transmissions
    uint8_t  tx_power;

    // The RF datarate lookup index.
    uint8_t  datarate;

    // The radio state machine state
    enum pios_radio_state state;

    // The event queue handle
    xQueueHandle eventQueue;

    // The device status registers.
    rfm22b_device_status status_regs;

    // The error statistics counters
    uint16_t prev_rx_seq_num;
    uint32_t rx_packet_stats[RFM22B_RX_PACKET_STATS_LEN];

    // The RFM22B state machine state
    enum pios_rfm22b_state rfm22b_state;

    // The packet statistics
    struct rfm22b_stats    stats;

    // Stats
    uint16_t errors;

    // RSSI in dBm
    int8_t   rssi_dBm;

    // The tx data packet
    uint8_t  tx_packet[RFM22B_MAX_PACKET_LEN];
    // The current tx packet
    uint8_t  *tx_packet_handle;
    // The tx data read index
    uint16_t tx_data_rd;
    // The tx data write index
    uint16_t tx_data_wr;
    // The tx packet sequence number
    uint16_t tx_seq;

    // The rx data packet
    uint8_t  rx_packet[RFM22B_MAX_PACKET_LEN];
    // The rx data packet
    uint8_t  *rx_packet_handle;
    // The receive buffer write index
    uint16_t rx_buffer_wr;
    // The receive buffer write index
    uint16_t rx_packet_len;

    // The PPM buffer
    int16_t  ppm[RFM22B_PPM_NUM_CHANNELS];
    // The PPM packet received callback.
    PPMReceivedCallback ppm_callback;

    // The id that the packet was received from
    uint32_t     rx_destination_id;
    // The maximum packet length (including header, etc.)
    uint8_t      max_packet_len;
    // The packet transmit time in ms.
    uint8_t      packet_time;
    // Do all packets originate from the coordinator modem?
    bool         one_way_link;
    // Should this modem send PPM data?
    bool         ppm_send_mode;
    // Should this modem receive PPM data?
    bool         ppm_recv_mode;
    // Are we sending / receiving only PPM data?
    bool         ppm_only_mode;

    // The channel list
    uint8_t      channels[RFM22B_NUM_CHANNELS];
    // The number of frequency hopping channels.
    uint8_t      num_channels;
    // The frequency hopping step size
    float        frequency_step_size;
    // current frequency hop channel
    uint8_t      channel;
    // current frequency hop channel index
    uint8_t      channel_index;
    // afc correction reading (in Hz)
    int8_t       afc_correction_Hz;

    // The packet timers.
    portTickType packet_start_ticks;
    portTickType tx_complete_ticks;
    portTickType time_delta;
    portTickType last_conntact;
};


// External function definitions

bool PIOS_RFM22_EXT_Int(void);
bool PIOS_RFM22B_Validate(struct pios_rfm22b_dev *rfm22b_dev);

// Global variable definitions

extern const struct pios_com_driver pios_rfm22b_com_driver;

#endif /* PIOS_RFM22B_PRIV_H */

/**
 * @}
 * @}
 */
