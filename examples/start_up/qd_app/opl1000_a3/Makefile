##### ARM GCC Toolchain Development #####
SDK_ROOT = ../../../../sdk/SDK
QD_ROOT = ../../../../quick_dev
EXAMPLE_ROOT = ..
PROJECT_ROOT = .

OUTPUT_FOLDER    = ./Output-GCC
LD_SCRIPT = $(SDK_ROOT)/APS/project/opl1000/link.ld
PATCH_LIB = $(SDK_ROOT)/APS/targets/opl1000/Output-GCC/opl1000_app_m3.a

# Output file names
OUTPUT = opl1000_app_m3
#OUTPUT = $(shell basename $(realpath $(EXAMPLE_ROOT))

# Compiler & Linker
CROSS_COMPILE = arm-none-eabi-
CC      = $(CROSS_COMPILE)gcc
OBJDUMP = $(CROSS_COMPILE)objdump
OBJCOPY = $(CROSS_COMPILE)objcopy

#############  CFLAGS   ##############

# Options for specific architecture
DEFS  = -DGCC
DEFS += -DARMCM3
DEFS += -DOPL1000_APS
DEFS += -D__noSIMULATOR__
DEFS += -D__BLE__
DEFS += -D__LE_HOST_USE_CMD__
DEFS += -D__LWIP_TASK__
DEFS += -D__WPA_SUPPLICANT__
DEFS += -D__noHW_CRYPTO_ENGINE__
DEFS += -D__WIFI_MAC_TASK__
DEFS += -D__NL1000_An__
DEFS += -D__PMP_ENABLE__
DEFS += -D__PMP_REGION__
DEFS += -D__HEAP_REGION__
DEFS += -DnoLWIP_DYNAMIC_DEBUG_ENABLE
DEFS += -D__AT_CMD_TASK__
DEFS += -D__noRTL_SIMULATION__
DEFS += -D_ARMABI_PURE= 
#####  QUICKDEV DEFS   #####
DEFS += -DOPL1000_A3

CFLAGS  = $(DEFS)
CFLAGS += -mthumb
CFLAGS += -mcpu=cortex-m3
CFLAGS += -mlittle-endian
CFLAGS += -std=c99
CFLAGS += -Os
CFLAGS += -x none
CFLAGS += -include sys_common.h
CFLAGS += -nostartfiles
CFLAGS += -no-pie
CFLAGS += -fdata-sections
CFLAGS += -ffunction-sections
CFLAGS += -specs=nosys.specs
CFLAGS += -Xlinker "--just-symbols=$(SDK_ROOT)/APS/targets/opl1000/object/symdefs_gcc.o"
CFLAGS += -Wno-implicit-function-declaration
CFLAGS += -Wno-int-conversion

LFLAGS = -T $(LD_SCRIPT)
LFLAGS += -Wl,--gc-sections
LFLAGS += -Wl,--entry=__Patch_EntryPoint
LFLAGS += -Wl,-Map=$(OUTPUT_FOLDER)/$(OUTPUT).map

#############   include    ##############
INCDIR  = -I $(SDK_ROOT)/APS/apps/le_app/cmd_app
INCDIR += -I $(SDK_ROOT)/APS/apps/le_app/mtc_app
INCDIR += -I $(SDK_ROOT)/APS/apps/le_app/pts_app
INCDIR += -I $(SDK_ROOT)/APS/apps/wifi_app
INCDIR += -I $(SDK_ROOT)/APS/driver/chip/opl1000/hal_auxadc
INCDIR += -I $(SDK_ROOT)/APS/driver/chip/opl1000/hal_dbg_uart
INCDIR += -I $(SDK_ROOT)/APS/driver/chip/opl1000/hal_dma
INCDIR += -I $(SDK_ROOT)/APS/driver/chip/opl1000/hal_i2c
INCDIR += -I $(SDK_ROOT)/APS/driver/chip/opl1000/hal_patch
INCDIR += -I $(SDK_ROOT)/APS/driver/chip/opl1000/hal_pwm
INCDIR += -I $(SDK_ROOT)/APS/driver/chip/opl1000/hal_spi
INCDIR += -I $(SDK_ROOT)/APS/driver/chip/opl1000/hal_system
INCDIR += -I $(SDK_ROOT)/APS/driver/chip/opl1000/hal_tick
INCDIR += -I $(SDK_ROOT)/APS/driver/chip/opl1000/hal_tmr
INCDIR += -I $(SDK_ROOT)/APS/driver/chip/opl1000/hal_uart
INCDIR += -I $(SDK_ROOT)/APS/driver/chip/opl1000/hal_vic
INCDIR += -I $(SDK_ROOT)/APS/driver/chip/opl1000/hal_wdt
INCDIR += -I $(SDK_ROOT)/APS/driver/chip/opl1000/securityipdriver
INCDIR += -I $(SDK_ROOT)/APS/driver/CMSIS/Device/opl1000/Include
INCDIR += -I $(SDK_ROOT)/APS/driver/CMSIS/Include
INCDIR += -I $(SDK_ROOT)/APS/FreeRtos/Source/include
INCDIR += -I $(SDK_ROOT)/APS/FreeRtos/Source/portable/GCC/ARM_CM3
INCDIR += -I $(SDK_ROOT)/APS/middleware/netlink/at
INCDIR += -I $(SDK_ROOT)/APS/middleware/netlink/ble_controller_layer/inc
INCDIR += -I $(SDK_ROOT)/APS/middleware/netlink/cli
INCDIR += -I $(SDK_ROOT)/APS/middleware/netlink/common/sys_api
INCDIR += -I $(SDK_ROOT)/APS/middleware/netlink/common/sys_api/rom_if
INCDIR += -I $(SDK_ROOT)/APS/middleware/netlink/common/sys_ctrl
INCDIR += -I $(SDK_ROOT)/APS/middleware/netlink/controller_task
INCDIR += -I $(SDK_ROOT)/APS/middleware/netlink/data_flow
INCDIR += -I $(SDK_ROOT)/APS/middleware/netlink/diag_task
INCDIR += -I $(SDK_ROOT)/APS/middleware/netlink/le_stack
INCDIR += -I $(SDK_ROOT)/APS/middleware/netlink/msg
INCDIR += -I $(SDK_ROOT)/APS/middleware/netlink/mw_fim
INCDIR += -I $(SDK_ROOT)/APS/middleware/netlink/mw_ota
INCDIR += -I $(SDK_ROOT)/APS/middleware/netlink/ps_task
INCDIR += -I $(SDK_ROOT)/APS/middleware/netlink/wifi_controller_layer
INCDIR += -I $(SDK_ROOT)/APS/middleware/netlink/wifi_controller_layer/rom_if
INCDIR += -I $(SDK_ROOT)/APS/middleware/netlink/wifi_mac
INCDIR += -I $(SDK_ROOT)/APS/middleware/netlink/wifi_mac/common
INCDIR += -I $(SDK_ROOT)/APS/middleware/netlink/wifi_mac/crypto
#INCDIR += -I $(SDK_ROOT)/APS/middleware/netlink/wifi_mac/utils
INCDIR += -I $(SDK_ROOT)/APS/middleware/third_party/fatfs/src
INCDIR += -I $(SDK_ROOT)/APS/middleware/third_party/httpclient
INCDIR += -I $(SDK_ROOT)/APS/middleware/third_party/lwip-2.0.3
INCDIR += -I $(SDK_ROOT)/APS/middleware/third_party/lwip-2.0.3/lwip/src/include
INCDIR += -I $(SDK_ROOT)/APS/middleware/third_party/lwip-2.0.3/lwip/src/include/lwip
INCDIR += -I $(SDK_ROOT)/APS/middleware/third_party/lwip-2.0.3/lwip/src/include/lwip/apps
INCDIR += -I $(SDK_ROOT)/APS/middleware/third_party/lwip-2.0.3/ports/freertos/include
INCDIR += -I $(SDK_ROOT)/APS/middleware/third_party/lwip-2.0.3/ports/freertos/netif
INCDIR += -I $(SDK_ROOT)/APS/middleware/third_party/lwip-2.0.3/ports/rom_if
INCDIR += -I $(SDK_ROOT)/APS/middleware/third_party/mbedtls/configs
INCDIR += -I $(SDK_ROOT)/APS/middleware/third_party/mbedtls/include
INCDIR += -I $(SDK_ROOT)/APS/middleware/third_party/mbedtls/include/mbedtls
INCDIR += -I $(SDK_ROOT)/APS/middleware/third_party/mbedtls/port/include
INCDIR += -I $(SDK_ROOT)/APS/middleware/third_party/tinycrypt/include
INCDIR += -I $(SDK_ROOT)/APS/middleware/third_party/tinycrypt/include/tinycrypt
INCDIR += -I $(SDK_ROOT)/APS/middleware/third_party/wpa_supplicant-0.7.3/src/ap
INCDIR += -I $(SDK_ROOT)/APS/middleware/third_party/wpa_supplicant-0.7.3/src/common
INCDIR += -I $(SDK_ROOT)/APS/middleware/third_party/wpa_supplicant-0.7.3/src/crypto
INCDIR += -I $(SDK_ROOT)/APS/middleware/third_party/wpa_supplicant-0.7.3/src/drivers
INCDIR += -I $(SDK_ROOT)/APS/middleware/third_party/wpa_supplicant-0.7.3/src/eapol_auth
INCDIR += -I $(SDK_ROOT)/APS/middleware/third_party/wpa_supplicant-0.7.3/src/eapol_supp
INCDIR += -I $(SDK_ROOT)/APS/middleware/third_party/wpa_supplicant-0.7.3/src/eap_common
INCDIR += -I $(SDK_ROOT)/APS/middleware/third_party/wpa_supplicant-0.7.3/src/eap_peer
INCDIR += -I $(SDK_ROOT)/APS/middleware/third_party/wpa_supplicant-0.7.3/src/eap_server
INCDIR += -I $(SDK_ROOT)/APS/middleware/third_party/wpa_supplicant-0.7.3/src/l2_packet
INCDIR += -I $(SDK_ROOT)/APS/middleware/third_party/wpa_supplicant-0.7.3/src/radius
INCDIR += -I $(SDK_ROOT)/APS/middleware/third_party/wpa_supplicant-0.7.3/src/rsn_supp
INCDIR += -I $(SDK_ROOT)/APS/middleware/third_party/wpa_supplicant-0.7.3/src/tls
INCDIR += -I $(SDK_ROOT)/APS/middleware/third_party/wpa_supplicant-0.7.3/src/utils
INCDIR += -I $(SDK_ROOT)/APS/middleware/third_party/wpa_supplicant-0.7.3/src/wps
INCDIR += -I $(SDK_ROOT)/APS/middleware/third_party/wpa_supplicant-0.7.3/wpa_supplicant
INCDIR += -I $(SDK_ROOT)/APS/middleware/third_party/wpa_supplicant-0.7.3/wpa_supplicant/dbus
INCDIR += -I $(SDK_ROOT)/APS/project/common
INCDIR += -I $(SDK_ROOT)/APS/project/opl1000/boot_sequence
INCDIR += -I $(SDK_ROOT)/APS/project/opl1000/include
INCDIR += -I $(SDK_ROOT)/APS/project/opl1000/startup
INCDIR += -I $(SDK_ROOT)/APS_PATCH/apps/le_app/include
INCDIR += -I $(SDK_ROOT)/APS_PATCH/apps/le_app/cmd_app
INCDIR += -I $(SDK_ROOT)/APS_PATCH/apps/le_app/mtc_app
INCDIR += -I $(SDK_ROOT)/APS_PATCH/apps/le_app/pts_app
INCDIR += -I $(SDK_ROOT)/APS_PATCH/apps/wifi_app
INCDIR += -I $(SDK_ROOT)/APS_PATCH/driver/chip/opl1000/hal_auxadc
INCDIR += -I $(SDK_ROOT)/APS_PATCH/driver/chip/opl1000/hal_dbg_uart
INCDIR += -I $(SDK_ROOT)/APS_PATCH/driver/chip/opl1000/hal_dma
INCDIR += -I $(SDK_ROOT)/APS_PATCH/driver/chip/opl1000/hal_i2c
INCDIR += -I $(SDK_ROOT)/APS_PATCH/driver/chip/opl1000/hal_patch
INCDIR += -I $(SDK_ROOT)/APS_PATCH/driver/chip/opl1000/hal_pwm
INCDIR += -I $(SDK_ROOT)/APS_PATCH/driver/chip/opl1000/hal_spi
INCDIR += -I $(SDK_ROOT)/APS_PATCH/driver/chip/opl1000/hal_system
INCDIR += -I $(SDK_ROOT)/APS_PATCH/driver/chip/opl1000/hal_tick
INCDIR += -I $(SDK_ROOT)/APS_PATCH/driver/chip/opl1000/hal_tmr
INCDIR += -I $(SDK_ROOT)/APS_PATCH/driver/chip/opl1000/hal_uart
INCDIR += -I $(SDK_ROOT)/APS_PATCH/driver/chip/opl1000/hal_vic
INCDIR += -I $(SDK_ROOT)/APS_PATCH/driver/chip/opl1000/hal_wdt
INCDIR += -I $(SDK_ROOT)/APS_PATCH/driver/chip/opl1000/securityipdriver
INCDIR += -I $(SDK_ROOT)/APS_PATCH/driver/CMSIS/Device/opl1000/Include
INCDIR += -I $(SDK_ROOT)/APS_PATCH/driver/CMSIS/Include
INCDIR += -I $(SDK_ROOT)/APS_PATCH/FreeRtos/Source/include
INCDIR += -I $(SDK_ROOT)/APS_PATCH/FreeRtos/Source/portable/Keil/ARM_CM3
INCDIR += -I $(SDK_ROOT)/APS_PATCH/middleware/netlink/at
INCDIR += -I $(SDK_ROOT)/APS_PATCH/middleware/netlink/ble_controller_layer/inc
INCDIR += -I $(SDK_ROOT)/APS_PATCH/middleware/netlink/cli
INCDIR += -I $(SDK_ROOT)/APS_PATCH/middleware/netlink/common/sys_api
INCDIR += -I $(SDK_ROOT)/APS_PATCH/middleware/netlink/common/sys_api/rom_if
INCDIR += -I $(SDK_ROOT)/APS_PATCH/middleware/netlink/common/sys_ctrl
INCDIR += -I $(SDK_ROOT)/APS_PATCH/middleware/netlink/controller_task
INCDIR += -I $(SDK_ROOT)/APS_PATCH/middleware/netlink/data_flow
INCDIR += -I $(SDK_ROOT)/APS_PATCH/middleware/netlink/diag_task
INCDIR += -I $(SDK_ROOT)/APS_PATCH/middleware/netlink/le_stack
INCDIR += -I $(SDK_ROOT)/APS_PATCH/middleware/netlink/le_stack/patch
INCDIR += -I $(SDK_ROOT)/APS_PATCH/middleware/netlink/msg
INCDIR += -I $(SDK_ROOT)/APS_PATCH/middleware/netlink/mw_fim
INCDIR += -I $(SDK_ROOT)/APS_PATCH/middleware/netlink/mw_ota
INCDIR += -I $(SDK_ROOT)/APS_PATCH/middleware/netlink/ps_task
INCDIR += -I $(SDK_ROOT)/APS_PATCH/middleware/netlink/wifi_controller_layer
INCDIR += -I $(SDK_ROOT)/APS_PATCH/middleware/netlink/wifi_controller_layer/rom_if
INCDIR += -I $(SDK_ROOT)/APS_PATCH/middleware/netlink/wifi_mac
INCDIR += -I $(SDK_ROOT)/APS_PATCH/middleware/netlink/wifi_mac/common
INCDIR += -I $(SDK_ROOT)/APS_PATCH/middleware/netlink/wifi_mac/crypto
#INCDIR += -I $(SDK_ROOT)/APS_PATCH/middleware/netlink/wifi_mac/utils
INCDIR += -I $(SDK_ROOT)/APS_PATCH/middleware/third_party/fatfs/src
INCDIR += -I $(SDK_ROOT)/APS_PATCH/middleware/third_party/httpclient
INCDIR += -I $(SDK_ROOT)/APS_PATCH/middleware/third_party/lwip-2.0.3
INCDIR += -I $(SDK_ROOT)/APS_PATCH/middleware/third_party/lwip-2.0.3/lwip/src/include
INCDIR += -I $(SDK_ROOT)/APS_PATCH/middleware/third_party/lwip-2.0.3/lwip/src/include/lwip/apps
INCDIR += -I $(SDK_ROOT)/APS_PATCH/middleware/third_party/lwip-2.0.3/ports/freertos/include
INCDIR += -I $(SDK_ROOT)/APS_PATCH/middleware/third_party/lwip-2.0.3/ports/freertos/netif
INCDIR += -I $(SDK_ROOT)/APS_PATCH/middleware/third_party/lwip-2.0.3/ports/rom_if
INCDIR += -I $(SDK_ROOT)/APS_PATCH/middleware/third_party/mbedtls/configs
INCDIR += -I $(SDK_ROOT)/APS_PATCH/middleware/third_party/mbedtls/include
INCDIR += -I $(SDK_ROOT)/APS_PATCH/middleware/third_party/mbedtls/include/mbedtls
INCDIR += -I $(SDK_ROOT)/APS_PATCH/middleware/third_party/mbedtls/port/include
INCDIR += -I $(SDK_ROOT)/APS_PATCH/middleware/third_party/tinycrypt/include/tinycrypt
INCDIR += -I $(SDK_ROOT)/APS_PATCH/middleware/third_party/wpa_supplicant-0.7.3/src/ap
INCDIR += -I $(SDK_ROOT)/APS_PATCH/middleware/third_party/wpa_supplicant-0.7.3/src/common
INCDIR += -I $(SDK_ROOT)/APS_PATCH/middleware/third_party/wpa_supplicant-0.7.3/src/crypto
INCDIR += -I $(SDK_ROOT)/APS_PATCH/middleware/third_party/wpa_supplicant-0.7.3/src/drivers
INCDIR += -I $(SDK_ROOT)/APS_PATCH/middleware/third_party/wpa_supplicant-0.7.3/src/eapol_auth
INCDIR += -I $(SDK_ROOT)/APS_PATCH/middleware/third_party/wpa_supplicant-0.7.3/src/eapol_supp
INCDIR += -I $(SDK_ROOT)/APS_PATCH/middleware/third_party/wpa_supplicant-0.7.3/src/eap_common
INCDIR += -I $(SDK_ROOT)/APS_PATCH/middleware/third_party/wpa_supplicant-0.7.3/src/eap_peer
INCDIR += -I $(SDK_ROOT)/APS_PATCH/middleware/third_party/wpa_supplicant-0.7.3/src/eap_server
INCDIR += -I $(SDK_ROOT)/APS_PATCH/middleware/third_party/wpa_supplicant-0.7.3/src/l2_packet
INCDIR += -I $(SDK_ROOT)/APS_PATCH/middleware/third_party/wpa_supplicant-0.7.3/src/radius
INCDIR += -I $(SDK_ROOT)/APS_PATCH/middleware/third_party/wpa_supplicant-0.7.3/src/rsn_supp
INCDIR += -I $(SDK_ROOT)/APS_PATCH/middleware/third_party/wpa_supplicant-0.7.3/src/tls
INCDIR += -I $(SDK_ROOT)/APS_PATCH/middleware/third_party/wpa_supplicant-0.7.3/src/utils
INCDIR += -I $(SDK_ROOT)/APS_PATCH/middleware/third_party/wpa_supplicant-0.7.3/src/wps
INCDIR += -I $(SDK_ROOT)/APS_PATCH/middleware/third_party/wpa_supplicant-0.7.3/wpa_supplicant
INCDIR += -I $(SDK_ROOT)/APS_PATCH/middleware/third_party/wpa_supplicant-0.7.3/wpa_supplicant/dbus
INCDIR += -I $(SDK_ROOT)/APS_PATCH/project/common
INCDIR += -I $(SDK_ROOT)/APS_PATCH/project/opl1000/boot_sequence
INCDIR += -I $(SDK_ROOT)/APS_PATCH/project/opl1000/include
INCDIR += -I $(SDK_ROOT)/APS_PATCH/project/opl1000/startup
INCDIR += -I $(SDK_ROOT)/APS_PATCH/project/opl1000/lib
##### QUICKDEV include path #####
INCDIR += -I $(QD_ROOT)
INCDIR += -I $(QD_ROOT)/app_ref/ble_data_prot/opl_data
INCDIR += -I $(QD_ROOT)/app_ref/ble_services
INCDIR += -I $(QD_ROOT)/app_ref/cloud
INCDIR += -I $(QD_ROOT)/app_ref/mngr_api/ble_mngr_api
INCDIR += -I $(QD_ROOT)/app_ref/mngr_api/net_mngr_api
INCDIR += -I $(QD_ROOT)/app_ref/mngr_api/wifi_mngr_api
INCDIR += -I $(QD_ROOT)/common/battery
INCDIR += -I $(QD_ROOT)/common/config/ps_config
INCDIR += -I $(QD_ROOT)/common/error
INCDIR += -I $(QD_ROOT)/common/evt_group
INCDIR += -I $(QD_ROOT)/common/fsm
INCDIR += -I $(QD_ROOT)/common/httpclient_exp
INCDIR += -I $(QD_ROOT)/common/log
INCDIR += -I $(QD_ROOT)/common/mp
INCDIR += -I $(QD_ROOT)/common/ota
INCDIR += -I $(QD_ROOT)/common/pwr_save
INCDIR += -I $(QD_ROOT)/common/rf_pwr
INCDIR += -I $(QD_ROOT)/common/ring_buffer
INCDIR += -I $(QD_ROOT)/common/time_stmp
INCDIR += -I $(QD_ROOT)/middleware/ble_mngr
INCDIR += -I $(QD_ROOT)/middleware/net_mngr
INCDIR += -I $(QD_ROOT)/middleware/wifi_mngr
INCDIR += -I $(QD_ROOT)/middleware/wifi_mngr/wifi_autocon
INCDIR += -I $(QD_ROOT)/middleware/wifi_mngr/wifi_agent
INCDIR += -I $(QD_ROOT)/middleware/wifi_mngr/wifi_pro_rec
INCDIR += -I $(EXAMPLE_ROOT)/main
INCDIR += -I $(EXAMPLE_ROOT)/main/hal
INCDIR += -I $(EXAMPLE_ROOT)/main/sys_config
INCDIR += -I $(PROJECT_ROOT)/hal
INCDIR += -I $(PROJECT_ROOT)/board

#############   source    ##############
M_SRC  = $(PROJECT_ROOT)/hal/main_patch.c
M_SRC += $(EXAMPLE_ROOT)/main/app_main.c
M_SRC += $(EXAMPLE_ROOT)/main/app_at_cmd.c
M_SRC += $(QD_ROOT)/middleware/wifi_mngr/wifi_pro_rec/mw_fim_default_group11_project.c
M_SRC += $(QD_ROOT)/common/mp/mw_fim_default_group12_project.c
M_SRC += $(QD_ROOT)/common/battery/mw_fim_default_group16_project.c
M_SRC += $(QD_ROOT)/middleware/net_mngr/net_mngr.c
M_SRC += $(QD_ROOT)/middleware/net_mngr/net_mngr_api.c
M_SRC += $(QD_ROOT)/middleware/net_mngr/net_mngr_at.c
M_SRC += $(QD_ROOT)/middleware/wifi_mngr/wifi_mngr.c
M_SRC += $(QD_ROOT)/middleware/wifi_mngr/wifi_mngr_api.c
M_SRC += $(QD_ROOT)/middleware/wifi_mngr/wifi_mngr_at.c
M_SRC += $(QD_ROOT)/middleware/wifi_mngr/wifi_agent/wifi_agent.c
M_SRC += $(QD_ROOT)/middleware/wifi_mngr/wifi_agent/wifi_agent_api.c
M_SRC += $(QD_ROOT)/middleware/wifi_mngr/wifi_autocon/wifi_autocon.c
M_SRC += $(QD_ROOT)/middleware/wifi_mngr/wifi_autocon/wifi_autocon_at.c
M_SRC += $(QD_ROOT)/middleware/wifi_mngr/wifi_pro_rec/wifi_pro_rec.c
M_SRC += $(QD_ROOT)/middleware/ble_mngr/ble_mngr.c
M_SRC += $(QD_ROOT)/middleware/ble_mngr/ble_mngr_api.c
M_SRC += $(QD_ROOT)/middleware/ble_mngr/ble_mngr_at.c
M_SRC += $(QD_ROOT)/app_ref/ble_services/gap_svc.c
M_SRC += $(QD_ROOT)/app_ref/ble_services/gatt_svc.c
M_SRC += $(QD_ROOT)/common/battery/bat_aux.c
M_SRC += $(QD_ROOT)/common/evt_group/evt_group.c
M_SRC += $(QD_ROOT)/common/fsm/fsm_kernel.c
M_SRC += $(QD_ROOT)/common/log/log.c
M_SRC += $(QD_ROOT)/common/ota/ota_mngr.c
M_SRC += $(QD_ROOT)/common/pwr_save/pwr_save.c
M_SRC += $(QD_ROOT)/common/rf_pwr/rf_pwr.c
M_SRC += $(QD_ROOT)/app_ref/ble_data_prot/opl_data/opl_data_hdl.c
M_SRC += $(QD_ROOT)/app_ref/ble_data_prot/opl_data/opl_data_prot.c
M_SRC += $(QD_ROOT)/app_ref/ble_data_prot/opl_data/opl_svc.c

#############   build   ##############
all:
	@mkdir $(OUTPUT_FOLDER)
	@$(CC) -o $(OUTPUT_FOLDER)/$(OUTPUT).elf $(LFLAGS) $(CFLAGS) $(INCDIR) $(M_SRC) $(PATCH_LIB)
	$(OBJCOPY) -O binary $(OUTPUT_FOLDER)/$(OUTPUT).elf $(OUTPUT_FOLDER)/$(OUTPUT).bin

#############   clean   ##############  
clean:
	@rm -rf $(OUTPUT_FOLDER)