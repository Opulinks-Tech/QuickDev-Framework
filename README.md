# QuickDev-Framework
- QuickDev framework aims to Opulinks SoC family (OPL1000 A3, OPL2500 A0, and after).
- Provide an easy entry to develop IoT application.
- Provide application reference modules consist of complete IoT application functions.
- Provide enhanced Opulinks SDK middleware.
- Modelized architecture, fulfills different vendor requirement.
- Provide easy configuration mechanism to config different kind of applications and different modules.
- Provide cloud template to adapt public/private cloud.
- Provide piratical usage examples to facilitate developing.

# Folder introduction  
- “examples”: provide groups of examples.
  - “ble”: The BLE relative examples.
    - "ble_svc_battery": demonstrate how to create a battery service to notify battery status to host periodically.
    - "ble_svc_data_io": demonstrate how to create a data I/O service between device and host. Vendor can build up their propriety BLE data protocol tunnelling this I/O service
  - “cloud”: examples that demonstrate how to adapt cloud template of QuickDev framework.
    - “tcp_demo”:  demonstrate a simple TCP communicate between server and device, and how to adapt the QuickDev cloud template to TCP server.
    - “mqtt_demo”: demonstrate a simple MQTT communicate with broker, and how to adapt the QuickDev cloud template to MQTT broker.
  - “start_up”: entry examples for QuickDev framework.
    - “qd_app”: This is a quick-start example for developer running QuickDev framework on Opulink Devkit. The “qd_app” (applying the Opulinks Utility App on iOS or Android), demonstrates the functionalities such as BLE/WiFi provision ad BLE OTA.
  - “wifi”: The Wi-Fi relative examples.
    - “wifi_scan_connect”: example that demonstrate a simple WiFi scan and connect request by using net_mngr to controlling Wi-Fi behavior.
    - “wifi_quick_connect”: example that demonstrate a quick way to set a WiFi AP profile into QuickDev framework and clear previous ones by using network manager.
- “quick_dev”: The QuickDev framework source code.
  - "app_ref": Provides reference modules for a complete product demonstration, the application developers can reference the source code and decide which modules to apply for fast product development.
    - “ble_services”: the GATT BLE service files, include GAP, GATT, and user-defined service. The example “ble_svc_battery” applies “bas_svc.c”, and the example “ble_svc_data_io” applies “ud_svc.c”.
    - "ble_data_prot": The Oplulink BLE data protocol (TBD.)
    - "cloud": provide IoT cloud template framework. Vender can apply this framework to adapt the private/public cloud.
    - "mngr_api": the API header files of the communication control manager, such as, network manager, WiFi manager, and BLE manager
  - "middleware": The middleware layer on top of Tritium SDK, to integrate common useful functions and features to minimize the effort for application developer for any applications.
    - "ble_mngr": source code of BLE manager, to provides reference of slave and master mode and GATT service creating
    - "net_mngr": source code of network manager, management of the network connections. That is, the WiFi connection, the IP (DHCP) retrieving, and the cloud connection activation.
    - "wifi_mngr": source code of WiFi manager, implemented commonly used WiFi related features, such as WiFi scan/connect, and Auto-connect sub-module.
- “sdk”: Opulinks SDK folder, please download SDK from Opulinks Github and put into this folder. Currently QuickDev framework supports Oplulinks OPL1000 A3 and OPL2500 A0 SoC.
  - OPL1000_A3: https://github.com/Opulinks-Tech/OPL1000A3-SDK
  - OPL2500_A0: TBD.

# User Guide
  - Please refer to: https://github.com/harrison-netlink/MyTest/wiki
