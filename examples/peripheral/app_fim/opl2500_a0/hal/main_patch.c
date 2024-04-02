/******************************************************************************
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
*  main_patch.c
*
*  Project:
*  --------
*  OPL2500 Project - the main patch implement file
*
*  Description:
*  ------------
*  This implement file is include the main patch function and api.
*
*  Author:
*  -------
*  FW team
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
#include "at_cmd_common.h"
#include "boot_sequence.h"
#include "freertos_cmsis.h"
#include "hal_system.h"
#include "hal_auxadc.h"
#include "hal_pin.h"
#include "hal_pin_def.h"
#include "hal_pin_config_project.h"
#include "hal_dbg_uart.h"
#include "hal_vic.h"
#include "hal_wdt.h"
#include "mw_fim.h"
#include "mw_fim_default.h"
#include "ps.h"
#include "sys_init.h"
#include "sys_cfg.h"
#include "sys_os_config.h"

#include "app_main.h"
#include "mw_fim_default_version_project.h"
#include "mw_fim_default_group11_project.h"
#include "mw_fim_default_group12_project.h"
#include "mw_fim_default_group16_project.h"
#include "mw_fim_default_group21_project.h"
#include "mw_fim_default_group01.h" // For EXT_PA_ENABLED
#include "qd_config.h"
#include "qd_module.h"
#include "qd_fwk_ver.h"
#include "svn_rev_patch.h"

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
static void Main_HeapPatchInit(void);
static void Main_PinMuxUpdate(void);
static void Main_MiscModulesInit(void);
static void Main_FlashLayoutUpdate(void);
static void Main_AppInit_patch(void);
static void Main_AtUartDbgUartSwitch(void);

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

    /* APS_HEAP_START and APS_HEAP_LENGTH are from scatter/linker file
     * When needs to change HEAP size, please modify scatter/linker file.
     * Do NOT write the argument here */
    osHeapAssign(APS_HEAP_START, APS_HEAP_LENGTH);
    Main_HeapPatchInit();
    
    // the initial of driver part for cold and warm boot
    Sys_MiscModulesInit = Main_MiscModulesInit;
    
    // update the switch AT UART / dbg UART function
    at_cmd_switch_uart1_dbguart = Main_AtUartDbgUartSwitch;
    
    // application init
    Sys_AppInit = Main_AppInit_patch;
}

/*************************************************************************
* FUNCTION:
*   Main_HeadPatchInit
*
* DESCRIPTION:
*   Update HEAP setting here.
*   This function must be run after osHeapAssign. i.e. HEAP size updated first.
*
* PARAMETERS
*   none
*
* RETURNS
*   none
*
*************************************************************************/
static void Main_HeapPatchInit(void)
{
#if SVN_REVISION_PATCH >= 6440
   osMemoryDef_t PartitionMemoryTable[MAX_NUM_MEM_POOL] = {
        /* {block_size, number}
         *     block_size: The memory block max allocation size.
         *     number: The number of the memory block of given block size.
         * The order of block size must be small to big.
         * When block_size or block_num is zero, it means end of table.
         *
         *{block_size, number} */
         {        32,     48},
         {        64,     32},
         {       128,     64},
         {       256,     28},
         {       512,     12},
         {         0,      0},
         {         0,      0},
         {         0,      0}
   };
    
   osMemoryPoolUpdate(PartitionMemoryTable);
#endif
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

    // Update FIM Version
    MwFim_GroupVersionUpdate(1, 1, MW_FIM_VER11_PROJECT);
    MwFim_GroupVersionUpdate(1, 2, MW_FIM_VER12_PROJECT);
    MwFim_GroupVersionUpdate(1, 3, MW_FIM_VER13_PROJECT);
    MwFim_GroupVersionUpdate(1, 4, MW_FIM_VER14_PROJECT);
    MwFim_GroupVersionUpdate(1, 5, MW_FIM_VER15_PROJECT);
    MwFim_GroupVersionUpdate(1, 6, MW_FIM_VER16_PROJECT);
    MwFim_GroupVersionUpdate(1, 7, MW_FIM_VER17_PROJECT);
    MwFim_GroupVersionUpdate(1, 8, MW_FIM_VER18_PROJECT);

    // user FIM implement ============================================

    // Apply zone2, flash address from 0x000A0000
    g_taMwFimZoneInfoTable[2].ulBaseAddr = 0x000A0000;
    g_taMwFimZoneInfoTable[2].ulBlockNum = 9;

    // Register FIM table
    MwFim_GroupInfoUpdate(2, 1, (T_MwFimFileInfo *)g_taMwFimGroupTable21_project);

    // Update FIM Version
    MwFim_GroupVersionUpdate(2, 1, MW_FIM_VER21_PROJECT);
    MwFim_GroupVersionUpdate(2, 2, MW_FIM_VER22_PROJECT);
    MwFim_GroupVersionUpdate(2, 3, MW_FIM_VER23_PROJECT);
    MwFim_GroupVersionUpdate(2, 4, MW_FIM_VER24_PROJECT);
    MwFim_GroupVersionUpdate(2, 5, MW_FIM_VER25_PROJECT);
    MwFim_GroupVersionUpdate(2, 6, MW_FIM_VER26_PROJECT);
    MwFim_GroupVersionUpdate(2, 7, MW_FIM_VER27_PROJECT);
    MwFim_GroupVersionUpdate(2, 8, MW_FIM_VER28_PROJECT);
}

/*************************************************************************
* FUNCTION:
*   Main_MiscModulesInit
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
static void Main_MiscModulesInit(void)
{
    //Hal_Wdt_Stop();   //disable watchdog here.

    // initialize aux driver
    Hal_Aux_Init();

#if (EXT_PA_ENABLED == 1)
    /*
     * Two steps to active ext-Pa mode:
     *
     * Step 1) Config " hal_pin_config_project.h "
     *         1-1) Assigned three pins according to schematic:
     *                  [ TX_EN ]
     *                  [ RX_EN ]
     *                  [ LNA_EN ]
     *              The three pins are MUST assinged to PIN_TYPE_GPIO_OUT_LOW.
     *         1-2) (Optional) (0xFF for not exist)
                    Assigned the pin : PwrCtrl according to schematic.
     *                  [ Pwr Ctrl ]
     *              The three pins are MUST assinged to PIN_TYPE_GPIO_OUT_LOW.
     *
     * Step 2) Set Hal_ExtPa_Pin_Set(), default value was disable.
     *
     */
    Hal_ExtPa_Pin_Set( 4, 6, 3, 5); /* No PwrCtrl case */
    // Hal_ExtPa_Pin_Set( 4, 6, 18, 5); /* PwrCtrl case */

    // Force Wifi pwr to ext-PA level
    uint8_t u8Temp = 0;
    MwFim_FileRead(MW_FIM_IDX_GP01_RF_CFG, 0, MW_FIM_RF_CFG_SIZE, &u8Temp);
    if(u8Temp < 0xE0)
    {
        u8Temp = 0xE0;
        MwFim_FileWrite(MW_FIM_IDX_GP01_RF_CFG, 0, MW_FIM_RF_CFG_SIZE, &u8Temp);
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

        Hal_Pin_Config(PIN_TYPE_UART1_TXD_IO22 | PIN_INMODE_IO5_FLOATING);
        Hal_Pin_Config(PIN_TYPE_UART1_RXD_IO1 | PIN_INMODE_IO1_PULL_UP);
    }
    else
    {
        Hal_Pin_Config(PIN_TYPE_UART1_TXD_IO0 | PIN_INMODE_IO0_FLOATING);
        Hal_Pin_Config(PIN_TYPE_UART1_RXD_IO2 | PIN_INMODE_IO2_PULL_UP);
        
        Hal_Pin_Config(PIN_TYPE_APS_UART_TXD_IO22 | PIN_INMODE_IO5_FLOATING);
        Hal_Pin_Config(PIN_TYPE_APS_UART_RXD_IO1 | PIN_INMODE_IO1_PULL_UP);
    }
    
    g_eAppMainUartMode = (E_PIN_MAIN_UART_MODE)!g_eAppMainUartMode;
}

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
