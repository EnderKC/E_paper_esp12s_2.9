#include "network_config.h"

const char *mqtt_broker = "x93656e1.ala.cn-hangzhou.emqxsl.cn";
const char *mqtt_state_topic = "emqx/esp8266/state";
const char *mqtt_command_topic = "emqx/esp8266/cmd";
const char *mqtt_username = "EnderKC";
const char *mqtt_password = "QWEqwe123";

String weather_APIkey = "S6HxLK_QBsQ8jY3Rv";
String driver_api_key = "SuBkUBbhV4zhLGD8R";

String weather_api = "http://api.seniverse.com/v3/weather/now.json?key=" +
                     weather_APIkey +
                     "&location=ip&language=zh-Hans&unit=c";

String driver_api_tianjin = "http://api.seniverse.com/v3/life/driving_restriction.json?key=" +
                            driver_api_key +
                            "&location=WWGQDCW6TBW1";

String driver_api_beijing = "http://api.seniverse.com/v3/life/driving_restriction.json?key=" +
                            driver_api_key +
                            "&location=WX4FBXXFKE4F";
