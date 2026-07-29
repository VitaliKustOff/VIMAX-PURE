#pragma once
#include "Model.h"

namespace vimax {
class Controller {
 public:
  void begin();
  void tick(const Sensors& s, Settings& cfg, Runtime& rt, Outputs& out);
  bool start(Process p, const Settings& cfg, Runtime& rt);
  void stop(Runtime& rt, Alarm reason=Alarm::ManualStop);
  void nextStage(Runtime& rt);
 private:
  void setStage(Stage s, Runtime& rt);
  void safety(const Sensors& s, const Settings& cfg, Runtime& rt, Outputs& out);
  void runRectification(const Sensors& s, const Settings& cfg, Runtime& rt, Outputs& out);
  float pressurePower(const Sensors& s, const Settings& cfg, Runtime& rt);
  float deflegControl(float setpoint, const Sensors& s, const Settings& cfg, Runtime& rt);
  bool settingsValid(const Settings& cfg) const;
};
}
