#include "Hardware.h"
#include "BoardConfig.h"

namespace vimax {

Hardware::Hardware()
  : oneWire_(pins::ONEWIRE),
    ds_(&oneWire_) {
}

bool Hardware::burst(float pct, uint32_t windowMs, uint32_t offset) const {
  pct = constrain(pct, 0.0f, 100.0f);

  if (windowMs == 0U) {
    return false;
  }

  const uint32_t position = (millis() + offset) % windowMs;
  const uint32_t onTime =
    static_cast<uint32_t>(windowMs * pct / 100.0f);

  return position < onTime;
}

void Hardware::begin() {
  pinMode(pins::HEATER_GATE, OUTPUT);
  pinMode(pins::VALVE_HEADS, OUTPUT);
  pinMode(pins::VALVE_BODY, OUTPUT);
  pinMode(pins::VALVE_WATER, OUTPUT);
  pinMode(pins::BUZZER, OUTPUT);

  pinMode(pins::FLOOD, INPUT_PULLUP);
  pinMode(pins::ZERO_CROSS, INPUT_PULLUP);

  analogReadResolution(12);
  allOff();

  ds_.begin();
  ds_.setWaitForConversion(false);
  ds_.requestTemperatures();
  lastTempRequest_ = millis();
}

void Hardware::read(Sensors& s) {
  const uint32_t now = millis();

  if (now - lastTempRequest_ >= 800U) {
    const uint8_t deviceCount = ds_.getDeviceCount();
    const uint8_t count = deviceCount < 4U ? deviceCount : 4U;

    for (uint8_t i = 0; i < count; ++i) {
      const float temperature = ds_.getTempCByIndex(i);
      temps_[i] =
        temperature == DEVICE_DISCONNECTED_C
          ? NAN
          : temperature;
    }

    for (uint8_t i = count; i < 4U; ++i) {
      temps_[i] = NAN;
    }

    ds_.requestTemperatures();
    lastTempRequest_ = now;

    // Временная привязка датчиков по порядку обнаружения.
    // Перед реальной эксплуатацией заменить на ROM-адреса DS18B20.
    s.boilerC = temps_[0];
    s.columnC = temps_[1];
    s.deflegC = temps_[2];
    s.tsaC = temps_[3];
    s.updatedMs = now;
  }

  const int pressureRaw = analogRead(pins::PRESSURE_ADC);
  const int currentRaw = analogRead(pins::CURRENT_ADC);

  s.pressureMmHg =
    static_cast<float>(pressureRaw) *
    (1200.0f / 4095.0f);

  s.currentA =
    static_cast<float>(currentRaw) *
    (30.0f / 4095.0f);
}

void Hardware::apply(
  const Outputs& o,
  bool allowPower,
  bool allowActuators
) {
  const bool heaterState =
    allowPower &&
    burst(o.heaterPct, 1000U);

  const bool headsState =
    allowActuators &&
    burst(o.headsTakeoffPct, 2000U, 200U);

  const bool bodyState =
    allowActuators &&
    burst(o.bodyTakeoffPct, 2000U, 700U);

  const bool waterState =
    allowActuators &&
    o.coolingWater &&
    burst(o.deflegWaterPct, 1000U, 400U);

  digitalWrite(pins::HEATER_GATE, heaterState ? HIGH : LOW);
  digitalWrite(pins::VALVE_HEADS, headsState ? HIGH : LOW);
  digitalWrite(pins::VALVE_BODY, bodyState ? HIGH : LOW);
  digitalWrite(pins::VALVE_WATER, waterState ? HIGH : LOW);
  digitalWrite(pins::BUZZER, o.buzzer ? HIGH : LOW);
}

void Hardware::allOff() {
  digitalWrite(pins::HEATER_GATE, LOW);
  digitalWrite(pins::VALVE_HEADS, LOW);
  digitalWrite(pins::VALVE_BODY, LOW);
  digitalWrite(pins::VALVE_WATER, LOW);
  digitalWrite(pins::BUZZER, LOW);
}

}  // namespace vimax
