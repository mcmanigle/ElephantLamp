#include "weather_icons.h"

void draw_weather_icon(TFT_eSprite *sprite, int x, int y, const char *weather_string, bool is_night)
{
  if(is_night)
    return draw_weather_icon_night(sprite, x, y, weather_string);
  else
    return draw_weather_icon_day(sprite, x, y, weather_string);
}

void draw_weather_icon_day(TFT_eSprite *sprite, int x, int y, const char *weather_string)
{
  const unsigned short *image = code_yellow_data;
  if(!strcmp(weather_string, "clear-night"))     image = clear_day_data;
  if(!strcmp(weather_string, "cloudy"))          image = overcast_data;
  if(!strcmp(weather_string, "fog"))             image = fog_day_data;
  if(!strcmp(weather_string, "hail"))            image = overcast_hail_data;
  if(!strcmp(weather_string, "lightning"))       image = thunderstorms_overcast_data;
  if(!strcmp(weather_string, "lightning-rainy")) image = thunderstorms_overcast_rain_data;
  if(!strcmp(weather_string, "partlycloudy"))    image = partly_cloudy_day_data;
  if(!strcmp(weather_string, "pouring"))         image = extreme_rain_data;
  if(!strcmp(weather_string, "rainy"))           image = overcast_rain_data;
  if(!strcmp(weather_string, "snowy"))           image = overcast_snow_data;
  if(!strcmp(weather_string, "snowy-rainy"))     image = overcast_sleet_data;
  if(!strcmp(weather_string, "sunny"))           image = clear_day_data;
  if(!strcmp(weather_string, "windy"))           image = wind_data;
  if(!strcmp(weather_string, "windy-variant"))   image = wind_data;
  sprite->pushImage(x, y, weather_icon_width, weather_icon_height, image);
}

void draw_weather_icon_night(TFT_eSprite *sprite, int x, int y, const char *weather_string)
{
  const unsigned short *image = code_yellow_data;
  if(!strcmp(weather_string, "clear-night"))     image = clear_night_data;
  if(!strcmp(weather_string, "cloudy"))          image = overcast_data;
  if(!strcmp(weather_string, "fog"))             image = fog_night_data;
  if(!strcmp(weather_string, "hail"))            image = overcast_hail_data;
  if(!strcmp(weather_string, "lightning"))       image = thunderstorms_overcast_data;
  if(!strcmp(weather_string, "lightning-rainy")) image = thunderstorms_overcast_rain_data;
  if(!strcmp(weather_string, "partlycloudy"))    image = partly_cloudy_night_data;
  if(!strcmp(weather_string, "pouring"))         image = extreme_rain_data;
  if(!strcmp(weather_string, "rainy"))           image = overcast_rain_data;
  if(!strcmp(weather_string, "snowy"))           image = overcast_snow_data;
  if(!strcmp(weather_string, "snowy-rainy"))     image = overcast_sleet_data;
  if(!strcmp(weather_string, "sunny"))           image = clear_night_data;
  if(!strcmp(weather_string, "windy"))           image = wind_data;
  if(!strcmp(weather_string, "windy-variant"))   image = wind_data;
  sprite->pushImage(x, y, weather_icon_width, weather_icon_height, image);
}
