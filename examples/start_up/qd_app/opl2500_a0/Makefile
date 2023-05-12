##### ARM GCC Toolchain Development #####
SDK_ROOT = ../../../../sdk/SDK
QD_ROOT = ../../../../quick_dev
EXAMPLE_ROOT = ..
PROJECT_ROOT = .

OUTPUT_FOLDER = ./Output-GCC
LD_SCRIPT = $(SDK_ROOT)/APS/project/opl2500/link.ld

# Output file names
OUTPUT = opl2500_app_aps
#OUTPUT = $(shell basename $(realpath $(EXAMPLE_ROOT))

# Compiler & Linker
CROSS_COMPILE = arm-none-eabi-
CC      = $(CROSS_COMPILE)gcc
OBJDUMP = $(CROSS_COMPILE)objdump
OBJCOPY = $(CROSS_COMPILE)objcopy
AR      = $(CROSS_COMPILE)ar


############# Ram mapping ############
APS_PATCH_START = 0x412454
APS_HEAP_START = 0x444000
APS_RAM_END = 0x454000


############# include LIB ############

PATCH_LIB = $(SDK_ROOT)/APS/targets/opl2500/Output-GCC/opl2500_patch_aps.a
SDK_LIB   = $(SDK_ROOT)/APS/targets/opl2500/Output-GCC/opl2500_sdk_aps.a
MAIN_LIB  = $(OUTPUT_FOLDER)/opl2500_app_aps.a


#############  CFLAGS   ##############

# Options for specific architecture
DEFS  = -DGCC
DEFS += -DARMCM4_FP
DEFS += -D__VFP_FP__
DEFS += -DOPL2500_APS
DEFS += -DOPL2500_PATCH
DEFS += -D__noSIMULATOR__
DEFS += -D__BLE__
DEFS += -D__LE_HOST_USE_CMD__
DEFS += -D__LWIP_TASK__
DEFS += -D__WIFI_MAC_TASK__
DEFS += -D__PMP_ENABLE__
DEFS += -D__PMP_REGION__
DEFS += -D__HEAP_REGION__
DEFS += -DnoLWIP_DYNAMIC_DEBUG_ENABLE
DEFS += -D__AT_CMD_TASK__
DEFS += -D__noRTL_SIMULATION__
DEFS += -DMBEDTLS_CONFIG_FILE="<config-opl-default.h>"
#####  QUICKDEV DEFS   #####
DEFS += -DOPL2500_A0

CFLAGS  = $(DEFS)
CFLAGS += -mthumb
CFLAGS += -mcpu=cortex-m4
CFLAGS += -mfpu=fpv4-sp-d16
CFLAGS += -mfloat-abi=hard
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
CFLAGS += -Xlinker "--just-symbols=$(SDK_ROOT)/APS/targets/opl2500/object/symdefs_gcc.o"
CFLAGS += -Wno-implicit-function-declaration
CFLAGS += -Wno-int-conversion

LFLAGS  = -T $(LD_SCRIPT) -L $(SDK_ROOT)/APS/project/opl2500/
LFLAGS += -Wl,--defsym=LD_APS_PATCH_START=$(APS_PATCH_START)
LFLAGS += -Wl,--defsym=LD_APS_HEAP_START=$(APS_HEAP_START)
LFLAGS += -Wl,--defsym=LD_APS_RAM_END=$(APS_RAM_END)
LFLAGS += -Wl,--gc-sections
LFLAGS += -Wl,--entry=__Patch_EntryPoint
LFLAGS += -Wl,--start-group $(PATCH_LIB) $(SDK_LIB) $(MAIN_LIB) 
LFLAGS += -Wl,--end-group
LFLAGS += -Wl,-Map=$(OUTPUT_FOLDER)/$(OUTPUT).map

#############   include    ##############
INCDIR  = -I $(SDK_ROOT)/APS/project/opl2500/boot_sequence
INCDIR += -I $(SDK_ROOT)/APS/project/opl2500/include
INCDIR += -I $(SDK_ROOT)/APS/project/opl2500/lib
INCDIR += -I $(SDK_ROOT)/APS/project/opl2500/startup
INCDIR += -I $(SDK_ROOT)/APS/project/common
INCDIR += -I $(SDK_ROOT)/APS/FreeRtos
INCDIR += -I $(SDK_ROOT)/APS/FreeRtos/Source/include
INCDIR += -I $(SDK_ROOT)/APS/FreeRtos/Source/portable/GCC/ARM_CM4F
INCDIR += -I $(SDK_ROOT)/APS/driver/CMSIS/Device/Opulinks/opl2500/Include
INCDIR += -I $(SDK_ROOT)/APS/driver/CMSIS/Include
INCDIR += -I $(SDK_ROOT)/APS/driver/chip
INCDIR += -I $(SDK_ROOT)/APS/driver/chip/opl2500/camera
INCDIR += -I $(SDK_ROOT)/APS/driver/chip/opl2500/hal_auxadc
INCDIR += -I $(SDK_ROOT)/APS/driver/chip/opl2500/hal_dbg_uart
INCDIR += -I $(SDK_ROOT)/APS/driver/chip/opl2500/hal_dma
INCDIR += -I $(SDK_ROOT)/APS/driver/chip/opl2500/hal_gpio
INCDIR += -I $(SDK_ROOT)/APS/driver/chip/opl2500/hal_i2c
INCDIR += -I $(SDK_ROOT)/APS/driver/chip/opl2500/hal_patch
INCDIR += -I $(SDK_ROOT)/APS/driver/chip/opl2500/hal_pdm
INCDIR += -I $(SDK_ROOT)/APS/driver/chip/opl2500/hal_pwm
INCDIR += -I $(SDK_ROOT)/APS/driver/chip/opl2500/hal_spi
INCDIR += -I $(SDK_ROOT)/APS/driver/chip/opl2500/hal_system
INCDIR += -I $(SDK_ROOT)/APS/driver/chip/opl2500/hal_tick
INCDIR += -I $(SDK_ROOT)/APS/driver/chip/opl2500/hal_tmr
INCDIR += -I $(SDK_ROOT)/APS/driver/chip/opl2500/hal_uart
INCDIR += -I $(SDK_ROOT)/APS/driver/chip/opl2500/hal_vic
INCDIR += -I $(SDK_ROOT)/APS/driver/chip/opl2500/hal_wdt
INCDIR += -I $(SDK_ROOT)/APS/driver/chip/opl2500/securityipdriver
INCDIR += -I $(SDK_ROOT)/APS/middleware/netlink
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
INCDIR += -I $(SDK_ROOT)/APS/middleware/netlink/wifi_mac
INCDIR += -I $(SDK_ROOT)/APS/middleware/netlink/wifi_mac/common
INCDIR += -I $(SDK_ROOT)/APS/middleware/third_party/httpclient
INCDIR += -I $(SDK_ROOT)/APS/middleware/third_party/lwip-2.0.3
INCDIR += -I $(SDK_ROOT)/APS/middleware/third_party/lwip-2.0.3/lwip/src/include
INCDIR += -I $(SDK_ROOT)/APS/middleware/third_party/lwip-2.0.3/lwip/src/include/lwip/
INCDIR += -I $(SDK_ROOT)/APS/middleware/third_party/lwip-2.0.3/lwip/src/include/lwip/apps
INCDIR += -I $(SDK_ROOT)/APS/middleware/third_party/lwip-2.0.3/ports/freertos/include
INCDIR += -I $(SDK_ROOT)/APS/middleware/third_party/lwip-2.0.3/ports/freertos/netif
INCDIR += -I $(SDK_ROOT)/APS/middleware/third_party/lwip-2.0.3/ports/rom_if
INCDIR += -I $(SDK_ROOT)/APS/middleware/third_party/lwip-2.0.3/contrib/apps
INCDIR += -I $(SDK_ROOT)/APS/middleware/third_party/mbedtls/configs
INCDIR += -I $(SDK_ROOT)/APS/middleware/third_party/mbedtls/include
INCDIR += -I $(SDK_ROOT)/APS/middleware/third_party/mbedtls/include/mbedtls
INCDIR += -I $(SDK_ROOT)/APS/middleware/third_party/mbedtls/port/include
INCDIR += -I $(SDK_ROOT)/APS/middleware/third_party/tinycrypt/include
INCDIR += -I $(SDK_ROOT)/APS/middleware/third_party/tinycrypt/include/tinycrypt
INCDIR += -I $(SDK_ROOT)/APS/apps/le_app/cmd_app
INCDIR += -I $(SDK_ROOT)/APS/apps/le_app/pts_app
INCDIR += -I $(SDK_ROOT)/APS_PATCH/project/opl2500/boot_sequence
INCDIR += -I $(SDK_ROOT)/APS_PATCH/project/opl2500/include
INCDIR += -I $(SDK_ROOT)/APS_PATCH/project/opl2500/lib
INCDIR += -I $(SDK_ROOT)/APS_PATCH/project/opl2500/startup
INCDIR += -I $(SDK_ROOT)/APS_PATCH/project/common
INCDIR += -I $(SDK_ROOT)/APS_PATCH/FreeRtos
INCDIR += -I $(SDK_ROOT)/APS_PATCH/FreeRtos/Source/include
INCDIR += -I $(SDK_ROOT)/APS_PATCH/FreeRtos/Source/portable/GCC/ARM_CM4F
INCDIR += -I $(SDK_ROOT)/APS_PATCH/driver/CMSIS/Device/Opulinks/opl2500/Include
INCDIR += -I $(SDK_ROOT)/APS_PATCH/driver/CMSIS/Include
INCDIR += -I $(SDK_ROOT)/APS_PATCH/driver/chip
INCDIR += -I $(SDK_ROOT)/APS_PATCH/driver/chip/opl2500/camera
INCDIR += -I $(SDK_ROOT)/APS_PATCH/driver/chip/opl2500/hal_auxadc
INCDIR += -I $(SDK_ROOT)/APS_PATCH/driver/chip/opl2500/hal_dbg_uart
INCDIR += -I $(SDK_ROOT)/APS_PATCH/driver/chip/opl2500/hal_dma
INCDIR += -I $(SDK_ROOT)/APS_PATCH/driver/chip/opl2500/hal_gpio
INCDIR += -I $(SDK_ROOT)/APS_PATCH/driver/chip/opl2500/hal_i2c
INCDIR += -I $(SDK_ROOT)/APS_PATCH/driver/chip/opl2500/hal_patch
INCDIR += -I $(SDK_ROOT)/APS_PATCH/driver/chip/opl2500/hal_pdm
INCDIR += -I $(SDK_ROOT)/APS_PATCH/driver/chip/opl2500/hal_pwm
INCDIR += -I $(SDK_ROOT)/APS_PATCH/driver/chip/opl2500/hal_spi
INCDIR += -I $(SDK_ROOT)/APS_PATCH/driver/chip/opl2500/hal_system
INCDIR += -I $(SDK_ROOT)/APS_PATCH/driver/chip/opl2500/hal_tick
INCDIR += -I $(SDK_ROOT)/APS_PATCH/driver/chip/opl2500/hal_tmr
INCDIR += -I $(SDK_ROOT)/APS_PATCH/driver/chip/opl2500/hal_uart
INCDIR += -I $(SDK_ROOT)/APS_PATCH/driver/chip/opl2500/hal_vic
INCDIR += -I $(SDK_ROOT)/APS_PATCH/driver/chip/opl2500/hal_wdt
INCDIR += -I $(SDK_ROOT)/APS_PATCH/driver/chip/opl2500/securityipdriver
INCDIR += -I $(SDK_ROOT)/APS_PATCH/middleware/netlink
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
INCDIR += -I $(SDK_ROOT)/APS_PATCH/middleware/netlink/msg
INCDIR += -I $(SDK_ROOT)/APS_PATCH/middleware/netlink/mw_fim
INCDIR += -I $(SDK_ROOT)/APS_PATCH/middleware/netlink/mw_ota
INCDIR += -I $(SDK_ROOT)/APS_PATCH/middleware/netlink/ps_task
INCDIR += -I $(SDK_ROOT)/APS_PATCH/middleware/netlink/wifi_controller_layer
INCDIR += -I $(SDK_ROOT)/APS_PATCH/middleware/netlink/wifi_mac
INCDIR += -I $(SDK_ROOT)/APS_PATCH/middleware/netlink/wifi_mac/common
INCDIR += -I $(SDK_ROOT)/APS_PATCH/middleware/third_party/httpclient
INCDIR += -I $(SDK_ROOT)/APS_PATCH/middleware/third_party/lwip-2.0.3
INCDIR += -I $(SDK_ROOT)/APS_PATCH/middleware/third_party/lwip-2.0.3/lwip/src/include
INCDIR += -I $(SDK_ROOT)/APS_PATCH/middleware/third_party/lwip-2.0.3/lwip/src/include/lwip/apps
INCDIR += -I $(SDK_ROOT)/APS_PATCH/middleware/third_party/lwip-2.0.3/ports/freertos/include
INCDIR += -I $(SDK_ROOT)/APS_PATCH/middleware/third_party/lwip-2.0.3/ports/freertos/netif
INCDIR += -I $(SDK_ROOT)/APS_PATCH/middleware/third_party/lwip-2.0.3/ports/rom_if
INCDIR += -I $(SDK_ROOT)/APS_PATCH/middleware/third_party/lwip-2.0.3/contrib/apps
INCDIR += -I $(SDK_ROOT)/APS_PATCH/middleware/third_party/mbedtls/configs
INCDIR += -I $(SDK_ROOT)/APS_PATCH/middleware/third_party/mbedtls/include
INCDIR += -I $(SDK_ROOT)/APS_PATCH/middleware/third_party/mbedtls/include/mbedtls
INCDIR += -I $(SDK_ROOT)/APS_PATCH/middleware/third_party/mbedtls/port/include
INCDIR += -I $(SDK_ROOT)/APS_PATCH/middleware/third_party/tinycrypt/include
INCDIR += -I $(SDK_ROOT)/APS_PATCH/middleware/third_party/tinycrypt/include/tinycrypt
INCDIR += -I $(SDK_ROOT)/APS_PATCH/apps/le_app/cmd_app
INCDIR += -I $(SDK_ROOT)/APS_PATCH/apps/le_app/include
INCDIR += -I $(SDK_ROOT)/APS_PATCH/apps/le_app/pts_app
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
#M_SRC = ./main_patch.o
M_SRC  = $(PROJECT_ROOT)/hal/main_patch.o
M_SRC += $(EXAMPLE_ROOT)/main/app_main.o
M_SRC += $(EXAMPLE_ROOT)/main/app_at_cmd.o
M_SRC += $(QD_ROOT)/middleware/wifi_mngr/wifi_pro_rec/mw_fim_default_group11_project.o
M_SRC += $(QD_ROOT)/common/mp/mw_fim_default_group12_project.o
M_SRC += $(QD_ROOT)/common/battery/mw_fim_default_group16_project.o
M_SRC += $(QD_ROOT)/middleware/net_mngr/net_mngr.o
M_SRC += $(QD_ROOT)/middleware/net_mngr/net_mngr_api.o
M_SRC += $(QD_ROOT)/middleware/net_mngr/net_mngr_at.o
M_SRC += $(QD_ROOT)/middleware/wifi_mngr/wifi_mngr.o
M_SRC += $(QD_ROOT)/middleware/wifi_mngr/wifi_mngr_api.o
M_SRC += $(QD_ROOT)/middleware/wifi_mngr/wifi_mngr_at.o
M_SRC += $(QD_ROOT)/middleware/wifi_mngr/wifi_agent/wifi_agent.o
M_SRC += $(QD_ROOT)/middleware/wifi_mngr/wifi_agent/wifi_agent_api.o
M_SRC += $(QD_ROOT)/middleware/wifi_mngr/wifi_autocon/wifi_autocon.o
M_SRC += $(QD_ROOT)/middleware/wifi_mngr/wifi_autocon/wifi_autocon_at.o
M_SRC += $(QD_ROOT)/middleware/wifi_mngr/wifi_pro_rec/wifi_pro_rec.o
M_SRC += $(QD_ROOT)/middleware/ble_mngr/ble_mngr.o
M_SRC += $(QD_ROOT)/middleware/ble_mngr/ble_mngr_api.o
M_SRC += $(QD_ROOT)/middleware/ble_mngr/ble_mngr_at.o
M_SRC += $(QD_ROOT)/app_ref/ble_services/gap_svc.o
M_SRC += $(QD_ROOT)/app_ref/ble_services/gatt_svc.o
M_SRC += $(QD_ROOT)/common/battery/bat_aux.o
M_SRC += $(QD_ROOT)/common/evt_group/evt_group.o
M_SRC += $(QD_ROOT)/common/fsm/fsm_kernel.o
M_SRC += $(QD_ROOT)/common/log/log.o
M_SRC += $(QD_ROOT)/common/ota/ota_mngr.o
M_SRC += $(QD_ROOT)/common/pwr_save/pwr_save.o
M_SRC += $(QD_ROOT)/common/rf_pwr/rf_pwr.o
M_SRC += $(QD_ROOT)/app_ref/ble_data_prot/opl_data/opl_data_hdl.o
M_SRC += $(QD_ROOT)/app_ref/ble_data_prot/opl_data/opl_data_prot.o
M_SRC += $(QD_ROOT)/app_ref/ble_data_prot/opl_data/opl_svc.o

#############   build   ##############
all: $(M_SRC)
	@mkdir -p $(OUTPUT_FOLDER)
	@rm -f $(OUTPUT_FOLDER)/$(MAIN_LIB)
	@$(AR) -r $(MAIN_LIB) $(M_SRC)
	@$(CC) -o $(OUTPUT_FOLDER)/$(OUTPUT).elf $(LFLAGS) $(CFLAGS) $(INCDIR)
	@$(OBJCOPY) -O binary --only-section=.aps_* $(OUTPUT_FOLDER)/$(OUTPUT).elf $(OUTPUT_FOLDER)/opl2500_ram1_EntryPoint.bin
	@echo Generate opl2500_ram1_EntryPoint.bin
	@$(OBJCOPY) -O binary --only-section=.shm_* $(OUTPUT_FOLDER)/$(OUTPUT).elf $(OUTPUT_FOLDER)/opl2500_ram2_ShmRegion.bin
	@echo Generate opl2500_ram2_ShmRegion.bin
	@$(OBJCOPY) -O binary --only-section=.part1_* $(OUTPUT_FOLDER)/$(OUTPUT).elf $(OUTPUT_FOLDER)/opl2500_ram3_PartMem.bin
	@echo Generate opl2500_ram3_PartMem.bin
	@$(OBJCOPY) -O binary --only-section=.cache_* $(OUTPUT_FOLDER)/$(OUTPUT).elf $(OUTPUT_FOLDER)/opl2500_ram4_CacheRam.bin
	@echo Generate opl2500_ram4_CacheRam.bin

%.o: %.c
	@$(CC) -c $(CFLAGS) $(INCDIR) $^ -o $@
	@echo Build $@

#############   clean   ##############  
clean:
	@rm -f $(M_SRC)
	@rm -rf $(OUTPUT_FOLDER)
