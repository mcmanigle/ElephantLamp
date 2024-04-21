#include "ha_devices.h"
#include "network.h"

const char* ha_light_turn_on_path  = "/api/services/light/turn_on";
const char* ha_light_turn_off_path = "/api/services/light/turn_off";
const char* ha_light_entity_id     = "light.main_bedroom_hue_room";

const char* ha_switch_toggle_path = "/api/services/switch/toggle";
const char* ha_whitenoise_power_entity_id = "switch.white_noise_machine_white_noise_machine";

const char* ha_button_push_path = "/api/services/button/press";
const char* ha_whitenoise_volume_up_entity_id   = "button.white_noise_machine_white_noise_volume_up";
const char* ha_whitenoise_volume_down_entity_id = "button.white_noise_machine_white_noise_volume_down";

void setLamp(enum LampSetting setting)
{
  JsonDocument lamp_reply;
  JsonDocument post_params;

  post_params["entity_id"] = ha_light_entity_id;
  post_params["transition"] = 1;
  
  switch (setting)
  {
  case OFF:
    break;
  
  case GLOW:
    post_params["brightness_pct"]    =   10;
    post_params["rgb_color"][0]      =  255;
    post_params["rgb_color"][1]      =   45;
    post_params["rgb_color"][2]      =   45;
    break;
  
  case RED:
    post_params["brightness_pct"]    =   40;
    post_params["rgb_color"][0]      =  255;
    post_params["rgb_color"][1]      =   45;
    post_params["rgb_color"][2]      =   45;
    break;
  
  case RELAX:
    post_params["brightness_pct"]    =   50;
    post_params["color_temp_kelvin"] = 2200;
    break;
  
  case BRIGHT:
    post_params["brightness_pct"]    =   80;
    post_params["color_temp_kelvin"] = 2800;
    break;
  
  default:
    break;
  }

  if(setting == OFF) lamp_reply = retrieveHAJson(ha_light_turn_off_path, &post_params);
  else               lamp_reply = retrieveHAJson(ha_light_turn_on_path,  &post_params);

  Serial.println("HA Lamp Set succeeded: ");
  serializeJsonPretty(lamp_reply, Serial);
  Serial.println();
}


void white_noise_machine_power()
{
  JsonDocument post_params;
  post_params["entity_id"] = ha_whitenoise_power_entity_id;
  retrieveHAJson(ha_switch_toggle_path, &post_params);
}

void white_noise_machine_volume_down()
{
  JsonDocument post_params;
  post_params["entity_id"] = ha_whitenoise_volume_down_entity_id;
  retrieveHAJson(ha_button_push_path, &post_params);
}

void white_noise_machine_volume_up()
{
  JsonDocument post_params;
  post_params["entity_id"] = ha_whitenoise_volume_up_entity_id;
  retrieveHAJson(ha_button_push_path, &post_params);
}


