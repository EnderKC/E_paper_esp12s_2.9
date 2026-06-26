#include "network_config.h"
#include "secrets.h"

const char *mqtt_broker = MQTT_BROKER;
const char *mqtt_state_topic = "emqx/esp8266/state";
const char *mqtt_command_topic = "emqx/esp8266/cmd";
const char *mqtt_username = MQTT_USERNAME;
const char *mqtt_password = MQTT_PASSWORD;

String weather_APIkey = WEATHER_API_KEY;
String driver_api_key = DRIVER_API_KEY;

String weather_api = "http://api.seniverse.com/v3/weather/now.json?key=" +
                     weather_APIkey +
                     "&location=ip&language=zh-Hans&unit=c";

String driver_api_tianjin = "http://api.seniverse.com/v3/life/driving_restriction.json?key=" +
                            driver_api_key +
                            "&location=WWGQDCW6TBW1";

String driver_api_beijing = "http://api.seniverse.com/v3/life/driving_restriction.json?key=" +
                            driver_api_key +
                            "&location=WX4FBXXFKE4F";
