#include <Arduino.h>
#include <AsyncTCP.h>
#include <DNSServer.h>
#include <WiFi.h>

#ifndef CONFIG_H
#define CONFIG_H

#define SSID "esp-tcp-server"
#define PASSWORD "123456789"

#define SERVER_HOST_NAME "esp-tcp-server"

#define TCP_SERVER_PORT 8000
#define DNS_PORT 53

#endif // CONFIG_H

static DNSServer DNS;

static void handleData(void *arg, AsyncClient *client, void *data, size_t len)
{
	Serial.printf("\n data received from client %s \n", client->remoteIP().toString().c_str());
	Serial.write((uint8_t *)data, len);

	//our big json string test
	String jsonString = "{\"data_from_module_type\":5,\"hub_unique_id\":\"hub-bfd\",\"slave_unique_id\":\"\",\"water_sensor\":{\"unique_id\":\"water-sensor-ba9\",\"firmware\":\"0.0.1\",\"hub_unique_id\":\"hub-bfd\",\"ip_address\":\"192.168.4.2\",\"mdns\":\"water-sensor-ba9.local\",\"pair_status\":127,\"ec\":{\"value\":0,\"calib_launch\":0,\"sensor_k_origin\":1,\"sensor_k_calibration\":1,\"calibration_solution\":1,\"regulation_state\":1,\"max_pumps_durations\":5000,\"set_point\":200},\"ph\":{\"value\":0,\"calib_launch\":0,\"regulation_state\":1,\"max_pumps_durations\":5000,\"set_point\":700},\"water\":{\"temperature\":0,\"pump_enable\":false}}}";
	// reply to client
	if (client->space() > strlen(jsonString.c_str()) && client->canSend())
	{
		client->add(jsonString.c_str(), strlen(jsonString.c_str()));
		client->send();
	}
}

static void handleError(void *arg, AsyncClient *client, int8_t error)
{
	Serial.printf("\n connection error %s from client %s \n", client->errorToString(error), client->remoteIP().toString().c_str());
}

static void handleDisconnect(void *arg, AsyncClient *client)
{
	Serial.printf("\n client %s disconnected \n", client->remoteIP().toString().c_str());
}

static void handleTimeOut(void *arg, AsyncClient *client, uint32_t time)
{
	Serial.printf("\n client ACK timeout ip: %s \n", client->remoteIP().toString().c_str());
}

static void handleNewClient(void *arg, AsyncClient *client)
{
	Serial.printf("\n new client has been connected to server, ip: %s", client->remoteIP().toString().c_str());
	// register events
	client->onData(&handleData, NULL);
	client->onError(&handleError, NULL);
	client->onDisconnect(&handleDisconnect, NULL);
	client->onTimeout(&handleTimeOut, NULL);
}

void setup()
{
	Serial.begin(115200);
	// create access point
	while (!WiFi.softAP(SSID, PASSWORD, 6, false, 15))
	{
		delay(500);
		Serial.print(".");
	}

	// start dns server
	if (!DNS.start(DNS_PORT, SERVER_HOST_NAME, WiFi.softAPIP()))
		Serial.printf("\n failed to start dns service \n");

	AsyncServer *server = new AsyncServer(TCP_SERVER_PORT); // start listening on tcp port 7050
	server->onClient(&handleNewClient, server);
	server->begin();
}

void loop()
{
	DNS.processNextRequest();
}