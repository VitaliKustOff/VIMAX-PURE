#pragma once
#include "Model.h"
#include <DallasTemperature.h>
#include <OneWire.h>

namespace vimax {
class Hardware {
 public:
  Hardware();
  void begin();
  void read(Sensors& s);
  void apply(const Outputs& o, bool allowPower, bool allowActuators);
  void allOff();
 private:
  bool burst(float pct,uint32_t windowMs,uint32_t offset=0) const;
  OneWire oneWire_;
  DallasTemperature ds_;
  uint32_t lastTempRequest_=0;
  float temps_[4]={NAN,NAN,NAN,NAN};
};
}
