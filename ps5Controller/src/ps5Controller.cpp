#include "ps5Controller.h"
#include "math.h"
#include <esp_bt_defs.h>
#include <esp_bt_main.h>

extern "C" {
#include "ps5.h"
}

#define ESP_BD_ADDR_HEX_PTR(addr) \
  (uint8_t*)addr + 0, (uint8_t*)addr + 1, (uint8_t*)addr + 2, \
  (uint8_t*)addr + 3, (uint8_t*)addr + 4, (uint8_t*)addr + 5

ps5Controller::ps5Controller() {}

bool ps5Controller::begin() {
  ps5SetEventObjectCallback(this, &ps5Controller::_event_callback);
  ps5SetConnectionObjectCallback(this, &ps5Controller::_connection_callback);

  if (!btStarted() && !btStart()) {
    log_e("btStart failed");
    return false;
  }

  esp_bluedroid_status_t btState = esp_bluedroid_get_status();
  if (btState == ESP_BLUEDROID_STATUS_UNINITIALIZED) {
    if (esp_bluedroid_init()) {
      log_e("esp_bluedroid_init failed");
      return false;
    }
  }

  if (btState != ESP_BLUEDROID_STATUS_ENABLED) {
    if (esp_bluedroid_enable()) {
      log_e("esp_bluedroid_enable failed");
      return false;
    }
  }

  ps5Init();
  return true;
}

bool ps5Controller::begin(const char* mac) {
  esp_bd_addr_t addr;
    
  if (sscanf(mac, ESP_BD_ADDR_STR, ESP_BD_ADDR_HEX_PTR(addr)) != ESP_BD_ADDR_LEN) {
    log_e("Could not convert %s\n to a MAC address", mac);
    return false;
  }

  ps5_l2cap_connect(addr);
  // ps5SetBluetoothMacAddress(addr);

  return begin();
}

void ps5Controller::end() {}

bool ps5Controller::isConnected() {
  auto connected = ps5IsConnected();
  static unsigned long tryReconnectAt = 0;
  if (!connected && millis() - tryReconnectAt > 5000UL) {
    tryReconnectAt = millis();
    ps5_l2cap_reconnect();
  }
  return connected;
}

void ps5Controller::ps5_disable_lightbar()
{
  output_report.valid_flag2 = DS5_FLAG2_LIGHTBAR_SETUP_CONTROL_ENABLE;
  output_report.lightbar_setup = DS5_LIGHTBAR_SETUP_LIGHT_OUT;

  ps5_send_output_report(&output_report);
}

void ps5Controller::setTriggerEffectOff(bool isLeft, bool isRight)
{
    TriggerEffectType type = Off;
    uint8_t * buffer = isLeft ? output_report.left_trigger_param : output_report.right_trigger_param;

    buffer[0] = 0x00;
    buffer[1] = 0x00;
    buffer[2] = 0x00;
    buffer[3] = 0x00;
    buffer[4] = 0x00;
    buffer[5] = 0x00;
    buffer[6] = 0x00;
    buffer[7] = 0x00;
    buffer[8] = 0x00;
    buffer[9] = 0x00;

    if (isLeft)
    {
      output_report.left_trigger_motor_mode = type;
      output_report.valid_flag0 |= DS_OUTPUT_VALID_FLAG0_LEFT_TRIGGER_MOTOR_ENABLE;
    }
    else
    {
      output_report.valid_flag0 &= ~DS_OUTPUT_VALID_FLAG0_LEFT_TRIGGER_MOTOR_ENABLE;
    }

    if (isRight)
    {
      output_report.right_trigger_motor_mode = type;
      output_report.valid_flag0 |= DS_OUTPUT_VALID_FLAG0_RIGHT_TRIGGER_MOTOR_ENABLE;
    }
    else
    {
      output_report.valid_flag0 &= ~DS_OUTPUT_VALID_FLAG0_RIGHT_TRIGGER_MOTOR_ENABLE;
    }

    ps5_send_output_report(&output_report);
}

void ps5Controller::setTriggerEffectWeapon(bool isLeft, uint8_t start_pos, uint8_t end_pos, uint8_t strength)
{
    TriggerEffectType type = Weapon;
    uint8_t * buffer = isLeft ? output_report.left_trigger_param : output_report.right_trigger_param;
    
    if(isLeft)
    {
        output_report.left_trigger_motor_mode = type;
        output_report.valid_flag0 |= DS_OUTPUT_VALID_FLAG0_LEFT_TRIGGER_MOTOR_ENABLE;
    }
    else
    { 
        output_report.right_trigger_motor_mode = type;
        output_report.valid_flag0 |= DS_OUTPUT_VALID_FLAG0_RIGHT_TRIGGER_MOTOR_ENABLE;
    }


    uint16_t startAndStopZones = (uint16_t)((1 << start_pos) | (1 << end_pos));

    buffer[0] = (uint8_t)((startAndStopZones >> 0) & 0xff);
    buffer[1] = (uint8_t)((startAndStopZones >> 8) & 0xff);
    buffer[2] = strength - 1;
    buffer[3] = 0x00;
    buffer[4] = 0x00;
    buffer[5] = 0x00;
    buffer[6] = 0x00;
    buffer[7] = 0x00;
    buffer[8] = 0x00;
    buffer[9] = 0x00;

    ps5_send_output_report(&output_report);
}

void ps5Controller::setTriggerEffectFeedback(bool isLeft, uint8_t start_pos, uint8_t strength)
{
    TriggerEffectType type = Feedback;
    uint8_t * buffer = isLeft ? output_report.left_trigger_param : output_report.right_trigger_param;
    
    if(isLeft)
    {
        output_report.left_trigger_motor_mode = type;
        output_report.valid_flag0 |= DS_OUTPUT_VALID_FLAG0_LEFT_TRIGGER_MOTOR_ENABLE;
    }
    else
    { 
        output_report.right_trigger_motor_mode = type;
        output_report.valid_flag0 |= DS_OUTPUT_VALID_FLAG0_RIGHT_TRIGGER_MOTOR_ENABLE;
    }

    uint8_t forceValue = (uint8_t)((strength - 1) & 0x07);
    uint32_t forceZones  = 0;
    uint16_t activeZones = 0;

    for (int i = start_pos; i < 10; i++)
    {
        forceZones  |= (uint32_t)(forceValue << (3 * i));
        activeZones |= (uint16_t)(1 << i);
    }

    buffer[0] = (uint8_t)((activeZones >> 0) & 0xff);
    buffer[1] = (uint8_t)((activeZones >> 8) & 0xff);
    buffer[2] = (uint8_t)((forceZones >>  0) & 0xff);
    buffer[3] = (uint8_t)((forceZones >>  8) & 0xff);
    buffer[4] = (uint8_t)((forceZones >> 16) & 0xff);
    buffer[5] = (uint8_t)((forceZones >> 24) & 0xff);
    buffer[6] = 0x00;
    buffer[7] = 0x00;
    buffer[8] = 0x00;
    buffer[9] = 0x00;

    ps5_send_output_report(&output_report);
}

void ps5Controller::setTriggerEffectMultiplePositionFeedback(bool isLeft, uint8_t* strength)
{
  TriggerEffectType type = Feedback;
  uint8_t * buffer = isLeft ? output_report.left_trigger_param : output_report.right_trigger_param;
  
  if(isLeft)
  {
      output_report.left_trigger_motor_mode = type;
      output_report.valid_flag0 |= DS_OUTPUT_VALID_FLAG0_LEFT_TRIGGER_MOTOR_ENABLE;
  }
  else
  { 
      output_report.right_trigger_motor_mode = type;
      output_report.valid_flag0 |= DS_OUTPUT_VALID_FLAG0_RIGHT_TRIGGER_MOTOR_ENABLE;
  }

  uint32_t forceZones  = 0;
  uint16_t activeZones = 0;
  for (int i = 0; i < 10; i++)
  {
      if (strength[i] > 0)
      {
          uint8_t forceValue = (uint8_t)((strength[i] - 1) & 0x07);
          forceZones  |= (uint32_t)(forceValue << (3 * i));
          activeZones |= (uint16_t)(1 << i);
      }
  }

  buffer[0] = (uint8_t)((activeZones >> 0) & 0xff);
  buffer[1] = (uint8_t)((activeZones >> 8) & 0xff);
  buffer[2] = (uint8_t)((forceZones >>  0) & 0xff);
  buffer[3] = (uint8_t)((forceZones >>  8) & 0xff);
  buffer[4] = (uint8_t)((forceZones >> 16) & 0xff);
  buffer[5] = (uint8_t)((forceZones >> 24) & 0xff);
  buffer[6] = 0x00;
  buffer[7] = 0x00;
  buffer[8] = 0x00;
  buffer[9] = 0x00;

  ps5_send_output_report(&output_report);
}

void ps5Controller::setTriggerEffectVibration(bool isLeft, uint8_t start_pos, uint8_t amplitude, uint8_t frequency)
 {
    TriggerEffectType type = Vibration;
    uint8_t * buffer = isLeft ? output_report.left_trigger_param : output_report.right_trigger_param;
    
    if(isLeft)
    {
        output_report.left_trigger_motor_mode = type;
        output_report.valid_flag0 |= DS_OUTPUT_VALID_FLAG0_LEFT_TRIGGER_MOTOR_ENABLE;
    }
    else
    { 
        output_report.right_trigger_motor_mode = type;
        output_report.valid_flag0 |= DS_OUTPUT_VALID_FLAG0_RIGHT_TRIGGER_MOTOR_ENABLE;
    }

    uint8_t strengthValue = (uint8_t)((amplitude - 1) & 0x07);
    uint32_t amplitudeZones = 0;
    uint16_t activeZones    = 0;

    for (int i = start_pos; i < 10; i++)
    {
        amplitudeZones |= (uint32_t)(strengthValue << (3 * i));
        activeZones   |= (uint16_t)(1 << i);
    }

    buffer[0] = (uint8_t)((activeZones    >>  0) & 0xff);
    buffer[1] = (uint8_t)((activeZones    >>  8) & 0xff);
    buffer[2] = (uint8_t)((amplitudeZones >>  0) & 0xff);
    buffer[3] = (uint8_t)((amplitudeZones >>  8) & 0xff);
    buffer[4] = (uint8_t)((amplitudeZones >> 16) & 0xff);
    buffer[5] = (uint8_t)((amplitudeZones >> 24) & 0xff);
    buffer[6] = 0x00;
    buffer[7] = 0x00;
    buffer[8] = frequency;
    buffer[9] = 0x00;

    ps5_send_output_report(&output_report);
 }


void ps5Controller::setTriggerEffectMultiplePositionVibration(bool isLeft, uint8_t frequency, uint8_t* amplitude)
{
  TriggerEffectType type = Vibration;
  uint8_t * buffer = isLeft ? output_report.left_trigger_param : output_report.right_trigger_param;
  
  if(isLeft)
  {
      output_report.left_trigger_motor_mode = type;
      output_report.valid_flag0 |= DS_OUTPUT_VALID_FLAG0_LEFT_TRIGGER_MOTOR_ENABLE;
  }
  else
  { 
      output_report.right_trigger_motor_mode = type;
      output_report.valid_flag0 |= DS_OUTPUT_VALID_FLAG0_RIGHT_TRIGGER_MOTOR_ENABLE;
  }

  uint32_t strengthZones = 0;
  uint16_t activeZones   = 0;
  for (int i = 0; i < 10; i++)
  {
      if (amplitude[i] > 0)
      {
          uint8_t strengthValue = (uint8_t)((amplitude[i] - 1) & 0x07);
          strengthZones |= (uint32_t)(strengthValue << (3 * i));
          activeZones   |= (uint16_t)(1 << i);
      }
  }

  buffer[0] = (uint8_t)((activeZones    >>  0) & 0xff);
  buffer[1] = (uint8_t)((activeZones    >>  8) & 0xff);
  buffer[2] = (uint8_t)((strengthZones >>  0) & 0xff);
  buffer[3] = (uint8_t)((strengthZones >>  8) & 0xff);
  buffer[4] = (uint8_t)((strengthZones >> 16) & 0xff);
  buffer[5] = (uint8_t)((strengthZones >> 24) & 0xff);
  buffer[6] = 0x00;
  buffer[7] = 0x00;
  buffer[8] = frequency;
  buffer[9] = 0x00;

  ps5_send_output_report(&output_report);
}

void ps5Controller::setTriggerEffectSlopeFeedback(bool isLeft, uint8_t start_pos, uint8_t end_pos, uint8_t start_strength, uint8_t end_strength)
{
  uint8_t strength[10] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};

  float slope = 1.0f * (end_strength - start_strength) / (end_pos - start_pos);
  for (int i = (int)start_pos; i < 10; i++)
      if (i <= end_pos)
        strength[i] = (uint8_t)round(start_strength + slope * (i - start_pos));
      else
        strength[i] = end_strength;

  ps5Controller::setTriggerEffectMultiplePositionFeedback(isLeft, strength);
}

void ps5Controller::setLed(uint8_t r, uint8_t g, uint8_t b) {
  output_report.lightbar_red = r;
  output_report.lightbar_green = g;
  output_report.lightbar_blue = b;
  output_report.valid_flag1 = DS5_FLAG1_LIGHTBAR;

  ps5_send_output_report(&output_report);
}

void ps5Controller::setRumble(uint8_t small, uint8_t large) {
  output_report.motor_right = small;
  output_report.motor_left = large;
  output_report.valid_flag0 = DS5_FLAG0_HAPTICS_SELECT | DS5_FLAG0_COMPATIBLE_VIBRATION;
  ps5_send_output_report(&output_report);
}

void ps5Controller::setFlashRate(uint8_t onTime, uint8_t offTime) {
  output.flashOn = onTime / 10;
  output.flashOff = offTime / 10;
}

void ps5Controller::attach(callback_t callback) { _callback_event = callback; }

void ps5Controller::attachOnConnect(callback_t callback) {
  _callback_connect = callback;
}

void ps5Controller::attachOnDisconnect(callback_t callback) {
  _callback_disconnect = callback;
}

void ps5Controller::_event_callback(
  void* object, ps5_t data, ps5_event_t event) {
  ps5Controller* This = (ps5Controller*)object;

  memcpy(&This->data, &data, sizeof(ps5_t));
  memcpy(&This->event, &event, sizeof(ps5_event_t));

  if (This->_callback_event) {
    This->_callback_event();
  }
}

void ps5Controller::_connection_callback(void* object, uint8_t isConnected) {
  ps5Controller* This = (ps5Controller*)object;

  if (isConnected) {
    delay(250);  // ToDo: figure out how to know when the channel is free again
                 // so this delay can be removed

    if (This->_callback_connect) {
      This->_callback_connect();
    }
  }
  else {
    if (This->_callback_disconnect) {
      This->_callback_disconnect();
    }
  }
}

#if !defined(NO_GLOBAL_INSTANCES)
ps5Controller ps5;
#endif
