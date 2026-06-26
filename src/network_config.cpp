#include "network_config.h"

const char *mqtt_broker = "";
const char *mqtt_state_topic = "emqx/esp8266/state";
const char *mqtt_command_topic = "emqx/esp8266/cmd";
const char *mqtt_username = "";
const char *mqtt_password = "";

String weather_APIkey = "";
String driver_api_key = "";

String weather_api = "http://api.seniverse.com/v3/weather/now.json?key=" +
                     weather_APIkey +
                     "&location=ip&language=zh-Hans&unit=c";

String driver_api_tianjin = "http://api.seniverse.com/v3/life/driving_restriction.json?key=" +
                            driver_api_key +
                            "&location=WWGQDCW6TBW1";

String driver_api_beijing = "http://api.seniverse.com/v3/life/driving_restriction.json?key=" +
                            driver_api_key +
                            "&location=WX4FBXXFKE4F";
