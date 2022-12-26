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