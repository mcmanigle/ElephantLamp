#include <Arduino.h>

#include "esp32s3/rom/rtc.h"
#include "driver/ledc.h"
#include "esp_system.h"
#include "station.h"
#include "network.h"
#include "ha_devices.h"
#include "telemetry.h"

#include "AXS15231B.h"
#include "touch.h"
#include "draw.h"


const char* ntpServer = "pool.ntp.org";
const char* tzString = "EST5EDT,M3.2.0,M11.1.0";

TFT_eSPI tft = TFT_eSPI();
TFT_eSprite sprite = TFT_eSprite(&tft);

const ledc_channel_t backlight_channel = LEDC_CHANNEL_0;

char loop_number = 0;

char time_string[6], am_pm_string[3];

enum DisplayPage {
  CLOCK_PAGE,
  CLOCK_PAGE_DECLUTTERED,
  WEATHER_PAGE,
  LAMP_PAGE,
  WHITE_NOISE_PAGE,
  DIM_PAGE
} current_page;

void draw();
void touchCallback(int, int);
void setupBacklight();
void backlightSetPct(int);

void draw()
{
  strftime(time_string,  sizeof(time_string),  "%I:%M", &timeinfo);
  if(time_string[0] == '0') strcpy(time_string, time_string+1);
  strcpy(am_pm_string, timeinfo.tm_hour < 12 ? "am" : "pm");
  
  Serial.printf("%lu: Drawing page %d\n", millis(), current_page);

  if(current_page == CLOCK_PAGE)
    draw_page_1(&sprite);
  else if(current_page == CLOCK_PAGE_DECLUTTERED)
    draw_page_1(&sprite, true);
  else if(current_page == WEATHER_PAGE)
    draw_weather_page(&sprite);
  else if(current_page == LAMP_PAGE)
    draw_lamp_page(&sprite);
  else if(current_page == WHITE_NOISE_PAGE)
    draw_white_noise_page(&sprite);
  else if(current_page == DIM_PAGE)
    draw_dim_page(&sprite);

  Serial.printf("Done! Pushing sprite.\n");
  lcd_PushColors_rotated_90(0, 0, 640, 180, (uint16_t*)sprite.getPointer());
}

void touchCallback(int x, int y)
{
  if(current_page == CLOCK_PAGE) {
    if(x < 200) {
      if(y < 90)
        current_page = LAMP_PAGE;
      else
        current_page = WHITE_NOISE_PAGE;
    }
    if(x > 440)
      current_page = WEATHER_PAGE;
  }
  else if(current_page == CLOCK_PAGE_DECLUTTERED) {
    if(x > 440)
      current_page = WEATHER_PAGE;
    else
      current_page = CLOCK_PAGE;
  }
  else if(current_page == WEATHER_PAGE) {
    current_page = CLOCK_PAGE;
  }
  else if(current_page == LAMP_PAGE) {
    current_page = CLOCK_PAGE;
    setLamp((LampSetting)(x * 5 / 640));
  }
  else if(current_page == WHITE_NOISE_PAGE) {
    if(x < 154)
      current_page = CLOCK_PAGE;
    else if(x < 322) {
      current_page = CLOCK_PAGE;
      white_noise_machine_power();
    }
    else if(x < 488)
      white_noise_machine_volume_down();
    else
      white_noise_machine_volume_up();
  }
  else if(current_page == DIM_PAGE) {
    backlightSetPct(100);
    current_page = CLOCK_PAGE;
  }
}

void setupBacklight()
{
  ledc_timer_config_t ledc_timer = {
    .speed_mode = LEDC_LOW_SPEED_MODE,
    .duty_resolution = LEDC_TIMER_6_BIT,
    .timer_num = LEDC_TIMER_0,
    .freq_hz = 500,
    .clk_cfg = LEDC_AUTO_CLK
  };

  ledc_channel_config_t ledc_channel = {
    .gpio_num = TFT_BL,
    .speed_mode = LEDC_LOW_SPEED_MODE,
    .channel = backlight_channel,
    .intr_type = LEDC_INTR_DISABLE,
    .timer_sel = LEDC_TIMER_0,
    .duty = 0,
    .hpoint = 0
  };

  ledc_timer_config(&ledc_timer);
  ledc_channel_config(&ledc_channel);
}

void backlightSetPct(int pct)
{
  ledc_set_duty(LEDC_LOW_SPEED_MODE, backlight_channel, pct * 63 / 100);
  ledc_update_duty(LEDC_LOW_SPEED_MODE, backlight_channel);
}
    
void setup()
{
  Serial.begin(115200);
  delay(2000);

  Serial.println("Booting!");
  
  // Serial.println("CPU0 reset reason:");
  // printResetReason(rtc_get_reset_reason(0));
  // verbosePrintResetReason(rtc_get_reset_reason(0));

  // Serial.println("CPU1 reset reason:");
  // printResetReason(rtc_get_reset_reason(1));
  // verbosePrintResetReason(rtc_get_reset_reason(1));

  wifi_init_sta();

  // printRamInfo();

  sprite.createSprite(640, 180);    // full screen landscape sprite in psram
  sprite.setSwapBytes(1);

  setupBacklight();
  setupTouch(&touchCallback);
  axs15231_init();

  configTzTime(tzString, ntpServer);

  current_page = CLOCK_PAGE;

  draw();
  backlightSetPct(100);
}

void loop() {
  Serial.printf("%lu - loop()   ", millis());
  printRamInfo();

  loopNetwork();
  loopTouch();

  if( timeinfo.tm_hour < 7 || timeinfo.tm_hour >= 19 )
  {
    if( (current_page != DIM_PAGE                      ) &&
        (millis() - last_touch_millis > 2*60*1000      )    )
    {
      backlightSetPct(5);
      current_page = DIM_PAGE;
    }
  }
  else {
    if( current_page == DIM_PAGE )
    {
      backlightSetPct(100);
      current_page = CLOCK_PAGE;
    }
    if( (current_page != CLOCK_PAGE_DECLUTTERED        ) &&
        (millis() - last_touch_millis > 2*60*1000      )    )
    {
      current_page = CLOCK_PAGE_DECLUTTERED;
    }

  }

  draw();
  delay(50);
}


