#pragma once
#include <WebServer.h>
#include "Model.h"

namespace vimax {
class Controller;
class WebUi {
 public:
  WebUi(Settings& cfg, Runtime& rt, Sensors& sensors, Outputs& outputs, Controller& controller);
  void begin();
  void loop();
 private:
  String stateJson() const;
  void sendFile(const char* path, const char* contentType, bool gzip=false);
  WebServer server_{80};
  Settings& cfg_; Runtime& rt_; Sensors& s_; Outputs& o_; Controller& ctl_;
};
}
