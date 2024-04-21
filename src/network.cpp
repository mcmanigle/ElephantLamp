#include "network.h"
#include "esp_http_client.h"
#include "secrets.h"
#include "http_extras.h"


const char* ha_host = "192.168.44.10";
const int ha_port = 8123;
const char* ha_sun_path            = "/api/states/sun.sun";
const char* ha_weather_path        = "/api/states/weather.510_grant_forest";
const char* ha_forecast_path       = "/api/states/sensor.home_forecast_hourly";
const char* ha_nursery_temp_path   = "/api/states/sensor.nursery_temperature";
const char* ha_millis_path         = "/api/states/input_number.elephant_lamp_millis";

const unsigned char ha_debug_level = 5;

unsigned long ntpPreviousMillis = 0 - 600000;
const unsigned long ntpInterval = 30000;
unsigned long haPreviousMillis = 0 - 600000;
const unsigned long haInterval = 20000;

const int   zuluOffsetHours    = -5;
const int   zuluOffsetHoursDST = -4;

struct tm timeinfo;
time_t next_sun_change;

char weather_temp_unit_string[5];

char nursery_temperature_string[8];
bool outside_time_night;
char outside_temperature_string[8];
char outside_weather_string[32];
char forecast_time_strings[5][8];
bool forecast_time_night[5];
char forecast_temperature_strings[5][8];
char forecast_weather_strings[5][32];

void updateHA();


void loopNetwork() {
  unsigned long currentMillis = millis();

  getLocalTime(&timeinfo);
  if (currentMillis - ntpPreviousMillis >= ntpInterval) {
    Serial.print("Local time: ");
    Serial.println(&timeinfo, "%A, %B %d %Y %H:%M:%S");
    ntpPreviousMillis = currentMillis;
  }

  if (currentMillis - haPreviousMillis >= haInterval) {
    updateHA();
    haPreviousMillis = currentMillis;
  }
}

JsonDocument retrieveHAJson(const char *path, JsonDocument *post_data) {
  JsonDocument result;

  char *http_buffer = (char *)calloc(MAX_HTTP_OUTPUT_BUFFER+1, sizeof(char));
  char *post_json;
  size_t post_json_length;
  char post_json_length_str[8];

  if(ha_debug_level > 2) Serial.printf("retrieveHAJson(%s);\n", path);

  esp_http_client_config_t http_client_config = {
    .host = ha_host,
    .port = ha_port,
    .path = path,
    .disable_auto_redirect = false,
    .event_handler = _http_event_handler,
    .user_data = http_buffer,
  };
  esp_http_client_handle_t client = esp_http_client_init(&http_client_config);
  esp_http_client_set_header(client, "Authorization", HA_AUTHORIZATION_TOKEN);

  if(post_data) {
    post_json_length = measureJson(*post_data);
    sprintf(post_json_length_str, "%u", post_json_length);
    post_json = (char *)calloc(post_json_length+1, sizeof(char));
    serializeJson(*post_data, post_json, post_json_length+1);
    Serial.printf("- Post data: %s\n", post_json);
    esp_http_client_set_method(client, HTTP_METHOD_POST);
    esp_http_client_set_post_field(client, post_json, post_json_length);
    esp_http_client_set_header(client, "Content-Type", "application/json");
    esp_http_client_set_header(client, "Content-Length", post_json_length_str);
  }

  esp_err_t http_err = esp_http_client_perform(client);
  if(ha_debug_level > 3)
    Serial.printf("- ESP error code: %d\n", http_err);
  if(ha_debug_level > 2)
    Serial.printf("- HTTP status code: %d\n", esp_http_client_get_status_code(client));

  if(http_err == ESP_OK && esp_http_client_get_status_code(client) == HttpStatus_Ok) {
    DeserializationError error = deserializeJson(result, http_buffer);
    if (error && ha_debug_level > 0) {
      Serial.print("- deserializeJson() failed: ");
      Serial.println(error.c_str());
    } else {
      if(ha_debug_level > 4) {
        Serial.println("- deserializeJson() succeeded: ");
        serializeJsonPretty(result, Serial);
        Serial.println();
      }
    }
  }
  else if(http_err == ESP_OK)
    Serial.printf("- Returned: %s\n", http_buffer);
  esp_http_client_cleanup(client);
  free(http_buffer);
  if(post_data) free(post_json);
  return result;
}

void updateHA() {
  static unsigned char command = 0;

  JsonDocument result;

  if(ha_debug_level > 2) Serial.printf("updateHA(); command = %u\n", command);

  if(command == 0)
  {
    result = retrieveHAJson(ha_sun_path);

    if(!result.isNull())
    {
      outside_time_night = strcmp(result["state"].as<const char*>(), "above_horizon");

      struct tm t;
      strptime(result["attributes"][outside_time_night ? "next_rising" : "next_setting"].as<const char*>(), "%Y-%m-%dT%T+00:00", &t);
      next_sun_change = mktime(&t);

      if(ha_debug_level > 3) {
        Serial.print("Sun State String: ");
        Serial.println(result["state"].as<const char*>());
      }
    }
  }

  if(command == 1)
  {
    result = retrieveHAJson(ha_weather_path);

    if(!result.isNull())
    {
      strcpy(outside_weather_string, result["state"].as<const char*>());
      strcpy(weather_temp_unit_string, result["attributes"]["temperature_unit"].as<const char*>());
      sprintf(outside_temperature_string, "%d %s", result["attributes"]["temperature"].as<int>(),
                                                   weather_temp_unit_string);
    }

    if(ha_debug_level > 3) {
      Serial.print("Outside Weather String: ");
      Serial.println(outside_weather_string);
      Serial.print("Outside Temperature String: ");
      Serial.println(outside_temperature_string);
    }
  }

  if(command == 2)
  {
    result = retrieveHAJson(ha_forecast_path);

    if(!result.isNull())
    {
      for(int i = 0; i < 5; i++)
      {
        struct tm t;
        strptime(result["attributes"]["forecast"][i]["datetime"].as<const char*>(), "%Y-%m-%dT%T+00:00", &t);
        forecast_time_night[i] = mktime(&t) > next_sun_change ? !outside_time_night : outside_time_night;

        int h = ( t.tm_hour + 24 + (timeinfo.tm_isdst ? zuluOffsetHoursDST : zuluOffsetHours) ) % 24;

        if     (h ==  0) sprintf(forecast_time_strings[i], "Midn't"       );
        else if(h  < 12) sprintf(forecast_time_strings[i], "%d am", h     );
        else if(h == 12) sprintf(forecast_time_strings[i], "Noon"         );
        else             sprintf(forecast_time_strings[i], "%d pm", h % 12);

        strcpy(forecast_weather_strings[i], result["attributes"]["forecast"][i]["condition"].as<const char*>());

        sprintf(forecast_temperature_strings[i], "%d %s", result["attributes"]["forecast"][i]["temperature"].as<int>(),
                                                          weather_temp_unit_string);
      }

      if(ha_debug_level > 3) {
        for(int i = 0; i < 5; i++) {
          Serial.printf("Forecast Time String #%d: ", i+1);
          Serial.println(forecast_time_strings[i]);
          Serial.printf("Forecast Weather String #%d: ", i+1);
          Serial.println(forecast_weather_strings[i]);
          Serial.printf("Forecast Temperature String #%d: ", i+1);
          Serial.println(forecast_temperature_strings[i]);
        }
      }
    }
  }

  if(command == 3)
  {
    result = retrieveHAJson(ha_nursery_temp_path);

    if(!result.isNull())
    {
      sprintf(nursery_temperature_string, "%d %s", int(round(atof(result["state"].as<const char*>()))),
                                          result["attributes"]["unit_of_measurement"].as<const char*>());
      if(ha_debug_level > 3) {
        Serial.print("Nursery Temperature String: ");
        Serial.println(nursery_temperature_string);
      }
    }
  }

  if(command == 4)
  {
    JsonDocument post_params;
    post_params["state"] = millis();

    result = retrieveHAJson(ha_millis_path, &post_params);
  }

  command = (command + 1) % 5;
}
