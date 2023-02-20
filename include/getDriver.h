String driver_api = "https://api.seniverse.com/v3/life/driving_restriction.json?key=" + driver_api_key + "&location=WWGQDCW6TBW1";

int timer_driver = 1;

Ticker driver_ticker;

void analyze_weather_json(String input, String (&data)[4])
{
    // String input;

    StaticJsonDocument<1024> doc;

    DeserializationError error = deserializeJson(doc, input);

    if (error)
    {
        Serial.print(F("deserializeJson() failed: "));
        Serial.println(error.f_str());
        return;
    }

    JsonObject results_0_location = doc["results"][0]["location"];
    String results_0_location_id = results_0_location["id"];                           // "WWGQDCW6TBW1"
    String results_0_location_name = results_0_location["name"];                       // "天津"
    String results_0_location_country = results_0_location["country"];                 // "CN"
    String results_0_location_path = results_0_location["path"];                       // "天津,天津,中国"
    String results_0_location_timezone = results_0_location["timezone"];               // "Asia/Shanghai"
    String results_0_location_timezone_offset = results_0_location["timezone_offset"]; // "+08:00"

    JsonObject results_0_restriction = doc["results"][0]["restriction"];
    String results_0_restriction_penalty = results_0_restriction["penalty"];
    String results_0_restriction_region = results_0_restriction["region"]; // "外环线(不含)以内道路"
    String results_0_restriction_time = results_0_restriction["time"];     // "每日7时至19时"
    String results_0_restriction_remarks = results_0_restriction["remarks"];

    for (JsonObject results_0_restriction_limit : results_0_restriction["limits"].as<JsonArray>())
    {

        String results_0_restriction_limit_date = results_0_restriction_limit["date"]; // "2023-02-20", ...

        String results_0_restriction_limit_plates_0 = results_0_restriction_limit["plates"][0]; // "5", ...
        String results_0_restriction_limit_plates_1 = results_0_restriction_limit["plates"][1]; // "0", ...

        String results_0_restriction_limit_memo = results_0_restriction_limit["memo"]; // "尾号限行", "尾号限行", ...
    }
}