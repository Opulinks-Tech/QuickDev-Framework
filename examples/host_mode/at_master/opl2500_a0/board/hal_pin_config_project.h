/* *****************************************************************************
 *  Copyright 2017 - 2021, Opulinks Technology Ltd.
 *  ----------------------------------------------------------------------------
 *  Statement:
 *  ----------
 *  This software is protected by Copyright and the information contained
 *  herein is confidential. The software may not be copied and the information
 *  contained herein may not be used or disclosed except with the written
 *  permission of Opulinks Technology Ltd. (C) 2021
 ******************************************************************************/

/******************************************************************************
*  Filename:
*  ---------
*  hal_pin_config_project.h
*
*  Project:
*  --------
*  OPL2500 series
*
*  Description:
*  ------------
*  This include file defines the patch proto-types of system functions
*  Include sys-reg and AOS domains.
*
*  Author:
*  -------
*  FW team
******************************************************************************/

#ifndef __HAL_PIN_CONFIG_PROJECT_H__
#define __HAL_PIN_CONFIG_PROJECT_H__

#ifdef __cplusplus
extern "C" {
#endif

/***********************
Head Block of The File
***********************/
// Sec 0: Comment block of the file

// Sec 1: Include File
#include <stdint.h>
#include "hal_pin_def.h"

// Sec 2: Constant Definitions, Imported Symbols, miscellaneous

/* Select Main UART IO 0/2 UART mode. For switching UART use */
#define HAL_PIN_MAIN_UART_MODE_PATCH   PIN_MAIN_UART_MODE_AT // PIN_MAIN_UART_MODE_AT
                                                             // PIN_MAIN_UART_MODE_DBG


/* ***********************************************************************************************
 * IO type select. Select IO with driving current and Input mode.
 * e.g. (PIN_TYPE_PWM5_IO14      | PIN_DRVCRNT_IO14_4mA  | PIN_INMODE_IO14_PULL_UP)
 *      (PIN_TYPE_SPI0_IO1_IO15  | PIN_DRVCRNT_IO15_2mA  | PIN_INMODE_IO15_FLOATING)
 *      (PIN_TYPE_I2C_SDA_IO_IO9 | PIN_DRVCRNT_IO9_2mA   | PIN_INMODE_IO9_PULL_UP)
 */
#define HAL_PIN_TYPE_PATCH_IO_0               (PIN_TYPE_UART1_TXD_IO0        | PIN_DRVCRNT_IO0_2mA   | PIN_INMODE_IO0_FLOATING)
                                                /* ***********************************
                                                 *     PIN_TYPE_APS_SWDIO_IO0
                                                 *     PIN_TYPE_APS_UART_TXD_IO0
                                                 *     PIN_TYPE_DBG0_IO0
                                                 *     PIN_TYPE_GPIO_DISABLE_IO0
                                                 *     PIN_TYPE_GPIO_IN_IO0
                                                 *     PIN_TYPE_GPIO_OUT_HIGH_IO0
                                                 *     PIN_TYPE_GPIO_OUT_LOW_IO0
                                                 *     PIN_TYPE_I2C_SCL_IO_IO0
                                                 *     PIN_TYPE_I2S_SCLK_IO0
                                                 *     PIN_TYPE_MSQ_SWCLK_IO0
                                                 *     PIN_TYPE_MSQ_UART_TXD_IO0
                                                 *     PIN_TYPE_SPI0_CS3_IO0
                                                 *     PIN_TYPE_SPI1_CS3_IO0
                                                 *     PIN_TYPE_SPI3_IO3_IO0
                                                 *     PIN_TYPE_SPI3_RXD3_IO0
                                                 *     PIN_TYPE_SPI3_TXD3_IO0
                                                 *     PIN_TYPE_TMS_IO0
                                                 *     PIN_TYPE_UART0_TXD_IO0
                                                 *     PIN_TYPE_UART1_TXD_IO0
                                                 * ***********************************
                                                 *     PIN_DRVCRNT_IO0_2mA
                                                 *     PIN_DRVCRNT_IO0_4mA
                                                 * ***********************************
                                                 *     PIN_INMODE_IO0_FLOATING
                                                 *     PIN_INMODE_IO0_PULL_UP
                                                 * *********************************** */


#define HAL_PIN_TYPE_PATCH_IO_1               (PIN_TYPE_APS_UART_RXD_IO1           | PIN_DRVCRNT_IO1_2mA   | PIN_INMODE_IO1_PULL_UP)
                                                /* ***********************************
                                                 *     PIN_TYPE_APS_SWDIO_IO1
                                                 *     PIN_TYPE_APS_UART_RXD_IO1
                                                 *     PIN_TYPE_CAM_PCLK_IO1
                                                 *     PIN_TYPE_DBG1_IO1
                                                 *     PIN_TYPE_EXT_2IO_CLK_IO1
                                                 *     PIN_TYPE_GPIO_DISABLE_IO1
                                                 *     PIN_TYPE_GPIO_IN_IO1
                                                 *     PIN_TYPE_GPIO_OUT_HIGH_IO1
                                                 *     PIN_TYPE_GPIO_OUT_LOW_IO1
                                                 *     PIN_TYPE_I2C_SCL_IO_IO1
                                                 *     PIN_TYPE_I2S_SCLK_IO1
                                                 *     PIN_TYPE_MSQ_SWCLK_IO1
                                                 *     PIN_TYPE_MSQ_UART_TXD_IO1
                                                 *     PIN_TYPE_PDM_CLK_IO1
                                                 *     PIN_TYPE_SPI2_CLK_IO1
                                                 *     PIN_TYPE_SPI3_IO3_IO1
                                                 *     PIN_TYPE_SPI3_RXD3_IO1
                                                 *     PIN_TYPE_SPI3_TXD3_IO1
                                                 *     PIN_TYPE_TDI_IO1
                                                 *     PIN_TYPE_UART0_RXD_IO1
                                                 *     PIN_TYPE_UART1_RXD_IO1
                                                 * ***********************************
                                                 *     PIN_DRVCRNT_IO1_2mA
                                                 *     PIN_DRVCRNT_IO1_4mA
                                                 * ***********************************
                                                 *     PIN_INMODE_IO1_FLOATING
                                                 *     PIN_INMODE_IO1_PULL_UP
                                                 * *********************************** */


#define HAL_PIN_TYPE_PATCH_IO_2               (PIN_TYPE_UART1_RXD_IO2        | PIN_DRVCRNT_IO2_2mA   | PIN_INMODE_IO2_PULL_UP)
                                                /* ***********************************
                                                 *     PIN_TYPE_APS_SWCLK_IO2
                                                 *     PIN_TYPE_APS_UART_RXD_IO2
                                                 *     PIN_TYPE_DBG2_IO2
                                                 *     PIN_TYPE_GPIO_DISABLE_IO2
                                                 *     PIN_TYPE_GPIO_IN_IO2
                                                 *     PIN_TYPE_GPIO_OUT_HIGH_IO2
                                                 *     PIN_TYPE_GPIO_OUT_LOW_IO2
                                                 *     PIN_TYPE_I2C_SDA_IO_IO2
                                                 *     PIN_TYPE_I2S_WS_OUT_IO2
                                                 *     PIN_TYPE_MSQ_SWDIO_IO2
                                                 *     PIN_TYPE_MSQ_UART_RXD_IO2
                                                 *     PIN_TYPE_SPI0_CS0_IO2
                                                 *     PIN_TYPE_SPI1_CS0_IO2
                                                 *     PIN_TYPE_SPI3_IO2_IO2
                                                 *     PIN_TYPE_SPI3_RXD2_IO2
                                                 *     PIN_TYPE_SPI3_TXD2_IO2
                                                 *     PIN_TYPE_TCK_IO2
                                                 *     PIN_TYPE_UART0_RXD_IO2
                                                 *     PIN_TYPE_UART1_RXD_IO2
                                                 * ***********************************
                                                 *     PIN_DRVCRNT_IO2_2mA
                                                 *     PIN_DRVCRNT_IO2_4mA
                                                 * ***********************************
                                                 *     PIN_INMODE_IO2_FLOATING
                                                 *     PIN_INMODE_IO2_PULL_UP
                                                 * *********************************** */


#define HAL_PIN_TYPE_PATCH_IO_3               (PIN_TYPE_GPIO_DISABLE_IO3        | PIN_DRVCRNT_IO3_2mA   | PIN_INMODE_IO3_FLOATING)
                                                /* ***********************************
                                                 *     PIN_TYPE_APS_UART_RTS_N_IO3
                                                 *     PIN_TYPE_CAM_DATA0_IO3
                                                 *     PIN_TYPE_DBG3_IO3
                                                 *     PIN_TYPE_EXT_2IO_CLK_IO3
                                                 *     PIN_TYPE_GPIO_DISABLE_IO3
                                                 *     PIN_TYPE_GPIO_IN_IO3
                                                 *     PIN_TYPE_GPIO_OUT_HIGH_IO3
                                                 *     PIN_TYPE_GPIO_OUT_LOW_IO3
                                                 *     PIN_TYPE_I2S_SDO0_IO3
                                                 *     PIN_TYPE_MSQ_UART_RTS_N_IO3
                                                 *     PIN_TYPE_SPI0_CS1_IO3
                                                 *     PIN_TYPE_SPI1_CS1_IO3
                                                 *     PIN_TYPE_SPI3_CS1_IO3
                                                 *     PIN_TYPE_TDO_IO3
                                                 *     PIN_TYPE_UART0_RTS_N_IO3
                                                 *     PIN_TYPE_UART1_RTS_N_IO3
                                                 * ***********************************
                                                 *     PIN_DRVCRNT_IO3_2mA
                                                 *     PIN_DRVCRNT_IO3_4mA
                                                 * ***********************************
                                                 *     PIN_INMODE_IO3_FLOATING
                                                 *     PIN_INMODE_IO3_PULL_DOWN
                                                 * *********************************** */


#define HAL_PIN_TYPE_PATCH_IO_4               (PIN_TYPE_GPIO_DISABLE_IO4        | PIN_DRVCRNT_IO4_2mA   | PIN_INMODE_IO4_FLOATING)
                                                /* ***********************************
                                                 *     PIN_TYPE_APS_SWDIO_IO4
                                                 *     PIN_TYPE_APS_UART_CTS_N_IO4
                                                 *     PIN_TYPE_CAM_DATA1_IO4
                                                 *     PIN_TYPE_CLK_EXT_APS_IO4
                                                 *     PIN_TYPE_CLK_EXT_MSQ_IO4
                                                 *     PIN_TYPE_DBG4_IO4
                                                 *     PIN_TYPE_EXT_2IO_CLK_EN_IO4
                                                 *     PIN_TYPE_GPIO_DISABLE_IO4
                                                 *     PIN_TYPE_GPIO_IN_IO4
                                                 *     PIN_TYPE_GPIO_OUT_HIGH_IO4
                                                 *     PIN_TYPE_GPIO_OUT_LOW_IO4
                                                 *     PIN_TYPE_I2C_SCL_IO_IO4
                                                 *     PIN_TYPE_I2S_SDI0_IO4
                                                 *     PIN_TYPE_MSQ_SWCLK_IO4
                                                 *     PIN_TYPE_MSQ_UART_CTS_N_IO4
                                                 *     PIN_TYPE_SPI0_IO3_IO4
                                                 *     PIN_TYPE_SPI0_RXD3_IO4
                                                 *     PIN_TYPE_SPI0_TXD3_IO4
                                                 *     PIN_TYPE_SPI1_IO3_IO4
                                                 *     PIN_TYPE_SPI1_RXD3_IO4
                                                 *     PIN_TYPE_SPI1_TXD3_IO4
                                                 *     PIN_TYPE_SPI3_IO1_IO4
                                                 *     PIN_TYPE_SPI3_RXD1_IO4
                                                 *     PIN_TYPE_SPI3_TXD1_IO4
                                                 *     PIN_TYPE_TDI_IO4
                                                 *     PIN_TYPE_UART0_CTS_N_IO4
                                                 *     PIN_TYPE_UART1_CTS_N_IO4
                                                 * ***********************************
                                                 *     PIN_DRVCRNT_IO4_2mA
                                                 *     PIN_DRVCRNT_IO4_4mA
                                                 * ***********************************
                                                 *     PIN_INMODE_IO4_FLOATING
                                                 *     PIN_INMODE_IO4_PULL_UP
                                                 * *********************************** */


#define HAL_PIN_TYPE_PATCH_IO_5               (PIN_TYPE_APS_UART_TXD_IO5           | PIN_DRVCRNT_IO5_2mA   | PIN_INMODE_IO5_FLOATING)
                                                /* ***********************************
                                                 *     PIN_TYPE_APS_UART_TXD_IO5
                                                 *     PIN_TYPE_CAM_DATA2_IO5
                                                 *     PIN_TYPE_CLK_EXT_APS_IO5
                                                 *     PIN_TYPE_CLK_EXT_MSQ_IO5
                                                 *     PIN_TYPE_DBG5_IO5
                                                 *     PIN_TYPE_GPIO_DISABLE_IO5
                                                 *     PIN_TYPE_GPIO_IN_IO5
                                                 *     PIN_TYPE_GPIO_OUT_HIGH_IO5
                                                 *     PIN_TYPE_GPIO_OUT_LOW_IO5
                                                 *     PIN_TYPE_I2C_SDA_IO_IO5
                                                 *     PIN_TYPE_I2S_SCLK_IO5
                                                 *     PIN_TYPE_MSQ_UART_TXD_IO5
                                                 *     PIN_TYPE_SPI0_IO2_IO5
                                                 *     PIN_TYPE_SPI0_RXD2_IO5
                                                 *     PIN_TYPE_SPI0_TXD2_IO5
                                                 *     PIN_TYPE_SPI1_IO2_IO5
                                                 *     PIN_TYPE_SPI1_RXD2_IO5
                                                 *     PIN_TYPE_SPI1_TXD2_IO5
                                                 *     PIN_TYPE_SPI3_IO0_IO5
                                                 *     PIN_TYPE_SPI3_RXD0_IO5
                                                 *     PIN_TYPE_SPI3_TXD0_IO5
                                                 *     PIN_TYPE_TCK_IO5
                                                 *     PIN_TYPE_UART0_TXD_IO5
                                                 *     PIN_TYPE_UART1_TXD_IO5
                                                 * ***********************************
                                                 *     PIN_DRVCRNT_IO5_2mA
                                                 *     PIN_DRVCRNT_IO5_4mA
                                                 * ***********************************
                                                 *     PIN_INMODE_IO5_FLOATING
                                                 *     PIN_INMODE_IO5_PULL_UP
                                                 * *********************************** */


#define HAL_PIN_TYPE_PATCH_IO_6               (PIN_TYPE_GPIO_OUT_LOW_IO6        | PIN_DRVCRNT_IO6_2mA   | PIN_INMODE_IO6_FLOATING)
                                                /* ***********************************
                                                 *     PIN_TYPE_APS_SWCLK_IO6
                                                 *     PIN_TYPE_APS_UART_RTS_N_IO6
                                                 *     PIN_TYPE_CAM_DATA3_IO6
                                                 *     PIN_TYPE_CLK_EXT_APS_IO6
                                                 *     PIN_TYPE_CLK_EXT_MSQ_IO6
                                                 *     PIN_TYPE_DBG6_IO6
                                                 *     PIN_TYPE_GPIO_DISABLE_IO6
                                                 *     PIN_TYPE_GPIO_IN_IO6
                                                 *     PIN_TYPE_GPIO_OUT_HIGH_IO6
                                                 *     PIN_TYPE_GPIO_OUT_LOW_IO6
                                                 *     PIN_TYPE_I2S_WS_OUT_IO6
                                                 *     PIN_TYPE_MSQ_SWDIO_IO6
                                                 *     PIN_TYPE_MSQ_UART_RTS_N_IO6
                                                 *     PIN_TYPE_PDM_CLK_IO6
                                                 *     PIN_TYPE_SPI0_CLK_IO6
                                                 *     PIN_TYPE_SPI1_CLK_IO6
                                                 *     PIN_TYPE_TDO_IO6
                                                 *     PIN_TYPE_UART0_RTS_N_IO6
                                                 *     PIN_TYPE_UART1_RTS_N_IO6
                                                 * ***********************************
                                                 *     PIN_DRVCRNT_IO6_2mA
                                                 *     PIN_DRVCRNT_IO6_4mA
                                                 * ***********************************
                                                 *     PIN_INMODE_IO6_FLOATING
                                                 *     PIN_INMODE_IO6_PULL_DOWN
                                                 * *********************************** */


#define HAL_PIN_TYPE_PATCH_IO_7               (PIN_TYPE_GPIO_IN_IO7        | PIN_DRVCRNT_IO7_2mA   | PIN_INMODE_IO7_FLOATING)
                                                /* ***********************************
                                                 *     PIN_TYPE_APS_SWDIO_IO7
                                                 *     PIN_TYPE_APS_UART_TXD_IO7
                                                 *     PIN_TYPE_CAM_DATA4_IO7
                                                 *     PIN_TYPE_CLK_EXT_APS_IO7
                                                 *     PIN_TYPE_CLK_EXT_MSQ_IO7
                                                 *     PIN_TYPE_DBG7_IO7
                                                 *     PIN_TYPE_GPIO_DISABLE_IO7
                                                 *     PIN_TYPE_GPIO_IN_IO7
                                                 *     PIN_TYPE_GPIO_OUT_HIGH_IO7
                                                 *     PIN_TYPE_GPIO_OUT_LOW_IO7
                                                 *     PIN_TYPE_I2S_SDO0_IO7
                                                 *     PIN_TYPE_MSQ_SWCLK_IO7
                                                 *     PIN_TYPE_MSQ_UART_TXD_IO7
                                                 *     PIN_TYPE_PDM_RXD_IO7
                                                 *     PIN_TYPE_SPI0_IO1_IO7
                                                 *     PIN_TYPE_SPI0_RXD1_IO7
                                                 *     PIN_TYPE_SPI0_TXD1_IO7
                                                 *     PIN_TYPE_SPI1_IO1_IO7
                                                 *     PIN_TYPE_SPI1_RXD1_IO7
                                                 *     PIN_TYPE_SPI1_TXD1_IO7
                                                 *     PIN_TYPE_TMS_IO7
                                                 *     PIN_TYPE_UART0_TXD_IO7
                                                 *     PIN_TYPE_UART1_TXD_IO7
                                                 * ***********************************
                                                 *     PIN_DRVCRNT_IO7_2mA
                                                 *     PIN_DRVCRNT_IO7_4mA
                                                 * ***********************************
                                                 *     PIN_INMODE_IO7_FLOATING
                                                 *     PIN_INMODE_IO7_PULL_UP
                                                 * *********************************** */


#define HAL_PIN_TYPE_PATCH_IO_8               (PIN_TYPE_GPIO_IN_IO8        | PIN_DRVCRNT_IO8_2mA   | PIN_INMODE_IO8_PULL_UP)
                                                /* ***********************************
                                                 *     PIN_TYPE_APS_SWCLK_IO8
                                                 *     PIN_TYPE_APS_UART_RXD_IO8
                                                 *     PIN_TYPE_CAM_DATA5_IO8
                                                 *     PIN_TYPE_CLK_EXT_APS_IO8
                                                 *     PIN_TYPE_CLK_EXT_MSQ_IO8
                                                 *     PIN_TYPE_DBG8_IO8
                                                 *     PIN_TYPE_GPIO_DISABLE_IO8
                                                 *     PIN_TYPE_GPIO_IN_IO8
                                                 *     PIN_TYPE_GPIO_OUT_HIGH_IO8
                                                 *     PIN_TYPE_GPIO_OUT_LOW_IO8
                                                 *     PIN_TYPE_I2C_SCL_IO_IO8
                                                 *     PIN_TYPE_I2S_SDI0_IO8
                                                 *     PIN_TYPE_MSQ_SWDIO_IO8
                                                 *     PIN_TYPE_MSQ_UART_RXD_IO8
                                                 *     PIN_TYPE_SPI0_IO0_IO8
                                                 *     PIN_TYPE_SPI0_RXD0_IO8
                                                 *     PIN_TYPE_SPI0_TXD0_IO8
                                                 *     PIN_TYPE_SPI1_IO0_IO8
                                                 *     PIN_TYPE_SPI1_RXD0_IO8
                                                 *     PIN_TYPE_SPI1_TXD0_IO8
                                                 *     PIN_TYPE_SPI3_CS0_IO8
                                                 *     PIN_TYPE_TDI_IO8
                                                 *     PIN_TYPE_UART0_RXD_IO8
                                                 *     PIN_TYPE_UART1_RXD_IO8
                                                 * ***********************************
                                                 *     PIN_DRVCRNT_IO8_2mA
                                                 *     PIN_DRVCRNT_IO8_4mA
                                                 * ***********************************
                                                 *     PIN_INMODE_IO8_FLOATING
                                                 *     PIN_INMODE_IO8_PULL_UP
                                                 * *********************************** */


#define HAL_PIN_TYPE_PATCH_IO_9               (PIN_TYPE_GPIO_DISABLE_IO9        | PIN_DRVCRNT_IO9_2mA   | PIN_INMODE_IO9_FLOATING)
                                                /* ***********************************
                                                 *     PIN_TYPE_APS_UART_CTS_N_IO9
                                                 *     PIN_TYPE_CAM_DATA6_IO9
                                                 *     PIN_TYPE_CLK_EXT_APS_IO9
                                                 *     PIN_TYPE_CLK_EXT_MSQ_IO9
                                                 *     PIN_TYPE_DBG9_IO9
                                                 *     PIN_TYPE_GPIO_DISABLE_IO9
                                                 *     PIN_TYPE_GPIO_IN_IO9
                                                 *     PIN_TYPE_GPIO_OUT_HIGH_IO9
                                                 *     PIN_TYPE_GPIO_OUT_LOW_IO9
                                                 *     PIN_TYPE_I2C_SDA_IO_IO9
                                                 *     PIN_TYPE_I2S_SDO0_IO9
                                                 *     PIN_TYPE_MSQ_UART_CTS_N_IO9
                                                 *     PIN_TYPE_SPI0_CS2_IO9
                                                 *     PIN_TYPE_SPI1_CS2_IO9
                                                 *     PIN_TYPE_SPI3_CLK_IO9
                                                 *     PIN_TYPE_TDO_IO9
                                                 *     PIN_TYPE_UART0_CTS_N_IO9
                                                 *     PIN_TYPE_UART1_CTS_N_IO9
                                                 * ***********************************
                                                 *     PIN_DRVCRNT_IO9_2mA
                                                 *     PIN_DRVCRNT_IO9_4mA
                                                 * ***********************************
                                                 *     PIN_INMODE_IO9_FLOATING
                                                 *     PIN_INMODE_IO9_PULL_UP
                                                 * *********************************** */


#define HAL_PIN_TYPE_PATCH_IO_10              (PIN_TYPE_SPI0_IO2_IO10           | PIN_DRVCRNT_IO10_4mA  | PIN_INMODE_IO10_PULL_UP)
                                                /* ***********************************
                                                 *     PIN_TYPE_APS_UART_CTS_N_IO10
                                                 *     PIN_TYPE_CAM_DATA7_IO10
                                                 *     PIN_TYPE_CLK_EXT_APS_IO10
                                                 *     PIN_TYPE_CLK_EXT_MSQ_IO10
                                                 *     PIN_TYPE_DBG10_IO10
                                                 *     PIN_TYPE_EXT_2IO_CLK_IO10
                                                 *     PIN_TYPE_GPIO_DISABLE_IO10
                                                 *     PIN_TYPE_GPIO_IN_IO10
                                                 *     PIN_TYPE_GPIO_OUT_HIGH_IO10
                                                 *     PIN_TYPE_GPIO_OUT_LOW_IO10
                                                 *     PIN_TYPE_I2C_SCL_IO_IO10
                                                 *     PIN_TYPE_I2S_SCLK_IO10
                                                 *     PIN_TYPE_MSQ_UART_RXD_IO10
                                                 *     PIN_TYPE_OTP_PROGRAM_PERIOD_IO10
                                                 *     PIN_TYPE_PDM_CLK_IO10
                                                 *     PIN_TYPE_SPI0_IO2_IO10
                                                 *     PIN_TYPE_SPI0_RXD2_IO10
                                                 *     PIN_TYPE_SPI0_TXD2_IO10
                                                 *     PIN_TYPE_SPI1_IO2_IO10
                                                 *     PIN_TYPE_SPI1_RXD2_IO10
                                                 *     PIN_TYPE_SPI1_TXD2_IO10
                                                 *     PIN_TYPE_TDI_IO10
                                                 *     PIN_TYPE_UART0_TXD_IO10
                                                 *     PIN_TYPE_UART1_RXD_IO10
                                                 * ***********************************
                                                 *     PIN_DRVCRNT_IO10_4mA
                                                 *     PIN_DRVCRNT_IO10_8mA
                                                 * ***********************************
                                                 *     PIN_INMODE_IO10_FLOATING
                                                 *     PIN_INMODE_IO10_PULL_UP
                                                 * *********************************** */


#define HAL_PIN_TYPE_PATCH_IO_11              (PIN_TYPE_SPI0_IO3_IO11           | PIN_DRVCRNT_IO11_4mA  | PIN_INMODE_IO11_PULL_UP)
                                                /* ***********************************
                                                 *     PIN_TYPE_APS_UART_RXD_IO11
                                                 *     PIN_TYPE_CAM_HREF_IO11
                                                 *     PIN_TYPE_CLK_EXT_APS_IO11
                                                 *     PIN_TYPE_CLK_EXT_MSQ_IO11
                                                 *     PIN_TYPE_DBG11_IO11
                                                 *     PIN_TYPE_GPIO_DISABLE_IO11
                                                 *     PIN_TYPE_GPIO_IN_IO11
                                                 *     PIN_TYPE_GPIO_OUT_HIGH_IO11
                                                 *     PIN_TYPE_GPIO_OUT_LOW_IO11
                                                 *     PIN_TYPE_I2C_SDA_IO_IO11
                                                 *     PIN_TYPE_I2S_WS_OUT_IO11
                                                 *     PIN_TYPE_MSQ_UART_RTS_N_IO11
                                                 *     PIN_TYPE_OTP_OPM_MODE_IO11
                                                 *     PIN_TYPE_PDM_RXD_IO11
                                                 *     PIN_TYPE_SPI0_IO3_IO11
                                                 *     PIN_TYPE_SPI0_RXD3_IO11
                                                 *     PIN_TYPE_SPI0_TXD3_IO11
                                                 *     PIN_TYPE_SPI1_IO3_IO11
                                                 *     PIN_TYPE_SPI1_RXD3_IO11
                                                 *     PIN_TYPE_SPI1_TXD3_IO11
                                                 *     PIN_TYPE_TDO_IO11
                                                 *     PIN_TYPE_UART0_RXD_IO11
                                                 *     PIN_TYPE_UART1_TXD_IO11
                                                 * ***********************************
                                                 *     PIN_DRVCRNT_IO11_4mA
                                                 *     PIN_DRVCRNT_IO11_8mA
                                                 * ***********************************
                                                 *     PIN_INMODE_IO11_FLOATING
                                                 *     PIN_INMODE_IO11_PULL_UP
                                                 * *********************************** */


#define HAL_PIN_TYPE_PATCH_IO_12              (PIN_TYPE_SPI0_CS0_IO12           | PIN_DRVCRNT_IO12_4mA  | PIN_INMODE_IO12_PULL_UP)
                                                /* ***********************************
                                                 *     PIN_TYPE_APS_UART_TXD_IO12
                                                 *     PIN_TYPE_CAM_VSYNC_IO12
                                                 *     PIN_TYPE_DBG12_IO12
                                                 *     PIN_TYPE_GPIO_DISABLE_IO12
                                                 *     PIN_TYPE_GPIO_IN_IO12
                                                 *     PIN_TYPE_GPIO_OUT_HIGH_IO12
                                                 *     PIN_TYPE_GPIO_OUT_LOW_IO12
                                                 *     PIN_TYPE_I2S_SDI0_IO12
                                                 *     PIN_TYPE_MSQ_UART_TXD_IO12
                                                 *     PIN_TYPE_PDM_CLK_IO12
                                                 *     PIN_TYPE_PU_EPA_OUT0_IO12
                                                 *     PIN_TYPE_SPI0_CS0_IO12
                                                 *     PIN_TYPE_SPI1_CS0_IO12
                                                 *     PIN_TYPE_TMS_IO12
                                                 *     PIN_TYPE_UART0_CTS_N_IO12
                                                 *     PIN_TYPE_UART1_CTS_N_IO12
                                                 * ***********************************
                                                 *     PIN_DRVCRNT_IO12_4mA
                                                 *     PIN_DRVCRNT_IO12_8mA
                                                 * ***********************************
                                                 *     PIN_INMODE_IO12_FLOATING
                                                 *     PIN_INMODE_IO12_PULL_UP
                                                 * *********************************** */


#define HAL_PIN_TYPE_PATCH_IO_13              (PIN_TYPE_SPI0_CLK_IO13           | PIN_DRVCRNT_IO13_4mA  | PIN_INMODE_IO13_FLOATING)
                                                /* ***********************************
                                                 *     PIN_TYPE_APS_UART_RTS_N_IO13
                                                 *     PIN_TYPE_CAM_XVCLK_IO13
                                                 *     PIN_TYPE_DBG13_IO13
                                                 *     PIN_TYPE_GPIO_DISABLE_IO13
                                                 *     PIN_TYPE_GPIO_IN_IO13
                                                 *     PIN_TYPE_GPIO_OUT_HIGH_IO13
                                                 *     PIN_TYPE_GPIO_OUT_LOW_IO13
                                                 *     PIN_TYPE_I2S_SDO0_IO13
                                                 *     PIN_TYPE_MSQ_UART_TXD_IO13
                                                 *     PIN_TYPE_PDM_RXD_IO13
                                                 *     PIN_TYPE_PU_EPA_OUT1_IO13
                                                 *     PIN_TYPE_SPI0_CLK_IO13
                                                 *     PIN_TYPE_SPI1_CLK_IO13
                                                 *     PIN_TYPE_TCK_IO13
                                                 *     PIN_TYPE_UART0_RTS_N_IO13
                                                 *     PIN_TYPE_UART1_RTS_N_IO13
                                                 * ***********************************
                                                 *     PIN_DRVCRNT_IO13_4mA
                                                 *     PIN_DRVCRNT_IO13_8mA
                                                 * ***********************************
                                                 *     PIN_INMODE_IO13_FLOATING
                                                 *     PIN_INMODE_IO13_PULL_DOWN
                                                 * *********************************** */


#define HAL_PIN_TYPE_PATCH_IO_14              (PIN_TYPE_SPI0_IO0_IO14           | PIN_DRVCRNT_IO14_4mA  | PIN_INMODE_IO14_FLOATING)
                                                /* ***********************************
                                                 *     PIN_TYPE_APS_SWCLK_IO14
                                                 *     PIN_TYPE_APS_UART_TXD_IO14
                                                 *     PIN_TYPE_CAM_SHUTTER_IO14
                                                 *     PIN_TYPE_DBG14_IO14
                                                 *     PIN_TYPE_GPIO_DISABLE_IO14
                                                 *     PIN_TYPE_GPIO_IN_IO14
                                                 *     PIN_TYPE_GPIO_OUT_HIGH_IO14
                                                 *     PIN_TYPE_GPIO_OUT_LOW_IO14
                                                 *     PIN_TYPE_I2C_SCL_IO_IO14
                                                 *     PIN_TYPE_I2S_SCLK_IO14
                                                 *     PIN_TYPE_MSQ_SWDIO_IO14
                                                 *     PIN_TYPE_MSQ_UART_CTS_N_IO14
                                                 *     PIN_TYPE_PU_EPA_OUT2_IO14
                                                 *     PIN_TYPE_PWM5_IO14
                                                 *     PIN_TYPE_SPI0_IO0_IO14
                                                 *     PIN_TYPE_SPI0_RXD0_IO14
                                                 *     PIN_TYPE_SPI0_TXD0_IO14
                                                 *     PIN_TYPE_SPI1_IO0_IO14
                                                 *     PIN_TYPE_SPI1_RXD0_IO14
                                                 *     PIN_TYPE_SPI1_TXD0_IO14
                                                 *     PIN_TYPE_TDI_IO14
                                                 *     PIN_TYPE_UART0_RTS_N_IO14
                                                 *     PIN_TYPE_UART1_CTS_N_IO14
                                                 * ***********************************
                                                 *     PIN_DRVCRNT_IO14_4mA
                                                 *     PIN_DRVCRNT_IO14_8mA
                                                 * ***********************************
                                                 *     PIN_INMODE_IO14_FLOATING
                                                 *     PIN_INMODE_IO14_PULL_UP
                                                 * *********************************** */


#define HAL_PIN_TYPE_PATCH_IO_15              (PIN_TYPE_SPI0_IO1_IO15           | PIN_DRVCRNT_IO15_4mA  | PIN_INMODE_IO15_FLOATING)
                                                /* ***********************************
                                                 *     PIN_TYPE_APS_SWDIO_IO15
                                                 *     PIN_TYPE_APS_UART_RXD_IO15
                                                 *     PIN_TYPE_CAM_HSYNC_IO15
                                                 *     PIN_TYPE_CAM_PCLK_IO15
                                                 *     PIN_TYPE_DBG15_IO15
                                                 *     PIN_TYPE_GPIO_DISABLE_IO15
                                                 *     PIN_TYPE_GPIO_IN_IO15
                                                 *     PIN_TYPE_GPIO_OUT_HIGH_IO15
                                                 *     PIN_TYPE_GPIO_OUT_LOW_IO15
                                                 *     PIN_TYPE_I2C_SDA_IO_IO15
                                                 *     PIN_TYPE_MSQ_SWCLK_IO15
                                                 *     PIN_TYPE_MSQ_UART_TXD_IO15
                                                 *     PIN_TYPE_PU_EPA_OUT3_IO15
                                                 *     PIN_TYPE_PWM4_IO15
                                                 *     PIN_TYPE_SPI0_IO1_IO15
                                                 *     PIN_TYPE_SPI0_RXD1_IO15
                                                 *     PIN_TYPE_SPI0_TXD1_IO15
                                                 *     PIN_TYPE_SPI1_IO1_IO15
                                                 *     PIN_TYPE_SPI1_RXD1_IO15
                                                 *     PIN_TYPE_SPI1_TXD1_IO15
                                                 *     PIN_TYPE_SPI3_CS3_IO15
                                                 *     PIN_TYPE_TCK_IO15
                                                 *     PIN_TYPE_UART0_CTS_N_IO15
                                                 *     PIN_TYPE_UART1_RTS_N_IO15
                                                 * ***********************************
                                                 *     PIN_DRVCRNT_IO15_4mA
                                                 *     PIN_DRVCRNT_IO15_8mA
                                                 * ***********************************
                                                 *     PIN_INMODE_IO15_FLOATING
                                                 *     PIN_INMODE_IO15_PULL_UP
                                                 * *********************************** */


#define HAL_PIN_TYPE_PATCH_IO_16              (PIN_TYPE_GPIO_DISABLE_IO16          | PIN_DRVCRNT_IO16_12mA | PIN_INMODE_IO16_FLOATING)
                                                /* ***********************************
                                                 *     PIN_TYPE_APS_SWDIO_IO16
                                                 *     PIN_TYPE_APS_UART_RTS_N_IO16
                                                 *     PIN_TYPE_DBG16_IO16
                                                 *     PIN_TYPE_GPIO_DISABLE_IO16
                                                 *     PIN_TYPE_GPIO_IN_IO16
                                                 *     PIN_TYPE_GPIO_OUT_HIGH_IO16
                                                 *     PIN_TYPE_GPIO_OUT_LOW_IO16
                                                 *     PIN_TYPE_I2C_SCL_IO_IO16
                                                 *     PIN_TYPE_I2S_SDO0_IO16
                                                 *     PIN_TYPE_MSQ_SWCLK_IO16
                                                 *     PIN_TYPE_MSQ_UART_RXD_IO16
                                                 *     PIN_TYPE_PU_EPA_OUT0_IO16
                                                 *     PIN_TYPE_PWM3_IO16
                                                 *     PIN_TYPE_SPI3_IO1_IO16
                                                 *     PIN_TYPE_SPI3_RXD1_IO16
                                                 *     PIN_TYPE_SPI3_TXD1_IO16
                                                 *     PIN_TYPE_TMS_IO16
                                                 *     PIN_TYPE_UART0_TXD_IO16
                                                 *     PIN_TYPE_UART1_RXD_IO16
                                                 * ***********************************
                                                 *     PIN_DRVCRNT_IO16_12mA
                                                 *     PIN_DRVCRNT_IO16_16mA
                                                 * ***********************************
                                                 *     PIN_INMODE_IO16_FLOATING
                                                 *     PIN_INMODE_IO16_PULL_DOWN
                                                 * *********************************** */


#define HAL_PIN_TYPE_PATCH_IO_17              (PIN_TYPE_GPIO_DISABLE_IO17          | PIN_DRVCRNT_IO17_12mA | PIN_INMODE_IO17_FLOATING)
                                                /* ***********************************
                                                 *     PIN_TYPE_APS_SWCLK_IO17
                                                 *     PIN_TYPE_APS_UART_CTS_N_IO17
                                                 *     PIN_TYPE_DBG17_IO17
                                                 *     PIN_TYPE_GPIO_DISABLE_IO17
                                                 *     PIN_TYPE_GPIO_IN_IO17
                                                 *     PIN_TYPE_GPIO_OUT_HIGH_IO17
                                                 *     PIN_TYPE_GPIO_OUT_LOW_IO17
                                                 *     PIN_TYPE_I2C_SDA_IO_IO17
                                                 *     PIN_TYPE_I2S_SDI0_IO17
                                                 *     PIN_TYPE_MSQ_SWDIO_IO17
                                                 *     PIN_TYPE_MSQ_UART_RTS_N_IO17
                                                 *     PIN_TYPE_PU_EPA_OUT1_IO17
                                                 *     PIN_TYPE_PWM2_IO17
                                                 *     PIN_TYPE_SPI1_CS3_IO17
                                                 *     PIN_TYPE_SPI3_IO0_IO17
                                                 *     PIN_TYPE_SPI3_RXD0_IO17
                                                 *     PIN_TYPE_SPI3_TXD0_IO17
                                                 *     PIN_TYPE_TDO_IO17
                                                 *     PIN_TYPE_UART0_RXD_IO17
                                                 *     PIN_TYPE_UART1_TXD_IO17
                                                 * ***********************************
                                                 *     PIN_DRVCRNT_IO17_12mA
                                                 *     PIN_DRVCRNT_IO17_16mA
                                                 * ***********************************
                                                 *     PIN_INMODE_IO17_FLOATING
                                                 *     PIN_INMODE_IO17_PULL_DOWN
                                                 * *********************************** */


#define HAL_PIN_TYPE_PATCH_IO_18              (PIN_TYPE_GPIO_DISABLE_IO18          | PIN_DRVCRNT_IO18_12mA | PIN_INMODE_IO18_FLOATING)
                                                /* ***********************************
                                                 *     PIN_TYPE_APS_SWDIO_IO18
                                                 *     PIN_TYPE_APS_UART_RXD_IO18
                                                 *     PIN_TYPE_CAM_FREX_IO18
                                                 *     PIN_TYPE_CLK_EXT_APS_IO18
                                                 *     PIN_TYPE_CLK_EXT_MSQ_IO18
                                                 *     PIN_TYPE_DBG18_IO18
                                                 *     PIN_TYPE_GPIO_DISABLE_IO18
                                                 *     PIN_TYPE_GPIO_IN_IO18
                                                 *     PIN_TYPE_GPIO_OUT_HIGH_IO18
                                                 *     PIN_TYPE_GPIO_OUT_LOW_IO18
                                                 *     PIN_TYPE_I2C_SCL_IO_IO18
                                                 *     PIN_TYPE_I2S_SCLK_IO18
                                                 *     PIN_TYPE_MSQ_SWCLK_IO18
                                                 *     PIN_TYPE_MSQ_UART_CTS_N_IO18
                                                 *     PIN_TYPE_PU_EPA_OUT0_IO18
                                                 *     PIN_TYPE_PWM1_IO18
                                                 *     PIN_TYPE_SPI1_CS1_IO18
                                                 *     PIN_TYPE_SPI3_CLK_IO18
                                                 *     PIN_TYPE_TDI_IO18
                                                 *     PIN_TYPE_UART0_RXD_IO18
                                                 *     PIN_TYPE_UART1_TXD_IO18
                                                 * ***********************************
                                                 *     PIN_DRVCRNT_IO18_12mA
                                                 *     PIN_DRVCRNT_IO18_16mA
                                                 * ***********************************
                                                 *     PIN_INMODE_IO18_FLOATING
                                                 *     PIN_INMODE_IO18_PULL_UP
                                                 * *********************************** */


#define HAL_PIN_TYPE_PATCH_IO_19              (PIN_TYPE_GPIO_DISABLE_IO19          | PIN_DRVCRNT_IO19_12mA | PIN_INMODE_IO19_FLOATING)
                                                /* ***********************************
                                                 *     PIN_TYPE_APS_SWCLK_IO19
                                                 *     PIN_TYPE_APS_UART_TXD_IO19
                                                 *     PIN_TYPE_CAM_EXPST_IO19
                                                 *     PIN_TYPE_CLK_EXT_APS_IO19
                                                 *     PIN_TYPE_CLK_EXT_MSQ_IO19
                                                 *     PIN_TYPE_DBG19_IO19
                                                 *     PIN_TYPE_GPIO_DISABLE_IO19
                                                 *     PIN_TYPE_GPIO_IN_IO19
                                                 *     PIN_TYPE_GPIO_OUT_HIGH_IO19
                                                 *     PIN_TYPE_GPIO_OUT_LOW_IO19
                                                 *     PIN_TYPE_I2C_SDA_IO_IO19
                                                 *     PIN_TYPE_I2S_WS_OUT_IO19
                                                 *     PIN_TYPE_MSQ_SWDIO_IO19
                                                 *     PIN_TYPE_MSQ_UART_RXD_IO19
                                                 *     PIN_TYPE_PU_EPA_OUT1_IO19
                                                 *     PIN_TYPE_PWM0_IO19
                                                 *     PIN_TYPE_SPI0_CS1_IO19
                                                 *     PIN_TYPE_SPI1_CS2_IO19
                                                 *     PIN_TYPE_SPI3_CS0_IO19
                                                 *     PIN_TYPE_TCK_IO19
                                                 *     PIN_TYPE_UART0_TXD_IO19
                                                 *     PIN_TYPE_UART1_RXD_IO19
                                                 * ***********************************
                                                 *     PIN_DRVCRNT_IO19_12mA
                                                 *     PIN_DRVCRNT_IO19_16mA
                                                 * ***********************************
                                                 *     PIN_INMODE_IO19_FLOATING
                                                 *     PIN_INMODE_IO19_PULL_UP
                                                 * *********************************** */


#define HAL_PIN_TYPE_PATCH_IO_20              (PIN_TYPE_GPIO_DISABLE_IO20       | PIN_DRVCRNT_IO20_12mA | PIN_INMODE_IO20_FLOATING)
                                                /* ***********************************
                                                 *     PIN_TYPE_APS_SWCLK_IO20
                                                 *     PIN_TYPE_APS_UART_RXD_IO20
                                                 *     PIN_TYPE_CAM_DATA7_IO20
                                                 *     PIN_TYPE_DBG20_IO20
                                                 *     PIN_TYPE_GPIO_DISABLE_IO20
                                                 *     PIN_TYPE_GPIO_IN_IO20
                                                 *     PIN_TYPE_GPIO_OUT_HIGH_IO20
                                                 *     PIN_TYPE_GPIO_OUT_LOW_IO20
                                                 *     PIN_TYPE_I2C_SCL_IO_IO20
                                                 *     PIN_TYPE_I2S_SDI0_IO20
                                                 *     PIN_TYPE_MSQ_SWDIO_IO20
                                                 *     PIN_TYPE_MSQ_UART_RXD_IO20
                                                 *     PIN_TYPE_PU_EPA_OUT2_IO20
                                                 *     PIN_TYPE_PWM0_IO20
                                                 *     PIN_TYPE_SPI0_CLK_IO20
                                                 *     PIN_TYPE_SPI0_CS2_IO20
                                                 *     PIN_TYPE_SPI3_CLK_IO20
                                                 *     PIN_TYPE_TDO_IO20
                                                 *     PIN_TYPE_UART0_RXD_IO20
                                                 *     PIN_TYPE_UART1_TXD_IO20
                                                 * ***********************************
                                                 *     PIN_DRVCRNT_IO20_12mA
                                                 *     PIN_DRVCRNT_IO20_16mA
                                                 * ***********************************
                                                 *     PIN_INMODE_IO20_FLOATING
                                                 *     PIN_INMODE_IO20_PULL_DOWN
                                                 * *********************************** */


#define HAL_PIN_TYPE_PATCH_IO_21              (PIN_TYPE_GPIO_DISABLE_IO21       | PIN_DRVCRNT_IO21_12mA | PIN_INMODE_IO21_FLOATING)
                                                /* ***********************************
                                                 *     PIN_TYPE_APS_UART_RTS_N_IO21
                                                 *     PIN_TYPE_CAM_HREF_IO21
                                                 *     PIN_TYPE_DBG21_IO21
                                                 *     PIN_TYPE_GPIO_DISABLE_IO21
                                                 *     PIN_TYPE_GPIO_IN_IO21
                                                 *     PIN_TYPE_GPIO_OUT_HIGH_IO21
                                                 *     PIN_TYPE_GPIO_OUT_LOW_IO21
                                                 *     PIN_TYPE_I2C_SDA_IO_IO21
                                                 *     PIN_TYPE_I2S_SDO0_IO21
                                                 *     PIN_TYPE_MSQ_UART_TXD_IO21
                                                 *     PIN_TYPE_PU_EPA_OUT3_IO21
                                                 *     PIN_TYPE_PWM1_IO21
                                                 *     PIN_TYPE_SPI0_CS0_IO21
                                                 *     PIN_TYPE_SPI0_CS3_IO21
                                                 *     PIN_TYPE_SPI3_CS0_IO21
                                                 *     PIN_TYPE_TCK_IO21
                                                 *     PIN_TYPE_UART0_TXD_IO21
                                                 *     PIN_TYPE_UART1_RXD_IO21
                                                 * ***********************************
                                                 *     PIN_DRVCRNT_IO21_12mA
                                                 *     PIN_DRVCRNT_IO21_16mA
                                                 * ***********************************
                                                 *     PIN_INMODE_IO21_FLOATING
                                                 *     PIN_INMODE_IO21_PULL_DOWN
                                                 * *********************************** */


#define HAL_PIN_TYPE_PATCH_IO_22              (PIN_TYPE_GPIO_DISABLE_IO22       | PIN_DRVCRNT_IO22_12mA | PIN_INMODE_IO22_FLOATING)
                                                /* ***********************************
                                                 *     PIN_TYPE_APS_UART_TXD_IO22
                                                 *     PIN_TYPE_CAM_XVCLK_IO22
                                                 *     PIN_TYPE_DBG22_IO22
                                                 *     PIN_TYPE_EXT_2IO_CLK_EN_IO22
                                                 *     PIN_TYPE_GPIO_DISABLE_IO22
                                                 *     PIN_TYPE_GPIO_IN_IO22
                                                 *     PIN_TYPE_GPIO_OUT_HIGH_IO22
                                                 *     PIN_TYPE_GPIO_OUT_LOW_IO22
                                                 *     PIN_TYPE_I2S_SDI0_IO22
                                                 *     PIN_TYPE_MSQ_SWDIO_IO22
                                                 *     PIN_TYPE_MSQ_UART_CTS_N_IO22
                                                 *     PIN_TYPE_PDM_CLK_IO22
                                                 *     PIN_TYPE_PWM2_IO22
                                                 *     PIN_TYPE_SPI2_CS_IN_IO22
                                                 *     PIN_TYPE_SPI3_CLK_IO22
                                                 *     PIN_TYPE_TCK_IO22
                                                 *     PIN_TYPE_UART0_TXD_IO22
                                                 *     PIN_TYPE_UART1_TXD_IO22
                                                 * ***********************************
                                                 *     PIN_DRVCRNT_IO22_12mA
                                                 *     PIN_DRVCRNT_IO22_16mA
                                                 * ***********************************
                                                 *     PIN_INMODE_IO22_FLOATING
                                                 *     PIN_INMODE_IO22_PULL_UP
                                                 * *********************************** */


#define HAL_PIN_TYPE_PATCH_IO_23              (PIN_TYPE_GPIO_DISABLE_IO23       | PIN_DRVCRNT_IO23_12mA | PIN_INMODE_IO23_FLOATING)
                                                /* ***********************************
                                                 *     PIN_TYPE_APS_UART_TXD_IO23
                                                 *     PIN_TYPE_CAM_SHUTTER_IO23
                                                 *     PIN_TYPE_DBG23_IO23
                                                 *     PIN_TYPE_EXT_2IO_CLK_EN_IO23
                                                 *     PIN_TYPE_GPIO_DISABLE_IO23
                                                 *     PIN_TYPE_GPIO_IN_IO23
                                                 *     PIN_TYPE_GPIO_OUT_HIGH_IO23
                                                 *     PIN_TYPE_GPIO_OUT_LOW_IO23
                                                 *     PIN_TYPE_I2C_SCL_IO_IO23
                                                 *     PIN_TYPE_I2S_WS_OUT_IO23
                                                 *     PIN_TYPE_MSQ_SWCLK_IO23
                                                 *     PIN_TYPE_MSQ_UART_RTS_N_IO23
                                                 *     PIN_TYPE_PWM3_IO23
                                                 *     PIN_TYPE_SPI0_CS2_IO23
                                                 *     PIN_TYPE_SPI1_CS3_IO23
                                                 *     PIN_TYPE_SPI3_IO2_IO23
                                                 *     PIN_TYPE_SPI3_RXD2_IO23
                                                 *     PIN_TYPE_SPI3_TXD2_IO23
                                                 *     PIN_TYPE_TDI_IO23
                                                 *     PIN_TYPE_UART0_RTS_N_IO23
                                                 *     PIN_TYPE_UART1_TXD_IO23
                                                 * ***********************************
                                                 *     PIN_DRVCRNT_IO23_12mA
                                                 *     PIN_DRVCRNT_IO23_16mA
                                                 * ***********************************
                                                 *     PIN_INMODE_IO23_FLOATING
                                                 *     PIN_INMODE_IO23_PULL_UP
                                                 * *********************************** */


#define HAL_PIN_TYPE_PATCH_IO_24              (PIN_TYPE_GPIO_DISABLE_IO24       | PIN_DRVCRNT_IO24_12mA | PIN_INMODE_IO24_FLOATING)
                                                /* ***********************************
                                                 *     PIN_TYPE_APS_UART_TXD_IO24
                                                 *     PIN_TYPE_CAM_HSYNC_IO24
                                                 *     PIN_TYPE_CLK_DS_AUX_78K125_IO24
                                                 *     PIN_TYPE_DBG24_IO24
                                                 *     PIN_TYPE_GPIO_DISABLE_IO24
                                                 *     PIN_TYPE_GPIO_IN_IO24
                                                 *     PIN_TYPE_GPIO_OUT_HIGH_IO24
                                                 *     PIN_TYPE_GPIO_OUT_LOW_IO24
                                                 *     PIN_TYPE_I2C_SDA_IO_IO24
                                                 *     PIN_TYPE_I2S_SCLK_IO24
                                                 *     PIN_TYPE_MSQ_SWDIO_IO24
                                                 *     PIN_TYPE_MSQ_UART_TXD_IO24
                                                 *     PIN_TYPE_PDM_CLK_IO24
                                                 *     PIN_TYPE_PWM4_IO24
                                                 *     PIN_TYPE_SPI2_CLK_IO24
                                                 *     PIN_TYPE_SPI3_CLK_IO24
                                                 *     PIN_TYPE_TDO_IO24
                                                 *     PIN_TYPE_UART1_RTS_N_IO24
                                                 * ***********************************
                                                 *     PIN_DRVCRNT_IO24_12mA
                                                 *     PIN_DRVCRNT_IO24_16mA
                                                 * ***********************************
                                                 *     PIN_INMODE_IO24_FLOATING
                                                 *     PIN_INMODE_IO24_PULL_UP
                                                 * *********************************** */


#define HAL_PIN_TYPE_PATCH_IO_25              (PIN_TYPE_GPIO_DISABLE_IO25       | PIN_DRVCRNT_IO25_12mA | PIN_INMODE_IO25_FLOATING)
                                                /* ***********************************
                                                 *     PIN_TYPE_APS_SWDIO_IO25
                                                 *     PIN_TYPE_APS_UART_RXD_IO25
                                                 *     PIN_TYPE_DBG25_IO25
                                                 *     PIN_TYPE_GPIO_DISABLE_IO25
                                                 *     PIN_TYPE_GPIO_IN_IO25
                                                 *     PIN_TYPE_GPIO_OUT_HIGH_IO25
                                                 *     PIN_TYPE_GPIO_OUT_LOW_IO25
                                                 *     PIN_TYPE_I2C_SCL_IO_IO25
                                                 *     PIN_TYPE_I2S_WS_OUT_IO25
                                                 *     PIN_TYPE_MSQ_SWCLK_IO25
                                                 *     PIN_TYPE_MSQ_UART_RXD_IO25
                                                 *     PIN_TYPE_PDM_RXD_IO25
                                                 *     PIN_TYPE_PU_EPA_OUT0_IO25
                                                 *     PIN_TYPE_PWM5_IO25
                                                 *     PIN_TYPE_SPI2_CS_IN_IO25
                                                 *     PIN_TYPE_SPI3_CS0_IO25
                                                 *     PIN_TYPE_TDI_IO25
                                                 *     PIN_TYPE_UART0_CTS_N_IO25
                                                 *     PIN_TYPE_UART1_CTS_N_IO25
                                                 * ***********************************
                                                 *     PIN_DRVCRNT_IO25_12mA
                                                 *     PIN_DRVCRNT_IO25_16mA
                                                 * ***********************************
                                                 *     PIN_INMODE_IO25_FLOATING
                                                 *     PIN_INMODE_IO25_PULL_UP
                                                 * *********************************** */


#define HAL_PIN_TYPE_PATCH_IO_26              (PIN_TYPE_GPIO_DISABLE_IO26       | PIN_DRVCRNT_IO26_2mA  | PIN_INMODE_IO26_FLOATING)
                                                /* ***********************************
                                                 *     PIN_TYPE_APS_SWCLK_IO26
                                                 *     PIN_TYPE_APS_UART_RTS_N_IO26
                                                 *     PIN_TYPE_CAM_VSYNC_IO26
                                                 *     PIN_TYPE_DBG26_IO26
                                                 *     PIN_TYPE_GPIO_DISABLE_IO26
                                                 *     PIN_TYPE_GPIO_IN_IO26
                                                 *     PIN_TYPE_GPIO_OUT_HIGH_IO26
                                                 *     PIN_TYPE_GPIO_OUT_LOW_IO26
                                                 *     PIN_TYPE_I2C_SDA_IO_IO26
                                                 *     PIN_TYPE_I2S_SDI0_IO26
                                                 *     PIN_TYPE_MSQ_SWDIO_IO26
                                                 *     PIN_TYPE_MSQ_UART_TXD_IO26
                                                 *     PIN_TYPE_PDM_RXD_IO26
                                                 *     PIN_TYPE_PU_EPA_OUT0_IO26
                                                 *     PIN_TYPE_PWM0_IO26
                                                 *     PIN_TYPE_SPI2_IO_IO26
                                                 *     PIN_TYPE_SPI2_RXD_IO26
                                                 *     PIN_TYPE_SPI2_TXD_IO26
                                                 *     PIN_TYPE_SPI3_CS0_IO26
                                                 *     PIN_TYPE_TMS_IO26
                                                 *     PIN_TYPE_UART0_RTS_N_IO26
                                                 *     PIN_TYPE_UART1_RTS_N_IO26
                                                 * ***********************************
                                                 *     PIN_DRVCRNT_IO26_2mA
                                                 *     PIN_DRVCRNT_IO26_4mA
                                                 * ***********************************
                                                 *     PIN_INMODE_IO26_FLOATING
                                                 *     PIN_INMODE_IO26_PULL_UP
                                                 * *********************************** */


#define HAL_PIN_TYPE_PATCH_IO_27              (PIN_TYPE_GPIO_DISABLE_IO27       | PIN_DRVCRNT_IO27_2mA  | PIN_INMODE_IO27_FLOATING)
                                                /* ***********************************
                                                 *     PIN_TYPE_APS_SWDIO_IO27
                                                 *     PIN_TYPE_APS_UART_CTS_N_IO27
                                                 *     PIN_TYPE_CAM_DATA8_IO27
                                                 *     PIN_TYPE_DBG27_IO27
                                                 *     PIN_TYPE_GPIO_DISABLE_IO27
                                                 *     PIN_TYPE_GPIO_IN_IO27
                                                 *     PIN_TYPE_GPIO_OUT_HIGH_IO27
                                                 *     PIN_TYPE_GPIO_OUT_LOW_IO27
                                                 *     PIN_TYPE_I2C_SCL_IO_IO27
                                                 *     PIN_TYPE_I2S_SCLK_IO27
                                                 *     PIN_TYPE_MSQ_SWCLK_IO27
                                                 *     PIN_TYPE_MSQ_UART_CTS_N_IO27
                                                 *     PIN_TYPE_PU_EPA_OUT1_IO27
                                                 *     PIN_TYPE_PWM1_IO27
                                                 *     PIN_TYPE_SPI0_CS2_IO27
                                                 *     PIN_TYPE_SPI3_CS2_IO27
                                                 *     PIN_TYPE_SPI3_IO0_IO27
                                                 *     PIN_TYPE_SPI3_RXD0_IO27
                                                 *     PIN_TYPE_SPI3_TXD0_IO27
                                                 *     PIN_TYPE_TDI_IO27
                                                 *     PIN_TYPE_UART0_CTS_N_IO27
                                                 *     PIN_TYPE_UART1_CTS_N_IO27
                                                 * ***********************************
                                                 *     PIN_DRVCRNT_IO27_2mA
                                                 *     PIN_DRVCRNT_IO27_4mA
                                                 * ***********************************
                                                 *     PIN_INMODE_IO27_FLOATING
                                                 *     PIN_INMODE_IO27_PULL_UP
                                                 * *********************************** */


#define HAL_PIN_TYPE_PATCH_IO_28              (PIN_TYPE_GPIO_DISABLE_IO28       | PIN_DRVCRNT_IO28_2mA  | PIN_INMODE_IO28_FLOATING)
                                                /* ***********************************
                                                 *     PIN_TYPE_APS_SWCLK_IO28
                                                 *     PIN_TYPE_APS_UART_CTS_N_IO28
                                                 *     PIN_TYPE_CAM_DATA9_IO28
                                                 *     PIN_TYPE_DBG28_IO28
                                                 *     PIN_TYPE_DT_DS_AUX_IO28
                                                 *     PIN_TYPE_EXT_2IO_CLK_IO28
                                                 *     PIN_TYPE_GPIO_DISABLE_IO28
                                                 *     PIN_TYPE_GPIO_IN_IO28
                                                 *     PIN_TYPE_GPIO_OUT_HIGH_IO28
                                                 *     PIN_TYPE_GPIO_OUT_LOW_IO28
                                                 *     PIN_TYPE_I2S_SDO0_IO28
                                                 *     PIN_TYPE_MSQ_SWDIO_IO28
                                                 *     PIN_TYPE_MSQ_UART_RXD_IO28
                                                 *     PIN_TYPE_PWM2_IO28
                                                 *     PIN_TYPE_SPI2_IO_IO28
                                                 *     PIN_TYPE_SPI2_RXD_IO28
                                                 *     PIN_TYPE_SPI2_TXD_IO28
                                                 *     PIN_TYPE_SPI3_CLK_IO28
                                                 *     PIN_TYPE_SPI3_IO2_IO28
                                                 *     PIN_TYPE_SPI3_RXD2_IO28
                                                 *     PIN_TYPE_SPI3_TXD2_IO28
                                                 *     PIN_TYPE_TCK_IO28
                                                 *     PIN_TYPE_UART0_TXD_IO28
                                                 *     PIN_TYPE_UART1_RXD_IO28
                                                 * ***********************************
                                                 *     PIN_DRVCRNT_IO28_2mA
                                                 *     PIN_DRVCRNT_IO28_4mA
                                                 * ***********************************
                                                 *     PIN_INMODE_IO28_FLOATING
                                                 *     PIN_INMODE_IO28_PULL_UP
                                                 * *********************************** */


#define HAL_PIN_TYPE_PATCH_IO_29              (PIN_TYPE_GPIO_DISABLE_IO29       | PIN_DRVCRNT_IO29_2mA  | PIN_INMODE_IO29_FLOATING)
                                                /* ***********************************
                                                 *     PIN_TYPE_APS_SWDIO_IO29
                                                 *     PIN_TYPE_APS_UART_RTS_N_IO29
                                                 *     PIN_TYPE_CAM_DATA10_IO29
                                                 *     PIN_TYPE_DBG29_IO29
                                                 *     PIN_TYPE_GPIO_DISABLE_IO29
                                                 *     PIN_TYPE_GPIO_IN_IO29
                                                 *     PIN_TYPE_GPIO_OUT_HIGH_IO29
                                                 *     PIN_TYPE_GPIO_OUT_LOW_IO29
                                                 *     PIN_TYPE_I2C_SDA_IO_IO29
                                                 *     PIN_TYPE_I2S_SDI0_IO29
                                                 *     PIN_TYPE_MSQ_SWCLK_IO29
                                                 *     PIN_TYPE_MSQ_UART_CTS_N_IO29
                                                 *     PIN_TYPE_PDM_CLK_IO29
                                                 *     PIN_TYPE_SPI2_IO_IO29
                                                 *     PIN_TYPE_SPI2_RXD_IO29
                                                 *     PIN_TYPE_SPI2_TXD_IO29
                                                 *     PIN_TYPE_SPI3_IO0_IO29
                                                 *     PIN_TYPE_SPI3_IO1_IO29
                                                 *     PIN_TYPE_SPI3_RXD0_IO29
                                                 *     PIN_TYPE_SPI3_RXD1_IO29
                                                 *     PIN_TYPE_SPI3_TXD0_IO29
                                                 *     PIN_TYPE_SPI3_TXD1_IO29
                                                 *     PIN_TYPE_TMS_IO29
                                                 *     PIN_TYPE_UART0_RXD_IO29
                                                 *     PIN_TYPE_UART1_CTS_N_IO29
                                                 * ***********************************
                                                 *     PIN_DRVCRNT_IO29_2mA
                                                 *     PIN_DRVCRNT_IO29_4mA
                                                 * ***********************************
                                                 *     PIN_INMODE_IO29_FLOATING
                                                 *     PIN_INMODE_IO29_PULL_UP
                                                 * *********************************** */


#define HAL_PIN_TYPE_PATCH_IO_30              (PIN_TYPE_GPIO_DISABLE_IO30       | PIN_DRVCRNT_IO30_2mA  | PIN_INMODE_IO30_FLOATING)
                                                /* ***********************************
                                                 *     PIN_TYPE_APS_SWCLK_IO30
                                                 *     PIN_TYPE_APS_UART_RXD_IO30
                                                 *     PIN_TYPE_CAM_DATA11_IO30
                                                 *     PIN_TYPE_DBG30_IO30
                                                 *     PIN_TYPE_GPIO_DISABLE_IO30
                                                 *     PIN_TYPE_GPIO_IN_IO30
                                                 *     PIN_TYPE_GPIO_OUT_HIGH_IO30
                                                 *     PIN_TYPE_GPIO_OUT_LOW_IO30
                                                 *     PIN_TYPE_I2C_SDA_IO_IO30
                                                 *     PIN_TYPE_I2S_SDI0_IO30
                                                 *     PIN_TYPE_MSQ_SWDIO_IO30
                                                 *     PIN_TYPE_MSQ_UART_RTS_N_IO30
                                                 *     PIN_TYPE_OTP_PROGRAM_PERIOD_IO30
                                                 *     PIN_TYPE_PU_EPA_OUT2_IO30
                                                 *     PIN_TYPE_SPI0_CS3_IO30
                                                 *     PIN_TYPE_SPI1_CS1_IO30
                                                 *     PIN_TYPE_SPI3_CS1_IO30
                                                 *     PIN_TYPE_TMS_IO30
                                                 *     PIN_TYPE_UART0_RXD_IO30
                                                 *     PIN_TYPE_UART1_RTS_N_IO30
                                                 * ***********************************
                                                 *     PIN_DRVCRNT_IO30_2mA
                                                 *     PIN_DRVCRNT_IO30_4mA
                                                 * ***********************************
                                                 *     PIN_INMODE_IO30_FLOATING
                                                 *     PIN_INMODE_IO30_PULL_UP
                                                 * *********************************** */


#define HAL_PIN_TYPE_PATCH_IO_31              (PIN_TYPE_GPIO_DISABLE_IO31       | PIN_DRVCRNT_IO31_2mA  | PIN_INMODE_IO31_FLOATING)
                                                /* ***********************************
                                                 *     PIN_TYPE_APS_SWDIO_IO31
                                                 *     PIN_TYPE_APS_UART_TXD_IO31
                                                 *     PIN_TYPE_CAM_PCLK_IO31
                                                 *     PIN_TYPE_DBG31_IO31
                                                 *     PIN_TYPE_GPIO_DISABLE_IO31
                                                 *     PIN_TYPE_GPIO_IN_IO31
                                                 *     PIN_TYPE_GPIO_OUT_HIGH_IO31
                                                 *     PIN_TYPE_GPIO_OUT_LOW_IO31
                                                 *     PIN_TYPE_I2C_SCL_IO_IO31
                                                 *     PIN_TYPE_I2S_SDO0_IO31
                                                 *     PIN_TYPE_MSQ_SWCLK_IO31
                                                 *     PIN_TYPE_MSQ_UART_RTS_N_IO31
                                                 *     PIN_TYPE_OTP_OPM_MODE_IO31
                                                 *     PIN_TYPE_PU_EPA_OUT3_IO31
                                                 *     PIN_TYPE_SPI3_IO3_IO31
                                                 *     PIN_TYPE_SPI3_RXD3_IO31
                                                 *     PIN_TYPE_SPI3_TXD3_IO31
                                                 *     PIN_TYPE_TDO_IO31
                                                 *     PIN_TYPE_UART0_TXD_IO31
                                                 *     PIN_TYPE_UART1_TXD_IO31
                                                 * ***********************************
                                                 *     PIN_DRVCRNT_IO31_2mA
                                                 *     PIN_DRVCRNT_IO31_4mA
                                                 * ***********************************
                                                 *     PIN_INMODE_IO31_FLOATING
                                                 *     PIN_INMODE_IO31_PULL_UP
                                                 * *********************************** */


#define HAL_PIN_TYPE_PATCH_IO_32              (PIN_TYPE_GPIO_DISABLE_IO32       | PIN_DRVCRNT_IO32_2mA  | PIN_INMODE_IO32_FLOATING)
                                                /* ***********************************
                                                 *     PIN_TYPE_APS_SWCLK_IO32
                                                 *     PIN_TYPE_APS_UART_RXD_IO32
                                                 *     PIN_TYPE_CAM_DATA2_IO32
                                                 *     PIN_TYPE_DBG0_IO32
                                                 *     PIN_TYPE_GPIO_DISABLE_IO32
                                                 *     PIN_TYPE_GPIO_IN_IO32
                                                 *     PIN_TYPE_GPIO_OUT_HIGH_IO32
                                                 *     PIN_TYPE_GPIO_OUT_LOW_IO32
                                                 *     PIN_TYPE_I2C_SCL_IO_IO32
                                                 *     PIN_TYPE_I2S_WS_OUT_IO32
                                                 *     PIN_TYPE_MSQ_SWDIO_IO32
                                                 *     PIN_TYPE_MSQ_UART_RXD_IO32
                                                 *     PIN_TYPE_OTP_PROGRAM_PERIOD_IO32
                                                 *     PIN_TYPE_PU_EPA_OUT1_IO32
                                                 *     PIN_TYPE_SPI2_CLK_IO32
                                                 *     PIN_TYPE_SPI2_IO_IO32
                                                 *     PIN_TYPE_SPI2_RXD_IO32
                                                 *     PIN_TYPE_SPI2_TXD_IO32
                                                 *     PIN_TYPE_SPI3_IO2_IO32
                                                 *     PIN_TYPE_SPI3_RXD2_IO32
                                                 *     PIN_TYPE_SPI3_TXD2_IO32
                                                 *     PIN_TYPE_TDI_IO32
                                                 *     PIN_TYPE_UART0_TXD_IO32
                                                 *     PIN_TYPE_UART1_RXD_IO32
                                                 * ***********************************
                                                 *     PIN_DRVCRNT_IO32_2mA
                                                 *     PIN_DRVCRNT_IO32_4mA
                                                 * ***********************************
                                                 *     PIN_INMODE_IO32_FLOATING
                                                 *     PIN_INMODE_IO32_PULL_UP
                                                 * *********************************** */


#define HAL_PIN_TYPE_PATCH_IO_33              (PIN_TYPE_GPIO_DISABLE_IO33       | PIN_DRVCRNT_IO33_2mA  | PIN_INMODE_IO33_FLOATING)
                                                /* ***********************************
                                                 *     PIN_TYPE_APS_SWDIO_IO33
                                                 *     PIN_TYPE_APS_UART_CTS_N_IO33
                                                 *     PIN_TYPE_CAM_DATA5_IO33
                                                 *     PIN_TYPE_DBG1_IO33
                                                 *     PIN_TYPE_GPIO_DISABLE_IO33
                                                 *     PIN_TYPE_GPIO_IN_IO33
                                                 *     PIN_TYPE_GPIO_OUT_HIGH_IO33
                                                 *     PIN_TYPE_GPIO_OUT_LOW_IO33
                                                 *     PIN_TYPE_I2C_SDA_IO_IO33
                                                 *     PIN_TYPE_I2S_SCLK_IO33
                                                 *     PIN_TYPE_MSQ_SWCLK_IO33
                                                 *     PIN_TYPE_MSQ_UART_RXD_IO33
                                                 *     PIN_TYPE_OTP_OPM_MODE_IO33
                                                 *     PIN_TYPE_PU_EPA_OUT2_IO33
                                                 *     PIN_TYPE_SPI2_CS_IN_IO33
                                                 *     PIN_TYPE_SPI2_IO_IO33
                                                 *     PIN_TYPE_SPI2_RXD_IO33
                                                 *     PIN_TYPE_SPI2_TXD_IO33
                                                 *     PIN_TYPE_SPI3_IO1_IO33
                                                 *     PIN_TYPE_SPI3_RXD1_IO33
                                                 *     PIN_TYPE_SPI3_TXD1_IO33
                                                 *     PIN_TYPE_TDO_IO33
                                                 *     PIN_TYPE_UART0_RXD_IO33
                                                 *     PIN_TYPE_UART1_CTS_N_IO33
                                                 * ***********************************
                                                 *     PIN_DRVCRNT_IO33_2mA
                                                 *     PIN_DRVCRNT_IO33_4mA
                                                 * ***********************************
                                                 *     PIN_INMODE_IO33_FLOATING
                                                 *     PIN_INMODE_IO33_PULL_UP
                                                 * *********************************** */


#define HAL_PIN_TYPE_PATCH_IO_34              (PIN_TYPE_GPIO_DISABLE_IO34       | PIN_DRVCRNT_IO34_2mA  | PIN_INMODE_IO34_FLOATING)
                                                /* ***********************************
                                                 *     PIN_TYPE_APS_SWCLK_IO34
                                                 *     PIN_TYPE_APS_UART_TXD_IO34
                                                 *     PIN_TYPE_CAM_DATA12_IO34
                                                 *     PIN_TYPE_CLK_EXT_APS_IO34
                                                 *     PIN_TYPE_CLK_EXT_MSQ_IO34
                                                 *     PIN_TYPE_DBG2_IO34
                                                 *     PIN_TYPE_GPIO_DISABLE_IO34
                                                 *     PIN_TYPE_GPIO_IN_IO34
                                                 *     PIN_TYPE_GPIO_OUT_HIGH_IO34
                                                 *     PIN_TYPE_GPIO_OUT_LOW_IO34
                                                 *     PIN_TYPE_I2S_WS_OUT_IO34
                                                 *     PIN_TYPE_MSQ_UART_TXD_IO34
                                                 *     PIN_TYPE_SPI3_CS3_IO34
                                                 *     PIN_TYPE_SPI3_IO1_IO34
                                                 *     PIN_TYPE_SPI3_RXD1_IO34
                                                 *     PIN_TYPE_SPI3_TXD1_IO34
                                                 *     PIN_TYPE_TCK_IO34
                                                 *     PIN_TYPE_UART0_RTS_N_IO34
                                                 *     PIN_TYPE_UART1_RTS_N_IO34
                                                 * ***********************************
                                                 *     PIN_DRVCRNT_IO34_2mA
                                                 *     PIN_DRVCRNT_IO34_4mA
                                                 * ***********************************
                                                 *     PIN_INMODE_IO34_FLOATING
                                                 *     PIN_INMODE_IO34_PULL_UP
                                                 * *********************************** */


#define HAL_PIN_TYPE_PATCH_IO_35              (PIN_TYPE_GPIO_DISABLE_IO35       | PIN_DRVCRNT_IO35_2mA  | PIN_INMODE_IO35_FLOATING)
                                                /* ***********************************
                                                 *     PIN_TYPE_APS_SWDIO_IO35
                                                 *     PIN_TYPE_APS_UART_CTS_N_IO35
                                                 *     PIN_TYPE_CAM_DATA13_IO35
                                                 *     PIN_TYPE_CLK_EXT_APS_IO35
                                                 *     PIN_TYPE_CLK_EXT_MSQ_IO35
                                                 *     PIN_TYPE_DBG3_IO35
                                                 *     PIN_TYPE_EXT_2IO_CLK_EN_IO35
                                                 *     PIN_TYPE_GPIO_DISABLE_IO35
                                                 *     PIN_TYPE_GPIO_IN_IO35
                                                 *     PIN_TYPE_GPIO_OUT_HIGH_IO35
                                                 *     PIN_TYPE_GPIO_OUT_LOW_IO35
                                                 *     PIN_TYPE_I2S_SDO0_IO35
                                                 *     PIN_TYPE_MSQ_UART_RTS_N_IO35
                                                 *     PIN_TYPE_OTP_PROGRAM_PERIOD_IO35
                                                 *     PIN_TYPE_PDM_RXD_IO35
                                                 *     PIN_TYPE_SPI3_IO0_IO35
                                                 *     PIN_TYPE_SPI3_RXD0_IO35
                                                 *     PIN_TYPE_SPI3_TXD0_IO35
                                                 *     PIN_TYPE_TCK_IO35
                                                 *     PIN_TYPE_UART0_TXD_IO35
                                                 *     PIN_TYPE_UART1_RTS_N_IO35
                                                 * ***********************************
                                                 *     PIN_DRVCRNT_IO35_2mA
                                                 *     PIN_DRVCRNT_IO35_4mA
                                                 * ***********************************
                                                 *     PIN_INMODE_IO35_FLOATING
                                                 *     PIN_INMODE_IO35_PULL_DOWN
                                                 * *********************************** */


#define HAL_PIN_TYPE_PATCH_IO_36              (PIN_TYPE_GPIO_DISABLE_IO36       | PIN_DRVCRNT_IO36_2mA  | PIN_INMODE_IO36_FLOATING)
                                                /* ***********************************
                                                 *     PIN_TYPE_APS_UART_RTS_N_IO36
                                                 *     PIN_TYPE_CAM_DATA14_IO36
                                                 *     PIN_TYPE_CAM_FREX_IO36
                                                 *     PIN_TYPE_CLK_DS_AUX_78K125_IO36
                                                 *     PIN_TYPE_DBG4_IO36
                                                 *     PIN_TYPE_EXT_2IO_CLK_EN_IO36
                                                 *     PIN_TYPE_GPIO_DISABLE_IO36
                                                 *     PIN_TYPE_GPIO_IN_IO36
                                                 *     PIN_TYPE_GPIO_OUT_HIGH_IO36
                                                 *     PIN_TYPE_GPIO_OUT_LOW_IO36
                                                 *     PIN_TYPE_I2S_SDI0_IO36
                                                 *     PIN_TYPE_MSQ_UART_RXD_IO36
                                                 *     PIN_TYPE_PDM_RXD_IO36
                                                 *     PIN_TYPE_SPI2_IO_IO36
                                                 *     PIN_TYPE_SPI2_RXD_IO36
                                                 *     PIN_TYPE_SPI2_TXD_IO36
                                                 *     PIN_TYPE_SPI3_IO0_IO36
                                                 *     PIN_TYPE_SPI3_IO1_IO36
                                                 *     PIN_TYPE_SPI3_RXD0_IO36
                                                 *     PIN_TYPE_SPI3_RXD1_IO36
                                                 *     PIN_TYPE_SPI3_TXD0_IO36
                                                 *     PIN_TYPE_SPI3_TXD1_IO36
                                                 *     PIN_TYPE_TDO_IO36
                                                 *     PIN_TYPE_UART0_TXD_IO36
                                                 *     PIN_TYPE_UART1_TXD_IO36
                                                 * ***********************************
                                                 *     PIN_DRVCRNT_IO36_2mA
                                                 *     PIN_DRVCRNT_IO36_4mA
                                                 * ***********************************
                                                 *     PIN_INMODE_IO36_FLOATING
                                                 *     PIN_INMODE_IO36_PULL_UP
                                                 * *********************************** */


#define HAL_PIN_TYPE_PATCH_IO_37              (PIN_TYPE_GPIO_DISABLE_IO37       | PIN_DRVCRNT_IO37_2mA  | PIN_INMODE_IO37_FLOATING)
                                                /* ***********************************
                                                 *     PIN_TYPE_APS_UART_RTS_N_IO37
                                                 *     PIN_TYPE_CAM_DATA15_IO37
                                                 *     PIN_TYPE_CAM_EXPST_IO37
                                                 *     PIN_TYPE_DBG5_IO37
                                                 *     PIN_TYPE_GPIO_DISABLE_IO37
                                                 *     PIN_TYPE_GPIO_IN_IO37
                                                 *     PIN_TYPE_GPIO_OUT_HIGH_IO37
                                                 *     PIN_TYPE_GPIO_OUT_LOW_IO37
                                                 *     PIN_TYPE_I2S_SCLK_IO37
                                                 *     PIN_TYPE_MSQ_UART_RTS_N_IO37
                                                 *     PIN_TYPE_OTP_OPM_MODE_IO37
                                                 *     PIN_TYPE_PU_EPA_OUT3_IO37
                                                 *     PIN_TYPE_SPI1_CS1_IO37
                                                 *     PIN_TYPE_SPI3_CS2_IO37
                                                 *     PIN_TYPE_TCK_IO37
                                                 *     PIN_TYPE_UART0_RTS_N_IO37
                                                 *     PIN_TYPE_UART1_RXD_IO37
                                                 * ***********************************
                                                 *     PIN_DRVCRNT_IO37_2mA
                                                 *     PIN_DRVCRNT_IO37_4mA
                                                 * ***********************************
                                                 *     PIN_INMODE_IO37_FLOATING
                                                 *     PIN_INMODE_IO37_PULL_UP
                                                 * *********************************** */


#define HAL_PIN_TYPE_PATCH_IO_38              (PIN_TYPE_GPIO_DISABLE_IO38       | PIN_DRVCRNT_IO38_2mA  | PIN_INMODE_IO38_FLOATING)
                                                /* ***********************************
                                                 *     PIN_TYPE_APS_UART_RTS_N_IO38
                                                 *     PIN_TYPE_CLK_EXT_APS_IO38
                                                 *     PIN_TYPE_CLK_EXT_MSQ_IO38
                                                 *     PIN_TYPE_DBG6_IO38
                                                 *     PIN_TYPE_GPIO_DISABLE_IO38
                                                 *     PIN_TYPE_GPIO_IN_IO38
                                                 *     PIN_TYPE_GPIO_OUT_HIGH_IO38
                                                 *     PIN_TYPE_GPIO_OUT_LOW_IO38
                                                 *     PIN_TYPE_I2S_SDO0_IO38
                                                 *     PIN_TYPE_MSQ_UART_CTS_N_IO38
                                                 *     PIN_TYPE_SPI0_CS3_IO38
                                                 *     PIN_TYPE_SPI3_CLK_IO38
                                                 *     PIN_TYPE_SPI3_CS3_IO38
                                                 *     PIN_TYPE_UART0_RTS_N_IO38
                                                 *     PIN_TYPE_UART1_CTS_N_IO38
                                                 * ***********************************
                                                 *     PIN_DRVCRNT_IO38_2mA
                                                 *     PIN_DRVCRNT_IO38_4mA
                                                 * ***********************************
                                                 *     PIN_INMODE_IO38_FLOATING
                                                 *     PIN_INMODE_IO38_PULL_UP
                                                 * *********************************** */


#define HAL_PIN_TYPE_PATCH_IO_SIP_39          (PIN_TYPE_GPIO_DISABLE_IO39       | PIN_DRVCRNT_IO39_2mA  | PIN_INMODE_IO39_FLOATING)
                                                /* ***********************************
                                                 *     PIN_TYPE_DBG12_IO39
                                                 *     PIN_TYPE_GPIO_DISABLE_IO39
                                                 *     PIN_TYPE_GPIO_OUT_HIGH_IO39
                                                 *     PIN_TYPE_GPIO_OUT_LOW_IO39
                                                 *     PIN_TYPE_PSRAM_IO0_IO39
                                                 *     PIN_TYPE_PSRAM_RXD0_IO39
                                                 *     PIN_TYPE_PSRAM_TXD0_IO39
                                                 * ***********************************
                                                 *     PIN_DRVCRNT_IO39_2mA
                                                 *     PIN_DRVCRNT_IO39_4mA
                                                 * ***********************************
                                                 *     PIN_INMODE_IO39_FLOATING
                                                 *     PIN_INMODE_IO39_PULL_UP
                                                 * *********************************** */


#define HAL_PIN_TYPE_PATCH_IO_SIP_40          (PIN_TYPE_GPIO_DISABLE_IO40       | PIN_DRVCRNT_IO40_2mA  | PIN_INMODE_IO40_FLOATING)
                                                /* ***********************************
                                                 *     PIN_TYPE_DBG12_IO40
                                                 *     PIN_TYPE_GPIO_DISABLE_IO40
                                                 *     PIN_TYPE_GPIO_OUT_HIGH_IO40
                                                 *     PIN_TYPE_GPIO_OUT_LOW_IO40
                                                 *     PIN_TYPE_PSRAM_IO1_IO40
                                                 *     PIN_TYPE_PSRAM_RXD1_IO40
                                                 *     PIN_TYPE_PSRAM_TXD1_IO40
                                                 * ***********************************
                                                 *     PIN_DRVCRNT_IO40_2mA
                                                 *     PIN_DRVCRNT_IO40_4mA
                                                 * ***********************************
                                                 *     PIN_INMODE_IO40_FLOATING
                                                 *     PIN_INMODE_IO40_PULL_UP
                                                 * *********************************** */


#define HAL_PIN_TYPE_PATCH_IO_SIP_41          (PIN_TYPE_GPIO_DISABLE_IO41       | PIN_DRVCRNT_IO41_2mA  | PIN_INMODE_IO41_FLOATING)
                                                /* ***********************************
                                                 *     PIN_TYPE_DBG12_IO41
                                                 *     PIN_TYPE_GPIO_DISABLE_IO41
                                                 *     PIN_TYPE_GPIO_OUT_HIGH_IO41
                                                 *     PIN_TYPE_GPIO_OUT_LOW_IO41
                                                 *     PIN_TYPE_PSRAM_IO2_IO41
                                                 *     PIN_TYPE_PSRAM_RXD2_IO41
                                                 *     PIN_TYPE_PSRAM_TXD2_IO41
                                                 * ***********************************
                                                 *     PIN_DRVCRNT_IO41_2mA
                                                 *     PIN_DRVCRNT_IO41_4mA
                                                 * ***********************************
                                                 *     PIN_INMODE_IO41_FLOATING
                                                 *     PIN_INMODE_IO41_PULL_UP
                                                 * *********************************** */


#define HAL_PIN_TYPE_PATCH_IO_SIP_42          (PIN_TYPE_GPIO_DISABLE_IO42       | PIN_DRVCRNT_IO42_2mA  | PIN_INMODE_IO42_FLOATING)
                                                /* ***********************************
                                                 *     PIN_TYPE_DBG12_IO42
                                                 *     PIN_TYPE_GPIO_DISABLE_IO42
                                                 *     PIN_TYPE_GPIO_OUT_HIGH_IO42
                                                 *     PIN_TYPE_GPIO_OUT_LOW_IO42
                                                 *     PIN_TYPE_PSRAM_IO3_IO42
                                                 *     PIN_TYPE_PSRAM_RXD3_IO42
                                                 *     PIN_TYPE_PSRAM_TXD3_IO42
                                                 * ***********************************
                                                 *     PIN_DRVCRNT_IO42_2mA
                                                 *     PIN_DRVCRNT_IO42_4mA
                                                 * ***********************************
                                                 *     PIN_INMODE_IO42_FLOATING
                                                 *     PIN_INMODE_IO42_PULL_UP
                                                 * *********************************** */


#define HAL_PIN_TYPE_PATCH_IO_SIP_43          (PIN_TYPE_GPIO_DISABLE_IO43       | PIN_DRVCRNT_IO43_2mA  | PIN_INMODE_IO43_FLOATING)
                                                /* ***********************************
                                                 *     PIN_TYPE_DBG15_IO43
                                                 *     PIN_TYPE_GPIO_DISABLE_IO43
                                                 *     PIN_TYPE_GPIO_OUT_HIGH_IO43
                                                 *     PIN_TYPE_GPIO_OUT_LOW_IO43
                                                 *     PIN_TYPE_SPI0_CS1_IO43
                                                 *     PIN_TYPE_SPI0_CS2_IO43
                                                 *     PIN_TYPE_SPI0_CS3_IO43
                                                 *     PIN_TYPE_UART1_RTS_N_IO43
                                                 * ***********************************
                                                 *     PIN_DRVCRNT_IO43_2mA
                                                 *     PIN_DRVCRNT_IO43_4mA
                                                 * ***********************************
                                                 *     PIN_INMODE_IO43_FLOATING
                                                 *     PIN_INMODE_IO43_PULL_UP
                                                 * *********************************** */


#define HAL_PIN_TYPE_PATCH_IO_SIP_44          (PIN_TYPE_GPIO_DISABLE_IO44       | PIN_DRVCRNT_IO44_2mA  | PIN_INMODE_IO44_FLOATING)
                                                /* ***********************************
                                                 *     PIN_TYPE_DBG11_IO44
                                                 *     PIN_TYPE_GPIO_DISABLE_IO44
                                                 *     PIN_TYPE_GPIO_OUT_HIGH_IO44
                                                 *     PIN_TYPE_GPIO_OUT_LOW_IO44
                                                 *     PIN_TYPE_SPI0_CLK_IO44
                                                 *     PIN_TYPE_TDO_IO44
                                                 *     PIN_TYPE_UART1_TXD_IO44
                                                 * ***********************************
                                                 *     PIN_DRVCRNT_IO44_2mA
                                                 *     PIN_DRVCRNT_IO44_4mA
                                                 * ***********************************
                                                 *     PIN_INMODE_IO44_FLOATING
                                                 *     PIN_INMODE_IO44_PULL_UP
                                                 * *********************************** */












/********************************************
Declaration of data structure
********************************************/
// Sec 3: structure, uniou, enum, linked list...

/********************************************
Declaration of Global Variables & Functions
********************************************/
// Sec 4: declaration of global  variable

// Sec 5: declaration of global function prototype

/***************************************************
Declaration of static Global Variables &  Functions
***************************************************/
// Sec 6: declaration of static global  variable

// Sec 7: declaration of static function prototype

#ifdef __cplusplus
}
#endif

#endif  // end of __HAL_PIN_CONFIG_PROJECT_H__
