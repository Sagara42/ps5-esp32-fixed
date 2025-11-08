#ifndef ps5_H
#define ps5_H

#include <stdbool.h>
#include <stdint.h>

#define DS5_FLAG0_COMPATIBLE_VIBRATION BIT(0)
#define DS5_FLAG0_HAPTICS_SELECT BIT(1)
#define DS5_FLAG1_LIGHTBAR BIT(2)
#define DS5_FLAG1_PLAYER_LED BIT(4)
#define DS5_LIGHTBAR_SETUP_LIGHT_OUT BIT(1)
#define DS5_FLAG2_LIGHTBAR_SETUP_CONTROL_ENABLE BIT(1)
#define DS_OUTPUT_VALID_FLAG0_RIGHT_TRIGGER_MOTOR_ENABLE BIT(2)
#define DS_OUTPUT_VALID_FLAG0_LEFT_TRIGGER_MOTOR_ENABLE BIT(3)
#define DS_OUTPUT_VALID_FLAG0_HEADPHONE_VOLUME_ENABLE BIT(4)

enum TriggerEffectType {
      Off       = 0x05, // 00 00 0 101
      Feedback  = 0x21, // 00 10 0 001
      Weapon    = 0x25, // 00 10 0 101
      Vibration = 0x26, // 00 10 0 110

      // Unofficial but unique effects left in the firmware
      // These might be removed in the future
      Bow       = 0x22, // 00 10 0 010
      Galloping = 0x23, // 00 10 0 011
      Machine   = 0x27, // 00 10 0 111

      // Leftover versions of offical modes with simpler logic and no paramater protections
      // These should not be used
      Simple_Feedback  = 0x01, // 00 00 0 001
      Simple_Weapon    = 0x02, // 00 00 0 010
      Simple_Vibration = 0x06, // 00 00 0 110

      // Leftover versions of offical modes with limited paramater ranges
      // These should not be used
      Limited_Feedback = 0x11, // 00 01 0 001
      Limited_Weapon   = 0x12, // 00 01 0 010

      // Debug or Calibration functions
      // Don't use these as they will courrupt the trigger state until the reset button is pressed
      DebugFC = 0xFC, // 11 11 1 100
      DebugFD = 0xFD, // 11 11 1 101
      DebugFE = 0xFE, // 11 11 1 110
};

/********************************************************************************/
/*                                  T Y P E S */
/********************************************************************************/

typedef struct __attribute((packed))
{
    uint8_t transaction_type;
    uint8_t report_id; /* 0x31 */
    uint8_t seq_tag;
    uint8_t tag;

    // Common to Bluetooth and USB (although we don't support USB).
    uint8_t valid_flag0;
    uint8_t valid_flag1;

    /* For DualShock 4 compatibility mode. */
    uint8_t motor_right;
    uint8_t motor_left;

    /* Audio controls */
    uint8_t reserved1[4];
    uint8_t mute_button_led; // 10

    uint8_t power_save_control;
    uint8_t right_trigger_motor_mode;
    uint8_t right_trigger_param[10];

    /* right trigger motor */
    uint8_t left_trigger_motor_mode;
    uint8_t left_trigger_param[10];
    uint8_t reserved2[6];

    /* LEDs and lightbar */
    uint8_t valid_flag2; // 40
    uint8_t reserved3[2];
    uint8_t lightbar_setup;
    uint8_t led_brightness; // 44
    uint8_t player_leds;
    uint8_t lightbar_red; // 46
    uint8_t lightbar_green;
    uint8_t lightbar_blue;

    //
    uint8_t reserved4[24]; // 49
    uint32_t crc32;
} ps5_output_report_t;

/********************/
/*    A N A L O G   */
/********************/

typedef struct {
  int8_t lx;
  int8_t ly;
  int8_t rx;
  int8_t ry;
} ps5_analog_stick_t;

typedef struct {
  uint8_t l2;
  uint8_t r2;
} ps5_analog_button_t;

typedef struct {
  ps5_analog_stick_t stick;
  ps5_analog_button_t button;
} ps5_analog_t;

/*********************/
/*   B U T T O N S   */
/*********************/

typedef struct {
  uint8_t right : 1;
  uint8_t down : 1;
  uint8_t up : 1;
  uint8_t left : 1;

  uint8_t square : 1;
  uint8_t cross : 1;
  uint8_t circle : 1;
  uint8_t triangle : 1;

  uint8_t upright : 1;
  uint8_t downright : 1;
  uint8_t upleft : 1;
  uint8_t downleft : 1;

  uint8_t l1 : 1;
  uint8_t r1 : 1;
  uint8_t l2 : 1;
  uint8_t r2 : 1;

  uint8_t share : 1;
  uint8_t options : 1;
  uint8_t l3 : 1;
  uint8_t r3 : 1;

  uint8_t ps : 1;
  uint8_t touchpad : 1;
} ps5_button_t;

/*******************************/
/*   S T A T U S   F L A G S   */
/*******************************/

typedef struct {
  uint8_t battery;
  uint8_t charging : 1;
  uint8_t audio : 1;
  uint8_t mic : 1;
} ps5_status_t;

/********************/
/*   S E N S O R S  */
/********************/

typedef struct {
  int16_t z;
} ps5_sensor_gyroscope_t;

typedef struct {
  int16_t x;
  int16_t y;
  int16_t z;
} ps5_sensor_accelerometer_t;

typedef struct {
  ps5_sensor_accelerometer_t accelerometer;
  ps5_sensor_gyroscope_t gyroscope;
} ps5_sensor_t;

/*******************/
/*    O T H E R    */
/*******************/

typedef struct {
  uint8_t smallRumble;
  uint8_t largeRumble;
  uint8_t r, g, b;
  uint8_t flashOn;
  uint8_t flashOff;  // Time to flash bright/dark (255 = 2.5 seconds)
} ps5_cmd_t;

typedef struct {
  ps5_button_t button_down;
  ps5_button_t button_up;
  ps5_analog_t analog_move;
} ps5_event_t;

typedef struct {
  ps5_analog_t analog;
  ps5_button_t button;
  ps5_status_t status;
  ps5_sensor_t sensor;
  uint8_t* latestPacket;
} ps5_t;

/***************************/
/*    C A L L B A C K S    */
/***************************/

typedef void (*ps5_connection_callback_t)(uint8_t isConnected);
typedef void (*ps5_connection_object_callback_t)(void* object, uint8_t isConnected);

typedef void (*ps5_event_callback_t)(ps5_t ps5, ps5_event_t event);
typedef void (*ps5_event_object_callback_t)(void* object, ps5_t ps5, ps5_event_t event);

void ps5_send_output_report(ps5_output_report_t *out);

/********************************************************************************/
/*                             F U N C T I O N S */
/********************************************************************************/

bool ps5IsConnected();
void ps5Init();
void ps5Enable();
void ps5Cmd(ps5_cmd_t ps5_cmd);
void ps5SetConnectionCallback(ps5_connection_callback_t cb);
void ps5SetConnectionObjectCallback(void* object, ps5_connection_object_callback_t cb);
void ps5SetEventCallback(ps5_event_callback_t cb);
void ps5SetEventObjectCallback(void* object, ps5_event_object_callback_t cb);
void ps5SetBluetoothMacAddress(const uint8_t* mac);
long ps5_l2cap_connect(uint8_t addr[6]);
long ps5_l2cap_reconnect(void);

#endif
