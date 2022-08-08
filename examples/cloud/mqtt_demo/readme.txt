Introduction:
'mqtt_demo' demonstrate a simple MQTT communicate with broker.
Default application will subscribe 2 topic & publish data periodically to a publish topic.

Testing:
1. Compiling and program the application.

2. While start up, device will begin BLE advertising.

3. Using "Opulinks Utilities" APP on smart phone to search and connect to the device.

4. Enter "Setting WIFI" to connect to your AP router.

5. After connected and had IP, device will establish the MQTT connection and subscribe topics.

6. And also after the connected event, device will start a periodical timer to publish data to broker.

7. using MQTT-X as other client and connect to same broker.

8. Subscribe the topic in MQTT-X same as device published.

9. Publsih the data to topic same as device subscribed.

Detail:
Default MQTT broker URL : broker.emqx.io
Default MQTT broker port : 8883
Default device subscribed topic :
	1. QD_FWK/MQTT_DEMO/SUB_Test/1
	2. QD_FWK/MQTT_DEMO/SUB_Test/2
Default device published topic :
	1. QD_FWK/MQTT_DEMO/PUB_Test/1