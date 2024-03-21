#include "esp32s3/rom/rtc.h"
#include "driver/ledc.h"
#include "esp_system.h"
#include "station.h"
#include "network.h"

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
  WEATHER_PAGE,
  LAMP_PAGE,
  DIM_PAGE
} current_page;


void printResetReason(int reason)
{
  switch ( reason)
  {
    case 1 : Serial.println ("POWERON_RESET");break;          /**<1,  Vbat power on reset*/
    case 3 : Serial.println ("SW_RESET");break;               /**<3,  Software reset digital core*/
    case 4 : Serial.println ("OWDT_RESET");break;             /**<4,  Legacy watch dog reset digital core*/
    case 5 : Serial.println ("DEEPSLEEP_RESET");break;        /**<5,  Deep Sleep reset digital core*/
    case 6 : Serial.println ("SDIO_RESET");break;             /**<6,  Reset by SLC module, reset digital core*/
    case 7 : Serial.println ("TG0WDT_SYS_RESET");break;       /**<7,  Timer Group0 Watch dog reset digital core*/
    case 8 : Serial.println ("TG1WDT_SYS_RESET");break;       /**<8,  Timer Group1 Watch dog reset digital core*/
    case 9 : Serial.println ("RTCWDT_SYS_RESET");break;       /**<9,  RTC Watch dog Reset digital core*/
    case 10 : Serial.println ("INTRUSION_RESET");break;       /**<10, Instrusion tested to reset CPU*/
    case 11 : Serial.println ("TGWDT_CPU_RESET");break;       /**<11, Time Group reset CPU*/
    case 12 : Serial.println ("SW_CPU_RESET");break;          /**<12, Software reset CPU*/
    case 13 : Serial.println ("RTCWDT_CPU_RESET");break;      /**<13, RTC Watch dog Reset CPU*/
    case 14 : Serial.println ("EXT_CPU_RESET");break;         /**<14, for APP CPU, reseted by PRO CPU*/
    case 15 : Serial.println ("RTCWDT_BROWN_OUT_RESET");break;/**<15, Reset when the vdd voltage is not stable*/
    case 16 : Serial.println ("RTCWDT_RTC_RESET");break;      /**<16, RTC Watch dog reset digital core and rtc module*/
    default : Serial.println ("NO_MEAN");
  }
}

void verbosePrintResetReason(int reason)
{
  switch ( reason)
  {
    case 1  : Serial.println ("Vbat power on reset");break;
    case 3  : Serial.println ("Software reset digital core");break;
    case 4  : Serial.println ("Legacy watch dog reset digital core");break;
    case 5  : Serial.println ("Deep Sleep reset digital core");break;
    case 6  : Serial.println ("Reset by SLC module, reset digital core");break;
    case 7  : Serial.println ("Timer Group0 Watch dog reset digital core");break;
    case 8  : Serial.println ("Timer Group1 Watch dog reset digital core");break;
    case 9  : Serial.println ("RTC Watch dog Reset digital core");break;
    case 10 : Serial.println ("Instrusion tested to reset CPU");break;
    case 11 : Serial.println ("Time Group reset CPU");break;
    case 12 : Serial.println ("Software reset CPU");break;
    case 13 : Serial.println ("RTC Watch dog Reset CPU");break;
    case 14 : Serial.println ("for APP CPU, reseted by PRO CPU");break;
    case 15 : Serial.println ("Reset when the vdd voltage is not stable");break;
    case 16 : Serial.println ("RTC Watch dog reset digital core and rtc module");break;
    default : Serial.println ("NO_MEAN");
  }
}

void printRamInfo()
{
  multi_heap_info_t info;
  heap_caps_get_info(&info, MALLOC_CAP_INTERNAL | MALLOC_CAP_8BIT); // internal RAM, memory capable to store data or to create new task
  Serial.printf("RAM: total free %d; ", info.total_free_bytes);   // total currently free in all non-continues blocks
  Serial.printf("minimum free %d; ", info.minimum_free_bytes);  // minimum free ever
  Serial.printf("largest block %d\n", info.largest_free_block);   // largest continues block to allocate big array
  Serial.printf("Heap: total %d; ", ESP.getHeapSize());
  Serial.printf("free %d\n", ESP.getFreeHeap());
  Serial.printf("PSRAM: total %d; ", ESP.getPsramSize());
  Serial.printf("free %d\n", ESP.getFreePsram());
}



void draw()
{
  strftime(time_string,  sizeof(time_string),  "%I:%M", &timeinfo);
  if(time_string[0] == '0') strcpy(time_string, time_string+1);
  strcpy(am_pm_string, timeinfo.tm_hour < 12 ? "am" : "pm");
  
  if(current_page == CLOCK_PAGE)
    draw_page_1(&sprite);
  else if(current_page == WEATHER_PAGE)
    draw_weather_page(&sprite);
  else if(current_page == LAMP_PAGE)
    draw_lamp_page(&sprite);
  else if(current_page == DIM_PAGE)
    draw_dim_page(&sprite);

  lcd_PushColors_rotated_90(0, 0, 640, 180, (uint16_t*)sprite.getPointer());
}

void touchCallback(int x, int y)
{
  if(current_page == CLOCK_PAGE) {
    if(x < 200)
      current_page = LAMP_PAGE;
    if(x > 440)
      current_page = WEATHER_PAGE;
  }
  else if(current_page == WEATHER_PAGE) {
    current_page = CLOCK_PAGE;
  }
  else if(current_page == LAMP_PAGE) {
    current_page = CLOCK_PAGE;
    setLamp((LampSetting)(x * 5 / 640));
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
  
  Serial.println("CPU0 reset reason:");
  printResetReason(rtc_get_reset_reason(0));
  verbosePrintResetReason(rtc_get_reset_reason(0));

  Serial.println("CPU1 reset reason:");
  printResetReason(rtc_get_reset_reason(1));
  verbosePrintResetReason(rtc_get_reset_reason(1));

  wifi_init_sta();

  printRamInfo();

  sprite.createSprite(640, 180);    // full screen landscape sprite in psram
  sprite.setSwapBytes(1);

  setupBacklight();
  setupTouch();
  axs15231_init();

  configTzTime(tzString, ntpServer);

  current_page = CLOCK_PAGE;

  draw();
  backlightSetPct(100);
}

void loop() {
  // Serial.printf("%lu - loop()   ", millis());
  // printRamInfo();

  loopNetwork();

  loopTouch(&touchCallback);

  if( timeinfo.tm_hour < 7 || timeinfo.tm_hour >= 19 )
  {
    if( (current_page != DIM_PAGE                      ) &&
        (millis() - last_touch_millis > 2*60*1000      )    )
    {
      backlightSetPct(10);
      current_page = DIM_PAGE;
    }
  }
  else {
    if( current_page == DIM_PAGE )
    {
      backlightSetPct(100);
      current_page = CLOCK_PAGE;
    }
  }

  draw();
  delay(50);
}


