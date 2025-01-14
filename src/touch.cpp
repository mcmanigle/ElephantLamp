#include "touch.h"

#include <Arduino.h>
#include <Wire.h>

#define AXS_TOUCH_ONE_POINT_LEN             6
#define AXS_TOUCH_BUF_HEAD_LEN              2

#define AXS_TOUCH_GESTURE_POS               0
#define AXS_TOUCH_POINT_NUM                 1
#define AXS_TOUCH_EVENT_POS                 2
#define AXS_TOUCH_X_H_POS                   2
#define AXS_TOUCH_X_L_POS                   3
#define AXS_TOUCH_ID_POS                    4
#define AXS_TOUCH_Y_H_POS                   4
#define AXS_TOUCH_Y_L_POS                   5
#define AXS_TOUCH_WEIGHT_POS                6
#define AXS_TOUCH_AREA_POS                  7

#define AXS_GET_POINT_NUM(buf) buf[AXS_TOUCH_POINT_NUM]
#define AXS_GET_GESTURE_TYPE(buf)  buf[AXS_TOUCH_GESTURE_POS]
#define AXS_GET_POINT_X(buf,point_index) (((uint16_t)(buf[AXS_TOUCH_ONE_POINT_LEN*point_index+AXS_TOUCH_X_H_POS] & 0x0F) <<8) + (uint16_t)buf[AXS_TOUCH_ONE_POINT_LEN*point_index+AXS_TOUCH_X_L_POS])
#define AXS_GET_POINT_Y(buf,point_index) (((uint16_t)(buf[AXS_TOUCH_ONE_POINT_LEN*point_index+AXS_TOUCH_Y_H_POS] & 0x0F) <<8) + (uint16_t)buf[AXS_TOUCH_ONE_POINT_LEN*point_index+AXS_TOUCH_Y_L_POS])
#define AXS_GET_POINT_EVENT(buf,point_index) (buf[AXS_TOUCH_ONE_POINT_LEN*point_index+AXS_TOUCH_EVENT_POS] >> 6)

bool touch_flag = false;
void setTouchFlag(void *data);
void readTouch();

const uint8_t ALS_ADDRESS = 0x3B;
const uint8_t read_touchpad_cmd[8] = {0xb5, 0xab, 0xa5, 0x5a, 0x0, 0x0, 0x0, 0x8};

const unsigned long touch_delay =  250;
unsigned long last_touch_millis = -250;


// Screen size 640 x 180

void (*touchCallback)(int x, int y) = nullptr;

void setupTouch(void (*callback)(int x, int y))
{
  touchCallback = callback;

  pinMode(TOUCH_RES, OUTPUT);
  digitalWrite(TOUCH_RES, HIGH);delay(2);
  digitalWrite(TOUCH_RES, LOW);delay(10);
  digitalWrite(TOUCH_RES, HIGH);delay(2);

  Wire.begin(TOUCH_IICSDA, TOUCH_IICSCL);

  gpio_config_t gpio_conf;
  gpio_conf.pin_bit_mask = BIT64(TOUCH_INT);
  gpio_conf.mode = GPIO_MODE_INPUT;
  gpio_conf.pull_up_en = GPIO_PULLUP_ENABLE;
  gpio_conf.pull_down_en = GPIO_PULLDOWN_DISABLE;
  gpio_conf.intr_type = GPIO_INTR_LOW_LEVEL;
  gpio_config(&gpio_conf);
  gpio_set_intr_type(TOUCH_INT_GPIO, GPIO_INTR_LOW_LEVEL);
  gpio_install_isr_service(0);
  gpio_isr_handler_add(TOUCH_INT_GPIO, setTouchFlag, nullptr);
}

void setTouchFlag(void *data)
{
  touch_flag = true;
}

void loopTouch() {
  if(touch_flag) {
    touch_flag = false;
    readTouch();
  }
}

void readTouch()
{
  Serial.println("readTouch()");

  const int debug = 10;
  int code = 0;

  uint8_t buff[20] = {0};
  Wire.beginTransmission(ALS_ADDRESS);

  if(Wire.write(read_touchpad_cmd, 8) != 8) {
    Serial.println("Error in loopTouch(): Wire.write(read_touchpad_cmd) failed.");
    return;
  }
  if(code = Wire.endTransmission()) {
    Serial.printf("Error in loopTouch(): Wire.endTransmission() returned %d\n", code);
    return;
  }
  Wire.requestFrom(ALS_ADDRESS, (uint8_t)8);
  unsigned long timeout_start_millis = millis();
  while (!Wire.available())
    if(millis() - timeout_start_millis > 100)
      break;
  if(Wire.available())
    Wire.readBytes(buff, 8);
  else {
    Serial.printf("Error in loopTouch(): Wire never available");
    return;
  }

  uint16_t pointX;
  uint16_t pointY;
  uint16_t type = 0;

  type = AXS_GET_GESTURE_TYPE(buff);
  pointX = AXS_GET_POINT_X(buff,0);
  pointY = AXS_GET_POINT_Y(buff,0);

  if (!type && (pointX || pointY)) {
    pointX = map(pointX, 627, 10, 0, 640);
    pointY = map(pointY, 176, 6, 0, 180);
    pointX = constrain(pointX, 0, 640);
    pointY = constrain(pointY, 0, 180);

    if(debug > 0)
      printf("Touch (%d, %d); ", pointX, pointY);
    
    if(millis() - last_touch_millis > touch_delay)
    {
      if(debug > 0) printf("callback!\n");
      touchCallback(pointX, pointY);
      last_touch_millis = millis();
    }
    else if(debug > 0) printf("delay!\n");
  }
}
