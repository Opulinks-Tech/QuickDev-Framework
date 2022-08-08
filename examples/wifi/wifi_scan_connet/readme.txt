Introduction:
'wifi_scan_connect' demonstrate a simple wifi scan and connect request by using net_mngr to controlling WI-FI behavior

Testing:
1. Compiling and program the application

2. While start up, device will begin to trigger scan request.

3. After received scan indicate, will try connect request with assigned AP ssid and password.

4. If connection success, device enter idle; if connection fail, device will trigger scan request again.

5. If received disconnect event during connection, device will also trigger scan request again.

Detail:
Hard code AP ssid & password:
	The default AP ssid & password for connect request are defines in macro in qd_config.h file,
	file located in examples\wifi\wifi_scan_connect\main\sys_config\qd_config.h

	#define APP_DEF_AP_SSID
	#define APP_DEF_AP_PWD

	You can modify the ssid and password for your needs.