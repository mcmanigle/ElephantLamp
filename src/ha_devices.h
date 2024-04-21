#pragma once

enum LampSetting {
  OFF,
  GLOW,
  RED,
  RELAX,
  BRIGHT
};

void setLamp(enum LampSetting setting);

void white_noise_machine_power();
void white_noise_machine_volume_down();
void white_noise_machine_volume_up();
