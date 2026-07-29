#include "WebUi.h"
#include "Controller.h"
#include <WiFi.h>
#include <LittleFS.h>

namespace vimax {
WebUi::WebUi(Settings&c,Runtime&r,Sensors&s,Outputs&o,Controller&ctl):cfg_(c),rt_(r),s_(s),o_(o),ctl_(ctl){}
String WebUi::stateJson() const {
  char b[1400];
  snprintf(b,sizeof(b),
    "{\"process\":%u,\"stage\":%u,\"running\":%s,\"paused\":%s,\"alarms\":%u,"
    "\"note\":\"%s\",\"t_kub\":%.2f,\"t_col\":%.2f,\"t_def\":%.2f,\"t_tsa\":%.2f,"
    "\"pressure\":%.1f,\"current\":%.2f,\"heaterPct\":%.1f,\"deflegWaterPct\":%.1f,"
    "\"headsTakeoffPct\":%.1f,\"bodyTakeoffPct\":%.1f,\"powerEnabled\":%s}",
    (unsigned)rt_.process,(unsigned)rt_.stage,rt_.running?"true":"false",rt_.paused?"true":"false",
    (unsigned)rt_.alarms,rt_.note,s_.boilerC,s_.columnC,s_.deflegC,s_.tsaC,s_.pressureMmHg,s_.currentA,
    o_.heaterPct,o_.deflegWaterPct,o_.headsTakeoffPct,o_.bodyTakeoffPct,cfg_.powerStageEnabled?"true":"false");
  return String(b);
}
void WebUi::sendFile(const char*p,const char*type,bool gzip){File f=LittleFS.open(p,"r");if(!f){server_.send(404,"text/plain","not found");return;}if(gzip)server_.sendHeader("Content-Encoding","gzip");server_.streamFile(f,type);f.close();}
void WebUi::begin(){
  LittleFS.begin(true); WiFi.mode(WIFI_AP_STA); WiFi.softAP("VIMAX-PURE","vimaxpure");
  server_.on("/",HTTP_GET,[this](){if(LittleFS.exists("/index.html.gz"))sendFile("/index.html.gz","text/html",true);else sendFile("/index.html","text/html");});
  server_.on("/api/state",HTTP_GET,[this](){server_.send(200,"application/json",stateJson());});
  server_.on("/api/start",HTTP_POST,[this](){
    Process p=(Process)server_.arg("process").toInt(); bool ok=ctl_.start(p,cfg_,rt_);
    server_.send(ok?200:409,"application/json",stateJson());
  });
  server_.on("/api/next",HTTP_POST,[this](){ctl_.nextStage(rt_);server_.send(200,"application/json",stateJson());});
  server_.on("/api/pause",HTTP_POST,[this](){rt_.paused=server_.arg("enabled")=="1";server_.send(200,"application/json",stateJson());});
  server_.on("/api/stop",HTTP_POST,[this](){ctl_.stop(rt_);server_.send(200,"application/json",stateJson());});
  server_.on("/api/power-arm",HTTP_POST,[this](){cfg_.powerStageEnabled=server_.arg("enabled")=="1";server_.send(200,"application/json",stateJson());});
  server_.on("/api/settings",HTTP_POST,[this](){
    auto f=[this](const char*n,float&v){if(server_.hasArg(n))v=server_.arg(n).toFloat();};
    auto u=[this](const char*n,uint32_t&v){if(server_.hasArg(n))v=(uint32_t)server_.arg(n).toInt();};
    f("installedPowerW",cfg_.installedPowerW);f("rectWorkPowerW",cfg_.rectWorkPowerW);
    f("pressureSetMmHg",cfg_.pressureSetMmHg);f("deflegSelfC",cfg_.deflegSelfC);f("deflegHeadsC",cfg_.deflegHeadsC);
    f("deflegBodyC",cfg_.deflegBodyC);f("deflegTailsC",cfg_.deflegTailsC);f("headsTakeoffPct",cfg_.headsTakeoffPct);
    f("bodyRampStartBoilerC",cfg_.bodyRampStartBoilerC);f("bodyRampEndBoilerC",cfg_.bodyRampEndBoilerC);
    f("bodyTakeoffStartPct",cfg_.bodyTakeoffStartPct);f("bodyTakeoffEndPct",cfg_.bodyTakeoffEndPct);
    f("tailsEndBoilerC",cfg_.tailsEndBoilerC);u("selfWorkSec",cfg_.selfWorkSec);u("headsSec",cfg_.headsSec);u("cooldownSec",cfg_.cooldownSec);
    server_.send(200,"application/json",stateJson());
  });
  server_.begin();
}
void WebUi::loop(){server_.handleClient();}
}
