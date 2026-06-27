#include "network_config.h"
#include "secrets.h"
#include "runtime_config.h"

namespace
{
String mqtt_broker_value = MQTT_BROKER;
String mqtt_username_value = MQTT_USERNAME;
String mqtt_password_value = MQTT_PASSWORD;
}

const char *mqtt_broker = mqtt_broker_value.c_str();
const char *mqtt_state_topic = "emqx/esp8266/state";
const char *mqtt_command_topic = "emqx/esp8266/cmd";
const char *mqtt_username = mqtt_username_value.c_str();
const char *mqtt_password = mqtt_password_value.c_str();

String weather_APIkey = WEATHER_API_KEY;
String driver_api_key = DRIVER_API_KEY;
String weather_location = "ip";

String weather_api;
String driver_api_tianjin;
String driver_api_beijing;

void applyNetworkConfig()
{
    const RuntimeConfig &config = getRuntimeConfig();

    mqtt_broker_value = config.mqttBroker;
    mqtt_username_value = config.mqttUsername;
    mqtt_password_value = config.mqttPassword;
    mqtt_broker = mqtt_broker_value.c_str();
    mqtt_username = mqtt_username_value.c_str();
    mqtt_password = mqtt_password_value.c_str();

    weather_APIkey = config.weatherApiKey;
    driver_api_key = config.driverApiKey;
    weather_location = config.weatherLocation.length() ? config.weatherLocation : "ip";

    weather_api = "http://api.seniverse.com/v3/weather/now.json?key=" +
                  weather_APIkey +
                  "&location=" + weather_location +
                  "&language=zh-Hans&unit=c";

    driver_api_tianjin = "http://api.seniverse.com/v3/life/driving_restriction.json?key=" +
                         driver_api_key +
                         "&location=WWGQDCW6TBW1";

    driver_api_beijing = "http://api.seniverse.com/v3/life/driving_restriction.json?key=" +
                         driver_api_key +
                         "&location=WX4FBXXFKE4F";
}
