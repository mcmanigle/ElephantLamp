#pragma once

#include <TFT_eSPI.h>


extern char time_string[], am_pm_string[];


void draw_page_1(TFT_eSprite *sprite, bool decluttered=false);
void draw_lamp_page(TFT_eSprite *sprite);
void draw_white_noise_page(TFT_eSprite *sprite);
void draw_dim_page(TFT_eSprite *sprite);
void draw_weather_page(TFT_eSprite *sprite);
