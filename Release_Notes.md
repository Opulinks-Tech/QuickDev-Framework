## Release Version:
State : RC
Version : v0.2.7

## Release Date: 2023/05/12

## Descript
1. Support OPL2500_A0 host_mode examples (at_master/at_slave/uart_master/uart_slave)
2. Support OPL2500_A0 peripheral/i2c_master example
3. Modify tcp_cloud
    - add power sleep control via TCP data, this function is defined by "OPL_POWER_SLEEP_CONTROL_VIA_TCP" with default disable in opl_config.h
    - fix twice TCP disconnect issue
    - modify the set_dtim execute timing to adjust the stability to send and receive data
4. Modify OPL2500_A0 example's scatter file to support OPL2500 new SDK version
5. Modify OPL1000_A3 heap start address default to 0x0043A000
6. Modify net_mngr to handle the wifi reset & disconnect event from wifi_mngr, and send notify to application
7. Modify WM_SkipDtimSet function
    - adjust the insert arguments
    - to set skip DTIM to 0 if the definition "WM_DTIM_PERIOD_TIME" set 0
8. Modify deep-sleep wakeup pin config set in pwr_save
9. Add "EXT_PA_ENABLE" definition to support OPL2500_P (front-end module) in qd_config.h, default set as "Disable"
10. Fix auto-connect interval table index caculation's error
11. Modify the specific C:/Keil path to open the Keil/ARMCC/ELF to using to relative Keil's path in example's batch file and keil project file (only modify on OPL2500 relative examples)

## Release Version:
State : RC
Version : v0.2.5

## Release Date: 2023/01/05

## Descript
1. Add aws_cloud in app_ref/cloud
2. Add devinfo_svc in app_ref/ble_services
3. Add check ota progress api in http_ota file
4. Modify the definition BM_DEF_DESIRED_MIN_CONN_INTERNAL & BM_DEF_DESIRED_MAX_CONN_INTERNAL to BM_DEF_DESIRED_MIN_ADV_INTERVAL & BM_DEF_DESIRED_MAX_ADV_INTERVAL
5. Modify the device name characteristic as read only in gap_svc file
6. Fix BM_AdvIntervalSet not work issue in ble_mngr file

## Release Version:
State : RC
Version : v0.2.4

## Release Date: 2022/12/26

## Descript
1. Modify pin configuration file of opl2500_a0 project in each example for fitting in newest opl2500 EVB
2. Modify Makefile that can using GNU make to compile example program (Currently only support on qd_app)
3. Add build.bat (windows) and build.sh (linux) script file for easy to compiling code via GNU make
4. Current measurement function supported on qd_app (Compiler option "OPL_DATA_CURRENT_MEASURE_ENABLED" must required, default 0)

## Release Version:
State : RC
Version : v0.2.3

## Release Date: 2022/10/31

## Descript
1. Add http_ota example located at examples/ota folder to demonstrate the OTA procedure through HTTP protocol
2. Add periodic data post after cloud connected scenario in tcp_demo and mqtt_demo examples
3. Add opl2500_a0 type project in mqtt_demo example
4. Re-organize the httpclient into httpclient_exp common module located at quick_dev/common/httpclient_exp
5. Set default fim version as 0 in each example
6. Remove default "at+ota" command in each example except http_ota example
7. Modify the ring buffer related calling function to a correct APIs

## Release Version:
State : RC
Version : v0.2.2

## Release Date: 2022/09/30

## Descript
1. Fix mqtt cloud watch-dog timeout and receive concurrency issue
2. Optimize mqtt cloud power consumption
3. Fix the memory leakage issue due to tcp cloud socket close incorrect issue
4. Fix mcu halt issue due to semaphore init timing
5. Modify ini file linker in each example

## Release Version:
State : RC
Version : v0.2.1

## Release Date: 2022/08/09

## Descript
1. Verify ble_svc_battery;opl1000_a3
2. Verify ble_svc_battery;opl1000_a3 & opl2500_a0
3. Fix battery adc issue on opl1000_a3
4. Set ble_svc_battery, ble_svc_data_io, wifi_scan_connect and wifi_quick_connect projects default power mode as performance type

## Release Version:
State : RC
Version : v0.2.0

## Release Date: 2022/08/08

## Limitaion
1. For opl2500_a0 only tcp_demo and qd_app are verified.