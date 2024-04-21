#pragma once

#include <ArduinoJson.h>

#include <time.h>

extern struct tm timeinfo;

extern char nursery_temperature_string[];
extern bool outside_time_night;
extern char outside_temperature_string[];
extern char outside_weather_string[];
extern char forecast_time_strings[][8];
extern bool forecast_time_night[];
extern char forecast_temperature_strings[][8];
extern char forecast_weather_strings[][32];

void loopNetwork();

JsonDocument retrieveHAJson(const char *path, JsonDocument *post_data = NULL);
