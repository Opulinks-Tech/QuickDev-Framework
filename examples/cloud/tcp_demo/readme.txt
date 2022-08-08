Introduction:
'tcp_demo' demonstrate a simple tcp communicate between server and device.

Testing:
1. Compiling and program the application.

2. While start up, device will try to connect to tcp server every 5s till connected.

3. Runs the tcp server by using python (python3 environment required),
   tcp server locate at QD_FWK\quick_dev\tools\tcp_server.py.

4. When server is ready, the connection should be established by device.

5. This application supported the both way transmission to send data from server or device.
	To post data from server to cloud:
		Type any string you want at server console.
		Ex. hello
		
		The server will post {"P":"hello"} in json format to device.

	To post data from device to cloud:
		Type at command with string data in at mode in device.
		Ex. at+tcpsend=hello

		The device will post {"P":"hello"} in json format to server.

6. And also during the connection, device will automate transfer keep alive data "ping" in every 1min,
   and will received the response "pong" from server.


Detail:
AT CMD support:
	1. at+tcpconnect=<host ip>,<host port>			// to connect to tcp server
	2. at+tcpdisconnect					// to disconnect the connection
	3. at+tcpkalive=<keep alive duration (ms)>		// to change the keep alive duration (must in connected)
	4. at+tcpsend=<string>					// to send data in string to server

Hard code host IP & Port:
	The default host IP and Port for device to connect are defines in macro in cloud_config.h file,
	file located in QD_FWK\quick_dev\app_ref\cloud\tcp_cloud\cloud_config.h

	#define TCP_HOST_IP
	#define TCP_HOST_PORT

	You can modify the host IP address and Port for your environment,
	and also note the server and device must in the same local network.