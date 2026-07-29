#include "App.h"

#include <Arduino.h>

namespace vimax {

App::App()
  : webUi_(
      settings_,
      runtime_,
      sensors_,
      outputs_,
      controller_
    ) {
}

void App::begin() {
  Serial.begin(115200);
  delay(300);

  Serial.println();
  Serial.println("VIMAX.PURE");
  Serial.println("Foundation v0.5");
  Serial.println("Rectification prototype compatibility mode");

  hardware_.begin();
  controller_.begin();
  webUi_.begin();

  Serial.println("Power stage: DISARMED");
}

void App::loop() {
  const uint32_t now = millis();

  hardware_.read(sensors_);

  if (now - lastControlMs_ >= limits::CONTROL_PERIOD_MS) {
    lastControlMs_ = now;

    controller_.tick(
      sensors_,
      settings_,
      runtime_,
      outputs_
    );

    const bool outputsAllowed =
      settings_.powerStageEnabled &&
      !hasAlarm(runtime_.alarms);

    hardware_.apply(
      outputs_,
      outputsAllowed,
      outputsAllowed
    );
  }

  webUi_.loop();

  delay(2);
}

}