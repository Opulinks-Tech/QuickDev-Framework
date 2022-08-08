Introduction:
'wifi_quick_connect' demonstrate a directly connect to specified AP with ssid and password by using net_mngr.

Testing:
1. Compiling and program the application

2. While start up, application will directly connect to specified AP.

Detail:
Hard code AP ssid & password:
	The default AP ssid & password for connect request are defines in macro in qd_config.h file,
	file located in examples\wifi\wifi_quick_connect\main\sys_config\qd_config.h

	#define APP_DEF_AP_SSID
	#define APP_DEF_AP_PWD

	You can modify the ssid and password for your needs.