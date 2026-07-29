#pragma once
#include <Arduino.h>

namespace vimax {

namespace limits {
inline constexpr uint32_t CONTROL_PERIOD_MS = 250U;
inline constexpr uint32_t SENSOR_STALE_MS = 5000U;
}

enum class Process : uint8_t {
  Monitoring=101, Thermostat=102, Power=103, Distillation=104,
  Rectification=109, NBK=112, Beer=116, ValveTest=129
};

enum class Stage : uint8_t {
  Idle=0, HeatUp=1, SelfWork=2, Heads=3, Body=4, Tails=5,
  Cooldown=6, Finished=7, Alarm=8, Placeholder=9
};

enum class Alarm : uint16_t {
  None=0, SensorLost=1, OverTemperature=2, OverPressure=4, Flood=8,
  ZeroCrossLost=16, ManualStop=32, InvalidSettings=64
};

struct Sensors {
  float boilerC=NAN;      // Tkyb
  float columnC=NAN;      // Tn / column control point
  float deflegC=NAN;      // Tdef
  float tsaC=NAN;
  float pressureMmHg=0;
  float currentA=0;
  float mainsV=230;
  uint32_t updatedMs=0;
};

struct Outputs {
  float heaterPct=0;
  float deflegWaterPct=0;
  float headsTakeoffPct=0;
  float bodyTakeoffPct=0;
  bool coolingWater=false;
  bool buzzer=false;
};

struct Settings {
  bool powerStageEnabled=false;
  bool actuatorTestMode=false;

  float installedPowerW=3000;
  float rectWorkPowerW=1800;
  float rectHeatPowerPct=100;

  // Labspirt defaults translated to engineering units.
  float pressureSetMmHg=170;
  float pressureKp=0.18f;
  float pressureKi=0.0015f;
  float pressureMinPowerPct=60;
  float pressureMaxPowerPct=100;

  float deflegSelfC=74.00f;
  float deflegHeadsC=75.84f;
  float deflegBodyC=74.00f;
  float deflegTailsC=77.00f;
  float deflegKp=12.0f;
  float deflegKi=0.20f;
  float deflegMinPct=0;
  float deflegMaxPct=100;

  float vaporDetectedPressureMmHg=100;
  uint32_t selfWorkSec=1800;
  uint32_t headsSec=600;
  float headsTakeoffPct=18;

  float bodyRampStartBoilerC=86.0f;
  float bodyRampEndBoilerC=92.0f;
  float bodyTakeoffStartPct=85;
  float bodyTakeoffEndPct=10;
  float tailsEndBoilerC=98.0f;
  uint32_t cooldownSec=300;

  float pressureAlarmMmHg=900;
  float maxBoilerC=105;
  float maxColumnC=99;
  float maxDeflegC=90;
};

struct Runtime {
  Process process=Process::Monitoring;
  Stage stage=Stage::Idle;
  Alarm alarms=Alarm::None;
  bool running=false;
  bool paused=false;
  uint32_t startedMs=0;
  uint32_t stageStartedMs=0;
  float pressureIntegral=0;
  float deflegIntegral=0;
  float heaterCommandPct=0;
  float deflegCommandPct=0;
  const char* note="Готов";
};

inline Alarm operator|(Alarm a, Alarm b) { return static_cast<Alarm>(static_cast<uint16_t>(a)|static_cast<uint16_t>(b)); }
inline Alarm& operator|=(Alarm& a, Alarm b) { a=a|b; return a; }
inline bool hasAlarm(Alarm a) { return a != Alarm::None; }
inline bool isRectification(Process p) { return p == Process::Rectification; }
}
