#pragma once

#include "Hardware.h"
#include "Controller.h"
#include "WebUi.h"
#include "BoardConfig.h"

namespace vimax {

class App {
public:
  App();

  void begin();
  void loop();

private:
  Hardware hardware_;
  Controller controller_;
  Settings settings_;
  Runtime runtime_;
  Sensors sensors_;
  Outputs outputs_;
  WebUi webUi_;

  uint32_t lastControlMs_ = 0;
};

}