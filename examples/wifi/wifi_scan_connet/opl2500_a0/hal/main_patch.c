/******************************************************************************
*  Copyright 2017 - 2018, Opulinks Technology Ltd.
*  ----------------------------------------------------------------------------
*  Statement:
*  ----------
*  This software is protected by Copyright and the information contained
*  herein is confidential. The software may not be copied and the information
*  contained herein may not be used or disclosed except with the written
*  permission of Opulinks Technology Ltd. (C) 2018
******************************************************************************/

/******************************************************************************
*  Filename:
*  ---------
*  main_patch.c
*
*  Project:
*  --------
*  OPL1000 Project - the main patch implement file
*
*  Description:
*  ------------
*  This implement file is include the main patch function and api.
*
*  Author:
*  -------
*  SDK Team
*
******************************************************************************/
/***********************
Head Block of The File
***********************/
// Sec 0: Comment block of the file


// Sec 1: Include File
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include "event_loop.h"
#include "wifi_api.h"
#include "wifi_event.h"
#include "wifi_event_handler.h"
#include "lwip_helper.h"
#include "at_cmd_common.h"
#include "at_cmd_tcpip.h"
#include "at_cmd_wifi.h"
#include "at_cmd_app.h"
#include "at_cmd_property.h"
#include "at_cmd_msg_ext.h"
#include "sys_init.h"
#include "sys_init_patch.h"
#include "sys_cfg.h"
#include "hal_system.h"
#include "mw_fim.h"
#include "mw_fim_default.h"
#include "cmsis_os.h"
#include "sys_os_config.h"
#include "hal_auxadc.h"
#include "hal_pin.h"
#include "hal_pin_def.h"
#include "hal_pin_config_project.h"
#include "hal_dbg_uart.h"
#include "hal_vic.h"
#include "hal_wdt.h"
#include "boot_sequence.h"
#include "at_cmd_common.h"
#include "freertos_cmsis.h"
#include "ps.h"

#include "app_main.h"
#include "mw_fim_default_version_project.h"
#include "mw_fim_default_group11_project.h"
#include "mw_fim_default_group12_project.h"
#include "mw_fim_default_group16_project.h"
#include "qd_config.h"
#include "qd_module.h"
#include "qd_fwk_ver.h"

// Sec 2: Constant Definitions, Imported Symbols, miscellaneous

/********************************************
Declaration of data structure
********************************************/
// Sec 3: structure, uniou, enum, linked list

/********************************************
Declaration of Global Variables & Functions
********************************************/
// Sec 4: declaration of global variable
extern uint8_t* g_ucaMemPartAddr;
extern uint32_t g_ulMemPartTotalSize;

// Sec 5: declaration of global function prototype


/***************************************************
Declaration of static Global Variables & Functions
***************************************************/
// Sec 6: declaration of static global variable
static E_PIN_MAIN_UART_MODE g_eAppMainUartMode = PIN_MAIN_UART_MODE_AT;

// Sec 7: declaration of static function prototype
void __Patch_EntryPoint(void) __attribute__((section("ENTRY_POINT")));
void __Patch_EntryPoint(void) __attribute__((used));
static void Main_PinMuxUpdate(void);
static void Main_FlashLayoutUpdate(void);
static void Main_AppInit_patch(void);
static void Main_MiscDriverConfigSetup(void);
static void Main_AtUartDbgUartSwitch(void);
#if 0
static void Main_ApsUartRxDectecConfig(void);
#endif
#ifdef __APS_UART_DETECT__ // !!! don't detect the IO pin, bcz it will be sometimmes false alarm.
static void Main_ApsUartRxDectecCb(E_GpioIdx_t tGpioIdx);
#endif

/***********
C Functions
***********/
// Sec 8: C Functions

/*************************************************************************
* FUNCTION:
*   __Patch_EntryPoint
*
* DESCRIPTION:
*   the entry point of SW patch
*
* PARAMETERS
*   none
*
* RETURNS
*   none
*
*************************************************************************/
void __Patch_EntryPoint(void)
{
    // don't remove this code
    SysInit_EntryPoint();

#if (SYS_CFG_32K_SWITCH_TO_RC == 1)
    // Uncomment this function when the device is without 32k XTAL.
    Sys_SwitchTo32kRC();
#endif
    
    // update the pin mux
    Hal_SysPinMuxAppInit = Main_PinMuxUpdate;
    
    // update the flash layout
    MwFim_FlashLayoutUpdate = Main_FlashLayoutUpdate;

    osHeapAssign(APS_HEAP_START, APS_HEAP_LENGTH);
    //Sys_SetUnsuedSramEndBound(0x43E000);
    
    // the initial of driver part for cold and warm boot
    Sys_MiscModulesInit = Main_MiscDriverConfigSetup;
    
    // update the switch AT UART / dbg UART function
    at_cmd_switch_uart1_dbguart = Main_AtUartDbgUartSwitch;
    
    // application init
    Sys_AppInit = Main_AppInit_patch;
}

/*************************************************************************
* FUNCTION:
*   Main_PinMuxUpdate
*
* DESCRIPTION:
*   update the flash layout
*
* PARAMETERS
*   none
*
* RETURNS
*   none
*
*************************************************************************/
static void Main_PinMuxUpdate(void)
{
    Hal_SysDisableAllTxPeriphPinmux();
    Hal_SysDisableAllRxPin();

    Hal_Pin_Config(HAL_PIN_TYPE_PATCH_IO_0);
    Hal_Pin_Config(HAL_PIN_TYPE_PATCH_IO_1);
    Hal_Pin_Config(HAL_PIN_TYPE_PATCH_IO_2);
    Hal_Pin_Config(HAL_PIN_TYPE_PATCH_IO_3);
    Hal_Pin_Config(HAL_PIN_TYPE_PATCH_IO_4);
    Hal_Pin_Config(HAL_PIN_TYPE_PATCH_IO_5);
    Hal_Pin_Config(HAL_PIN_TYPE_PATCH_IO_6);
    Hal_Pin_Config(HAL_PIN_TYPE_PATCH_IO_7);
    Hal_Pin_Config(HAL_PIN_TYPE_PATCH_IO_8);
    Hal_Pin_Config(HAL_PIN_TYPE_PATCH_IO_9);
    Hal_Pin_Config(HAL_PIN_TYPE_PATCH_IO_10);
    Hal_Pin_Config(HAL_PIN_TYPE_PATCH_IO_11);
    Hal_Pin_Config(HAL_PIN_TYPE_PATCH_IO_12);
    Hal_Pin_Config(HAL_PIN_TYPE_PATCH_IO_13);
    Hal_Pin_Config(HAL_PIN_TYPE_PATCH_IO_14);
    Hal_Pin_Config(HAL_PIN_TYPE_PATCH_IO_15);
    Hal_Pin_Config(HAL_PIN_TYPE_PATCH_IO_16);
    Hal_Pin_Config(HAL_PIN_TYPE_PATCH_IO_17);
    Hal_Pin_Config(HAL_PIN_TYPE_PATCH_IO_18);
    Hal_Pin_Config(HAL_PIN_TYPE_PATCH_IO_19);
    Hal_Pin_Config(HAL_PIN_TYPE_PATCH_IO_20);
    Hal_Pin_Config(HAL_PIN_TYPE_PATCH_IO_21);
    Hal_Pin_Config(HAL_PIN_TYPE_PATCH_IO_22);
    Hal_Pin_Config(HAL_PIN_TYPE_PATCH_IO_23);
    Hal_Pin_Config(HAL_PIN_TYPE_PATCH_IO_24);
    Hal_Pin_Config(HAL_PIN_TYPE_PATCH_IO_25);
    Hal_Pin_Config(HAL_PIN_TYPE_PATCH_IO_26);
    Hal_Pin_Config(HAL_PIN_TYPE_PATCH_IO_27);
    Hal_Pin_Config(HAL_PIN_TYPE_PATCH_IO_28);
    Hal_Pin_Config(HAL_PIN_TYPE_PATCH_IO_29);
    Hal_Pin_Config(HAL_PIN_TYPE_PATCH_IO_30);
    Hal_Pin_Config(HAL_PIN_TYPE_PATCH_IO_31);
    Hal_Pin_Config(HAL_PIN_TYPE_PATCH_IO_32);
    Hal_Pin_Config(HAL_PIN_TYPE_PATCH_IO_33);
    Hal_Pin_Config(HAL_PIN_TYPE_PATCH_IO_34);
    Hal_Pin_Config(HAL_PIN_TYPE_PATCH_IO_35);
    Hal_Pin_Config(HAL_PIN_TYPE_PATCH_IO_36);
    Hal_Pin_Config(HAL_PIN_TYPE_PATCH_IO_37);

    /* SiP IO */
    Hal_Pin_Config(HAL_PIN_TYPE_PATCH_IO_SIP_39);
    Hal_Pin_Config(HAL_PIN_TYPE_PATCH_IO_SIP_40);
    Hal_Pin_Config(HAL_PIN_TYPE_PATCH_IO_SIP_41);
    Hal_Pin_Config(HAL_PIN_TYPE_PATCH_IO_SIP_42);
    Hal_Pin_Config(HAL_PIN_TYPE_PATCH_IO_SIP_43);
    Hal_Pin_Config(HAL_PIN_TYPE_PATCH_IO_SIP_44);
    
    Hal_Pin_UpdatePsramCfg();
    
    // at_io01_uart_mode_set(HAL_PIN_MAIN_UART_MODE_PATCH);
}

/*************************************************************************
* FUNCTION:
*   Main_FlashLayoutUpdate
*
* DESCRIPTION:
*   update the flash layout
*
* PARAMETERS
*   none
*
* RETURNS
*   none
*
*************************************************************************/
static void Main_FlashLayoutUpdate(void)
{
    g_taMwFimZoneInfoTable[1].ulBaseAddr = 0x00090000;
    g_taMwFimZoneInfoTable[1].ulBlockNum = 9;

    // Register FIM table
#if (WM_ENABLED == 1)
    MwFim_GroupInfoUpdate(1, 1, (T_MwFimFileInfo *)g_taMwFimGroupTable11_project);
#endif

    MwFim_GroupInfoUpdate(1, 2, (T_MwFimFileInfo *)g_taMwFimGroupTable12_project);
	
#if (BAT_MEAS_ENABLED == 1)
    MwFim_GroupInfoUpdate(1, 6, (T_MwFimFileInfo *)g_taMwFimGroupTable16_project);
#endif
    
    // user implement

    // Update FIM Version
    MwFim_GroupVersionUpdate(1, 1, MW_FIM_VER11_PROJECT);
    MwFim_GroupVersionUpdate(1, 2, MW_FIM_VER12_PROJECT);
    MwFim_GroupVersionUpdate(1, 3, MW_FIM_VER13_PROJECT);
    MwFim_GroupVersionUpdate(1, 4, MW_FIM_VER14_PROJECT);
    MwFim_GroupVersionUpdate(1, 5, MW_FIM_VER15_PROJECT);
    MwFim_GroupVersionUpdate(1, 6, MW_FIM_VER16_PROJECT);
    MwFim_GroupVersionUpdate(1, 7, MW_FIM_VER17_PROJECT);
    MwFim_GroupVersionUpdate(1, 8, MW_FIM_VER18_PROJECT);
}

/*************************************************************************
* FUNCTION:
*   Main_MiscDriverConfigSetup
*
* DESCRIPTION:
*   the initial of driver part for cold and warm boot
*
* PARAMETERS
*   none
*
* RETURNS
*   none
*
*************************************************************************/
static void Main_MiscDriverConfigSetup(void)
{
    //Hal_Wdt_Stop();   //disable watchdog here.

    // initialize aux driver
    Hal_Aux_Init();

#if 0
    // IO 1 : detect the GPIO high level if APS UART Rx is connected to another UART Tx port.
    // cold boot
    if (0 == Boot_CheckWarmBoot())
    {
        Hal_DbgUart_RxIntEn(0);
        
        if ((HAL_PIN_TYPE_PATCH_IO_1 && PIN_TYPE_UART1_TXD_IO0) == PIN_TYPE_UART1_TXD_IO0)
        {
            Main_ApsUartRxDectecConfig();
        }
    }
#endif
}


/*************************************************************************
* FUNCTION:
*   Main_AtUartDbgUartSwitch
*
* DESCRIPTION:
*   switch the UART1 and dbg UART
*
* PARAMETERS
*   none
*
* RETURNS
*   none
*
*************************************************************************/
static void Main_AtUartDbgUartSwitch(void)
{
    if (g_eAppMainUartMode == PIN_MAIN_UART_MODE_AT)
    {
        Hal_Pin_Config(PIN_TYPE_APS_UART_TXD_IO0 | PIN_INMODE_IO0_FLOATING);
        Hal_Pin_Config(PIN_TYPE_APS_UART_RXD_IO2 | PIN_INMODE_IO2_PULL_UP);

        Hal_Pin_Config(PIN_TYPE_UART1_TXD_IO5 | PIN_INMODE_IO5_FLOATING);
        Hal_Pin_Config(PIN_TYPE_UART1_RXD_IO1 | PIN_INMODE_IO1_PULL_UP);
    }
    else
    {
        Hal_Pin_Config(PIN_TYPE_UART1_TXD_IO0 | PIN_INMODE_IO0_FLOATING);
        Hal_Pin_Config(PIN_TYPE_UART1_RXD_IO2 | PIN_INMODE_IO2_PULL_UP);
        
        Hal_Pin_Config(PIN_TYPE_APS_UART_TXD_IO5 | PIN_INMODE_IO5_FLOATING);
        Hal_Pin_Config(PIN_TYPE_APS_UART_RXD_IO1 | PIN_INMODE_IO1_PULL_UP);
    }
    
    g_eAppMainUartMode = (E_PIN_MAIN_UART_MODE)!g_eAppMainUartMode;
}


/*************************************************************************
* FUNCTION:
*   Main_ApsUartRxDectecConfig
*
* DESCRIPTION:
*   detect the GPIO high level if APS UART Rx is connected to another UART Tx port.
*
* PARAMETERS
*   none
*
* RETURNS
*   none
*
*************************************************************************/
#if 0
static void Main_ApsUartRxDectecConfig(void)
{
    E_GpioLevel_t eGpioLevel;

    Hal_Pin_ConfigSet(1, PIN_TYPE_GPIO_INPUT, PIN_DRIVING_LOW);
    eGpioLevel = Hal_Vic_GpioInput(GPIO_IDX_01);
    if (GPIO_LEVEL_HIGH == eGpioLevel)
    {
        // it is connected
        Hal_Pin_ConfigSet(1, HAL_PIN_TYPE_IO_1, HAL_PIN_DRIVING_IO_1);
        Hal_DbgUart_RxIntEn(1);
    }
    else //if (GPIO_LEVEL_LOW == eGpioLevel)
    {
#ifdef __APS_UART_DETECT__ // !!! don't detect the IO pin, bcz it will be sometimes false alarm.
        // it is not conncected, set the high level to trigger the GPIO interrupt
        Hal_Vic_GpioCallBackFuncSet(GPIO_IDX_01, Main_ApsUartRxDectecCb);
        //Hal_Vic_GpioDirection(GPIO_IDX_01, GPIO_INPUT);
        Hal_Vic_GpioIntTypeSel(GPIO_IDX_01, INT_TYPE_LEVEL);
        Hal_Vic_GpioIntInv(GPIO_IDX_01, 0);
        Hal_Vic_GpioIntMask(GPIO_IDX_01, 0);
        Hal_Vic_GpioIntEn(GPIO_IDX_01, 1);
#endif
    }
}
#endif

/*************************************************************************
* FUNCTION:
*   Main_ApsUartRxDectecCb
*
* DESCRIPTION:
*   detect the GPIO high level if APS UART Rx is connected to another UART Tx port.
*
* PARAMETERS
*   1. tGpioIdx : Index of call-back GPIO
*
* RETURNS
*   none
*
*************************************************************************/
#ifdef __APS_UART_DETECT__ // !!! done't detect the IO pin, bcz it will be sometimes false alarm.
static void Main_ApsUartRxDectecCb(E_GpioIdx_t tGpioIdx)
{
#if 0
    // disable the GPIO interrupt
    Hal_Vic_GpioIntEn(GPIO_IDX_01, 0);

    // it it connected
    Hal_Pin_ConfigSet(1, HAL_PIN_TYPE_IO_1, HAL_PIN_DRIVING_IO_1);
    Hal_DbgUart_RxIntEn(1);
#endif
}
#endif

/*************************************************************************
* FUNCTION:
*   Main_AppInit_patch
*
* DESCRIPTION:
*   the initial of application
*
* PARAMETERS
*   none
*
* RETURNS
*   none
*
*************************************************************************/
static void Main_AppInit_patch(void)
{
    printf("====================================\r\n");
    printf("Quick Dev Framework\r\n");
    printf("State: %s\r\n", QD_FWK_RELEASE_STATE);
    printf("Version: %s\r\n", QD_FWK_RELEASE_VER);
    printf("Date: %s\r\n", QD_FWK_RELEASE_DATE);
    printf("====================================\r\n");

#if defined(OPL2500_A0)
    at_cmd_wifi_func_init();
    at_cmd_app_func_preinit();
    at_cmd_property_func_init();
    at_msg_ext_init();

    // wifi_init(NULL, NULL);
#endif

    // add the application initialization from here

#if (SYS_CFG_32K_SWITCH_TO_RC == 1)
#else
    ps_32k_xtal_measure(200);
#endif

    sys_cfg_rf_init(NULL);

    sys_cfg_clk_set(SYS_CFG_CLK_RATE);

    // set default tracer level in user app type to INFO, WARN, ERRO (DEBG not open)
    tracer_def_level_set(TRACER_TASK_TYPE_APP, 0x06);

    APP_MainInit();
}
