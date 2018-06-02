#include "Mqtt.h"
#include "Power.h"
#include "Monitor.h"

#include <algorithm>
#include <cstring>
#include <stdio.h>
#include <string>

#include <Windows.h>

static const char* ClientId = "computerMqtt";
static const char* PowerStatus = "computer/power/status";
static const char* PowerControl = "computer/power/control";
static const char* MonitorControl = "computer/monitor/control";
static const char* AudioStatus = "computer/audio/status";
static const char* AudioControl = "computer/audio/control";

namespace
{
	inline char charToLower(char c)
	{
		return (char)::tolower(c);
	}

	bool parseBool(bool& boolOut, const char* payload)
	{
		std::string str(payload);
		std::transform(str.begin(), str.end(), str.begin(), ::charToLower);

		if (str == "true" || str == "1")
		{
			boolOut = true;
			return true;
		}
		else if (str == "false" || str == "0")
		{
			boolOut = false;
			return true;
		}

		return false;
	}
}

class ComputerMqtt : public Mqtt
{
public:
	ComputerMqtt(const char* host, int port)
	: Mqtt(ClientId, host, port, {PowerStatus, "true"})
	{}

	virtual void onConnected() override
	{
		subscribe(PowerControl);
		subscribe(MonitorControl);
		subscribe(AudioControl);

		publish({ PowerStatus, "true" }, true);
	}

	virtual void onMessage(const Message& message) override
	{
		auto&& topic = message.topic;

		if (topic == PowerControl)
			handlePower(message);
		else if (topic == MonitorControl)
			handleMonitor(message);
		else if (topic == AudioControl)
			handleAudio(message);

	}

	void handlePower(const Message& message)
	{
		bool value = false;
		if (!parseBool(value, message.payload.c_str()))
		{
			printf("Could not parse data for topic: %s", PowerControl);
			return;
		}

		if (value == true)
			return;

		printf("Suspending machine");
		standby();
	}

	void handleMonitor(const Message& message)
	{
		bool value = false;
		if (!parseBool(value, message.payload.c_str()))
		{
			printf("Could not parse data for topic: %s", MonitorControl);
			return;
		}

		printf("Turning monitor %s\n", value ? "On" : "Off");
		setMonitorPower(value);
	}

	void handleAudio(const Message& /*message*/)
	{
		// FIXME
	}
};

int main(int argc, char* argv[])
{
	if (argc < 3)
	{
		printf("Too few arguments: %d\n", argc);
		printf("Usage:\n");
		printf("%s <mqtt host> <port>", argv[0]);
		return -1;
	}

	const char* host = argv[1];
	const int port= atoi(argv[2]);
	if (port == 0)
	{
		printf("Invalid port specified\n");
		return -2;
	}

	printf("Starting service\n");

	ComputerMqtt mqtt(host, port);

	while (true)
	{
		Sleep(1000);
	}

	return 0;
}
