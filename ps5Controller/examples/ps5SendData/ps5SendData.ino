#include <ps5Controller.h>

int r = 255;
int g = 0;
int b = 0;

// Calculates the next value in a rainbow sequence
void nextRainbowColor() {
  if (r > 0 && b == 0) {
    r--;
    g++;
  }
  if (g > 0 && r == 0) {
    g--;
    b++;
  }
  if (b > 0 && g == 0) {
    r++;
    b--;
  }
}

void setup() {
  Serial.begin(115200);

  // Replace the "1a:2b:3c:01:01:01" with the MAC address
  // the controller wants to pair to
  // Note: MAC address must be unicast
  ps5.begin("1a:2b:3c:01:01:01");
  Serial.println("Ready.");
}

void loop() {
  if (ps5.isConnected()) 
  {
     ps5.setLed(r, g, b);
     nextRainbowColor();

    if (ps5.Left())
    {
      ps5.setTriggerEffectOff(true, false);
      Serial.println("Left.");
    }

    if (ps5.Right())
    {
       ps5.setTriggerEffectWeapon(true, 3, 8, 8);
       Serial.println("Right.");
    }

    if(ps5.Up()){
      ps5.setTriggerEffectFeedback(true, 3, 8);
      Serial.println("Up.");
    }

    if(ps5.Down()){
      ps5.setTriggerEffectVibration(true, 5, 8, 25);
      Serial.println("Down.");
    }

    if (ps5.Square()) Serial.println("Square Button");
    if (ps5.Cross()) Serial.println("Cross Button");
    if (ps5.Circle()) Serial.println("Circle Button");
    if (ps5.Triangle()) Serial.println("Triangle Button");

    if (ps5.UpRight()) Serial.println("Up Right");
    if (ps5.DownRight()) Serial.println("Down Right");
    if (ps5.UpLeft()) Serial.println("Up Left");
    if (ps5.DownLeft()) Serial.println("Down Left");

    if (ps5.L1()) Serial.println("L1 Button");
    if (ps5.R1()) Serial.println("R1 Button");

    if (ps5.Share()) Serial.println("Share Button");
    if (ps5.Options()) Serial.println("Options Button");
    if (ps5.L3()) Serial.println("L3 Button");
    if (ps5.R3()) Serial.println("R3 Button");

    // // Sets the rumble of the controllers
    // // Params: Weak rumble intensity, Strong rumble intensity
    // // Range: 0->255
    ps5.setRumble(ps5.L2Value(), ps5.R2Value());

    // Don't send data to the controller immediately, will cause buffer overflow
    delay(10);
  }
}
