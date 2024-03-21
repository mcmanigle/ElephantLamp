#include "draw.h"

#include "network.h"

#include "weather_icons.h"
#include "bulb.h"
#include "baby.h"

#include "Free_Fonts.h"
#include "AvenirMedium.h"
#include "AvenirNextCondensed.h"

// Screen size 640 x 180

unsigned short rgb(unsigned char r, unsigned char g, unsigned char b)
{
  return ((r & 0xf8) << 8) | ((g & 0xfc) << 3) | (b >> 3);
}

void draw_page_1(TFT_eSprite *sprite) {
  sprite->fillSprite(TFT_BLACK);

  sprite->setTextColor(TFT_WHITE, TFT_BLACK);
  sprite->setTextDatum(BR_DATUM);
  sprite->loadFont(AvenirNextCondensed124);
  sprite->drawString(time_string, 325, 162);
  sprite->unloadFont();
  
  sprite->loadFont(AvenirNextCondensed72);
  sprite->setTextDatum(BL_DATUM);
  sprite->drawString(am_pm_string, 345, 152);
  sprite->unloadFont();

  sprite->loadFont(AvenirNextCondensed36);
  sprite->setTextDatum(MR_DATUM);
  draw_weather_icon(sprite, 470, 15, outside_weather_string, outside_time_night);
  sprite->drawString(outside_temperature_string, 630,  55);
  sprite->pushImage(484, 105, baby_width, baby_height, baby_data);
  sprite->drawString(nursery_temperature_string, 630, 134);
  sprite->unloadFont();

  sprite->pushImage(10, 90 - bulb_height/2, bulb_width, bulb_height, bulb_data);
}

void draw_dim_page(TFT_eSprite *sprite) {
  sprite->fillSprite(TFT_BLACK);
  sprite->setTextColor(TFT_RED, TFT_BLACK);

  sprite->loadFont(AvenirMedium124);
  sprite->setTextDatum(BR_DATUM);
  sprite->drawString(time_string, 450, 157);
  sprite->unloadFont();

  sprite->loadFont(AvenirNextCondensed72);
  sprite->setTextDatum(BL_DATUM);
  sprite->drawString(am_pm_string, 480, 152);
  sprite->unloadFont();
}

void draw_lamp_page(TFT_eSprite *sprite) {
  const int bottom_line_y = 135;
  const int horizontal_step = 128;
  int x = horizontal_step / 2;
  const int circle_y = 92;
  const int r = 25;
  const int stroke = 2;
  const unsigned short stroke_color = TFT_LIGHTGREY;

  sprite->fillSprite(TFT_BLACK);

  sprite->loadFont(AvenirNextCondensed36);
  sprite->setTextDatum(TC_DATUM);
  sprite->drawString("Lamp Control", 320, 10);
  
  sprite->drawString("Off", x, bottom_line_y);
  sprite->fillCircle(x, circle_y, r+stroke, stroke_color);
  sprite->fillCircle(x, circle_y, r, TFT_BLACK);

  x += horizontal_step;
  sprite->drawString("Glow", x, bottom_line_y);
  sprite->fillCircle(x, circle_y, r+stroke, stroke_color);
  sprite->fillCircle(x, circle_y, r, rgb(64, 0, 0));

  x += horizontal_step;
  sprite->drawString("Red", x, bottom_line_y);
  sprite->fillCircle(x, circle_y, r+stroke, stroke_color);
  sprite->fillCircle(x, circle_y, r, rgb(128, 0, 0));

  x += horizontal_step;
  sprite->drawString("Relax", x, bottom_line_y);
  sprite->fillCircle(x, circle_y, r+stroke, stroke_color);
  sprite->fillCircle(x, circle_y, r, rgb(255, 224, 192));

  x += horizontal_step;
  sprite->drawString("Bright", x, bottom_line_y);
  sprite->fillCircle(x, circle_y, r+stroke, stroke_color);
  sprite->fillCircle(x, circle_y, r, rgb(242, 242, 255));

  sprite->unloadFont();
}

void draw_weather_page(TFT_eSprite *sprite) {
  const int top_line_y = 14;
  const int icon_line_y = 90 - weather_icon_height/2;
  const int bottom_line_y = 135;
  const int left_margin = 0;
  const int horizontal_step = 128;

  sprite->fillSprite(TFT_BLACK);

  sprite->loadFont(AvenirNextCondensed36);
  sprite->setTextColor(TFT_WHITE, TFT_BLACK);
  sprite->setTextDatum(TC_DATUM);

  sprite->drawString("Now", left_margin + horizontal_step/2, top_line_y);
  draw_weather_icon(sprite, horizontal_step/2 - weather_icon_width/2, icon_line_y, outside_weather_string, outside_time_night);
  sprite->drawString(outside_temperature_string, left_margin + horizontal_step/2, bottom_line_y);

  for(int i = 0; i < 4; i++)
  {
    draw_weather_icon(sprite, horizontal_step/2 + horizontal_step*(i+1) - weather_icon_width/2, icon_line_y, forecast_weather_strings[i], forecast_time_night[i]);
    sprite->drawString(forecast_time_strings[i], left_margin + horizontal_step/2 + horizontal_step*(i+1), top_line_y);
    sprite->drawString(forecast_temperature_strings[i], left_margin + horizontal_step/2 + horizontal_step*(i+1), bottom_line_y);
  }

  sprite->unloadFont(); 
}