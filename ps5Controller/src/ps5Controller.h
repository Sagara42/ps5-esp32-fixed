#ifndef ps5Controller_h
#define ps5Controller_h

#include "Arduino.h"

extern "C" {
#include "ps5.h"
}

class ps5Controller {
 public:
  typedef void (*callback_t)();

  ps5_t data;
  ps5_event_t event;
  ps5_cmd_t output;
  ps5_output_report_t output_report;

  ps5Controller();

  bool begin();
  bool begin(const char* mac);
  void end();

  bool isConnected();
  void ps5_disable_lightbar();
  
  /// @brief Disable effect
  /// @param isLeft for left trigger
  /// @param isRight for right trigger
  void setTriggerEffectOff(bool isLeft, bool isRight);
  /// @brief Simulate weapon trigger
  /// @param isLeft true for left, false for right
  /// @param start_pos The starting zone of the trigger effect. Must be between 2 and 7 inclusive.
  /// @param end_pos The ending zone of the trigger effect. Must be between start_pos+1 and 8 inclusive.
  /// @param strength The force of the resistance. Must be between 0 and 8 inclusive.
  void setTriggerEffectWeapon(bool isLeft, uint8_t start_pos, uint8_t end_pos, uint8_t strength);
  /// @brief Trigger will resist movement beyond the start position.
  /// @param isLeft true for left, false for right
  /// @param start_pos The starting zone of the trigger effect. Must be between 0 and 9 inclusive.
  /// @param strength The force of the resistance. Must be between 0 and 8 inclusive.
  void setTriggerEffectFeedback(bool isLeft, uint8_t start_pos, uint8_t strength);
  /// @brief Trigger will resist movement at varrying strengths in 10 regions.
  /// @param isLeft true for left, false for right
  /// @param strength Array of 10 resistance values for zones 0 through 9. Must be between 0 and 8 inclusive.
  void setTriggerEffectMultiplePositionFeedback(bool isLeft, uint8_t* strength);
  /// @brief Trigger will vibrate with the input amplitude and frequency beyond the start position.
  /// @param isLeft true for left, false for right
  /// @param start_pos The starting zone of the trigger effect. Must be between 0 and 9 inclusive.
  /// @param amplitude Strength of the automatic cycling action. Must be between 0 and 8 inclusive.
  /// @param frequency Frequency of the automatic cycling action in hertz.
  void setTriggerEffectVibration(bool isLeft, uint8_t start_pos, uint8_t amplitude, uint8_t frequency);
  /// @brief Trigger will vibrate movement at varrying amplitudes and one frequency in 10 regions.
  /// @param isLeft true for left, false for right
  /// @param frequency Frequency of the automatic cycling action in hertz.
  /// @param amplitude Array of 10 strength values for zones 0 through 9. Must be between 0 and 8 inclusive
  void setTriggerEffectMultiplePositionVibration(bool isLeft, uint8_t frequency, uint8_t* amplitude);
  /// @brief Trigger will resist movement at a linear range of strengths.
  /// @param isLeft true for left, false for right
  /// @param start_pos The starting zone of the trigger effect. Must be between 0 and 8 inclusive.
  /// @param end_pos The ending zone of the trigger effect. Must be between start_pos+1 and 9 inclusive.
  /// @param start_strength The force of the resistance at the start. Must be between 1 and 8 inclusive.
  /// @param end_strength The force of the resistance at the end. Must be between 1 and 8 inclusive.
  void setTriggerEffectSlopeFeedback(bool isLeft, uint8_t start_pos, uint8_t end_pos, uint8_t start_strength, uint8_t end_strength);

  void setLed(uint8_t r, uint8_t g, uint8_t b);
  void setRumble(uint8_t small, uint8_t large);
  void setFlashRate(uint8_t onTime, uint8_t offTime);

  void attach(callback_t callback);
  void attachOnConnect(callback_t callback);
  void attachOnDisconnect(callback_t callback);

  uint8_t* LatestPacket() { return data.latestPacket; }

public:
  bool Right() { return data.button.right; }
  bool Down() { return data.button.down; }
  bool Up() { return data.button.up; }
  bool Left() { return data.button.left; }

  bool Square() { return data.button.square; }
  bool Cross() { return data.button.cross; }
  bool Circle() { return data.button.circle; }
  bool Triangle() { return data.button.triangle; }

  bool UpRight() { return data.button.upright; }
  bool DownRight() { return data.button.downright; }
  bool UpLeft() { return data.button.upleft; }
  bool DownLeft() { return data.button.downleft; }

  bool L1() { return data.button.l1; }
  bool R1() { return data.button.r1; }
  bool L2() { return data.button.l2; }
  bool R2() { return data.button.r2; }

  bool Share() { return data.button.share; }
  bool Options() { return data.button.options; }
  bool L3() { return data.button.l3; }
  bool R3() { return data.button.r3; }

  bool PSButton() { return data.button.ps; }
  bool Touchpad() { return data.button.touchpad; }

  uint8_t L2Value() { return data.analog.button.l2; }
  uint8_t R2Value() { return data.analog.button.r2; }

  int8_t LStickX() { return data.analog.stick.lx; }
  int8_t LStickY() { return data.analog.stick.ly; }
  int8_t RStickX() { return data.analog.stick.rx; }
  int8_t RStickY() { return data.analog.stick.ry; }

  uint8_t Battery() { return data.status.battery; }
  bool Charging() { return data.status.charging; }
  bool Audio() { return data.status.audio; }
  bool Mic() { return data.status.mic; }

 private:
  static void _event_callback(void* object, ps5_t data, ps5_event_t event);
  static void _connection_callback(void* object, uint8_t isConnected);

  callback_t _callback_event = nullptr;
  callback_t _callback_connect = nullptr;
  callback_t _callback_disconnect = nullptr;
};

#ifndef NO_GLOBAL_INSTANCES
extern ps5Controller ps5;
#endif

#endif
