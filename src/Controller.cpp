#include "Controller.h"
#include "BoardConfig.h"
#include <math.h>

namespace vimax {
namespace {
float lerpClamped(float x0,float y0,float x1,float y1,float x){
  if(x1<=x0) return y1;
  const float k=constrain((x-x0)/(x1-x0),0.0f,1.0f);
  return y0+(y1-y0)*k;
}
}

void Controller::begin() {}

bool Controller::settingsValid(const Settings& c) const {
  return c.installedPowerW>0 && c.rectWorkPowerW>0 &&
    c.bodyRampEndBoilerC>c.bodyRampStartBoilerC &&
    c.tailsEndBoilerC>c.bodyRampEndBoilerC &&
    c.pressureMaxPowerPct>=c.pressureMinPowerPct &&
    c.deflegMaxPct>=c.deflegMinPct;
}

void Controller::setStage(Stage s, Runtime& rt){
  rt.stage=s; rt.stageStartedMs=millis(); rt.pressureIntegral=0; rt.deflegIntegral=0;
  switch(s){
    case Stage::HeatUp: rt.note="Ректификация: разгон"; break;
    case Stage::SelfWork: rt.note="Работа колонны на себя"; break;
    case Stage::Heads: rt.note="Отбор голов"; break;
    case Stage::Body: rt.note="Отбор тела"; break;
    case Stage::Tails: rt.note="Отбор хвостов"; break;
    case Stage::Cooldown: rt.note="Охлаждение"; break;
    case Stage::Finished: rt.note="Процесс завершён"; break;
    case Stage::Placeholder: rt.note="Модуль пока является заглушкой"; break;
    case Stage::Alarm: rt.note="Аварийная остановка"; break;
    default: rt.note="Готов"; break;
  }
}

bool Controller::start(Process p,const Settings& cfg,Runtime& rt){
  rt.process=p; rt.alarms=Alarm::None; rt.startedMs=millis(); rt.paused=false;
  if(!isRectification(p)){
    rt.running=false; setStage(Stage::Placeholder,rt); return false;
  }
  if(!settingsValid(cfg)){
    rt.running=false; rt.alarms|=Alarm::InvalidSettings; setStage(Stage::Alarm,rt); return false;
  }
  rt.running=true; setStage(Stage::HeatUp,rt); return true;
}

void Controller::stop(Runtime& rt,Alarm reason){
  rt.running=false;
  if(reason!=Alarm::None) rt.alarms|=reason;
  setStage(hasAlarm(rt.alarms)?Stage::Alarm:Stage::Idle,rt);
}

void Controller::nextStage(Runtime& rt){
  if(!rt.running || !isRectification(rt.process)) return;
  switch(rt.stage){
    case Stage::HeatUp:setStage(Stage::SelfWork,rt);break;
    case Stage::SelfWork:setStage(Stage::Heads,rt);break;
    case Stage::Heads:setStage(Stage::Body,rt);break;
    case Stage::Body:setStage(Stage::Tails,rt);break;
    case Stage::Tails:setStage(Stage::Cooldown,rt);break;
    case Stage::Cooldown:setStage(Stage::Finished,rt);break;
    default:break;
  }
}

void Controller::safety(const Sensors&s,const Settings&c,Runtime&rt,Outputs&out){
  Alarm a=Alarm::None;
  const bool stale=millis()-s.updatedMs>limits::SENSOR_STALE_MS;
  if(stale || isnan(s.boilerC) || isnan(s.deflegC)) a|=Alarm::SensorLost;
  if((!isnan(s.boilerC)&&s.boilerC>c.maxBoilerC)||(!isnan(s.columnC)&&s.columnC>c.maxColumnC)||(!isnan(s.deflegC)&&s.deflegC>c.maxDeflegC)) a|=Alarm::OverTemperature;
  if(s.pressureMmHg>c.pressureAlarmMmHg) a|=Alarm::OverPressure;
  if(digitalRead(pins::FLOOD)==LOW) a|=Alarm::Flood;
  if(hasAlarm(a)){
    rt.alarms|=a; rt.running=false; setStage(Stage::Alarm,rt);
    out={}; out.buzzer=true;
  }
}

float Controller::pressurePower(const Sensors&s,const Settings&c,Runtime&rt){
  const float base=constrain(100.0f*c.rectWorkPowerW/c.installedPowerW,c.pressureMinPowerPct,c.pressureMaxPowerPct);
  const float error=c.pressureSetMmHg-s.pressureMmHg;
  rt.pressureIntegral=constrain(rt.pressureIntegral+error*c.pressureKi,c.pressureMinPowerPct-base,c.pressureMaxPowerPct-base);
  rt.heaterCommandPct=constrain(base+error*c.pressureKp+rt.pressureIntegral,c.pressureMinPowerPct,c.pressureMaxPowerPct);
  return rt.heaterCommandPct;
}

float Controller::deflegControl(float setpoint,const Sensors&s,const Settings&c,Runtime&rt){
  if(isnan(s.deflegC)) return 100;
  // More temperature => more cooling water.
  const float error=s.deflegC-setpoint;
  rt.deflegIntegral=constrain(rt.deflegIntegral+error*c.deflegKi,c.deflegMinPct,c.deflegMaxPct);
  rt.deflegCommandPct=constrain(error*c.deflegKp+rt.deflegIntegral,c.deflegMinPct,c.deflegMaxPct);
  return rt.deflegCommandPct;
}

void Controller::runRectification(const Sensors&s,const Settings&c,Runtime&rt,Outputs&o){
  o.coolingWater=true;
  const uint32_t elapsed=(millis()-rt.stageStartedMs)/1000UL;
  switch(rt.stage){
    case Stage::HeatUp:
      o.heaterPct=c.rectHeatPowerPct;
      o.deflegWaterPct=0;
      if(s.pressureMmHg>=c.vaporDetectedPressureMmHg) setStage(Stage::SelfWork,rt);
      break;
    case Stage::SelfWork:
      o.heaterPct=pressurePower(s,c,rt);
      o.deflegWaterPct=deflegControl(c.deflegSelfC,s,c,rt);
      if(elapsed>=c.selfWorkSec) setStage(Stage::Heads,rt);
      break;
    case Stage::Heads:
      o.heaterPct=pressurePower(s,c,rt);
      o.deflegWaterPct=deflegControl(c.deflegHeadsC,s,c,rt);
      o.headsTakeoffPct=c.headsTakeoffPct;
      if(elapsed>=c.headsSec) setStage(Stage::Body,rt);
      break;
    case Stage::Body:
      o.heaterPct=pressurePower(s,c,rt);
      o.deflegWaterPct=deflegControl(c.deflegBodyC,s,c,rt);
      o.bodyTakeoffPct=lerpClamped(c.bodyRampStartBoilerC,c.bodyTakeoffStartPct,c.bodyRampEndBoilerC,c.bodyTakeoffEndPct,s.boilerC);
      if(s.boilerC>=c.bodyRampEndBoilerC) setStage(Stage::Tails,rt);
      break;
    case Stage::Tails:
      o.heaterPct=pressurePower(s,c,rt);
      o.deflegWaterPct=deflegControl(c.deflegTailsC,s,c,rt);
      o.bodyTakeoffPct=0;
      if(s.boilerC>=c.tailsEndBoilerC) setStage(Stage::Cooldown,rt);
      break;
    case Stage::Cooldown:
      o={}; o.coolingWater=true; o.deflegWaterPct=100;
      if(elapsed>=c.cooldownSec) setStage(Stage::Finished,rt);
      break;
    case Stage::Finished:
      o={}; rt.running=false;
      break;
    default: o={}; break;
  }
}

void Controller::tick(const Sensors&s,Settings&c,Runtime&rt,Outputs&out){
  out={};
  if(!rt.running){
    if(rt.stage==Stage::Alarm) out.buzzer=true;
    return;
  }
  if(!isRectification(rt.process)){
    rt.running=false; setStage(Stage::Placeholder,rt); return;
  }
  if(!rt.paused) runRectification(s,c,rt,out);
  safety(s,c,rt,out);
}
}
