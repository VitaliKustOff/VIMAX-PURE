#include <Arduino.h>

#include "Hardware.h"
#include "Controller.h"
#include "WebUi.h"

using namespace vimax;

Hardware hw;
Controller ctl;
Settings cfg;
Runtime rt;
Sensors sensors;
Outputs outputs;
WebUi web(cfg, rt, sensors, outputs, ctl);

void setup() {
  Serial.begin(115200);
  delay(300);

  Serial.println();
  Serial.println("VIMAX.PURE ESP32-S3 N16R8 v0.4.1 RECTIFICATION");

  hw.begin();
  ctl.begin();
  web.begin();

  Serial.println("AP: VIMAX-PURE");
  Serial.println("Password: vimaxpure");
  Serial.println("Power output: DISARMED");
}

void loop() {
  static uint32_t lastControlMs = 0;
  const uint32_t now = millis();

  hw.read(sensors);

  if (now - lastControlMs >= limits::CONTROL_PERIOD_MS) {
    lastControlMs = now;

    ctl.tick(sensors, cfg, rt, outputs);

    const bool outputsAllowed =
      cfg.powerStageEnabled &&
      !hasAlarm(rt.alarms);

    hw.apply(outputs, outputsAllowed, outputsAllowed);
  }

  web.loop();
  delay(2);
}
