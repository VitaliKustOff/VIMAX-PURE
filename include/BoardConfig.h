#pragma once
#include <Arduino.h>

namespace vimax::pins {
// Safe defaults for ESP32-S3 DevKitC. Verify against your actual carrier board.
constexpr gpio_num_t HEATER_GATE = GPIO_NUM_4;   // SSR/triac driver input, active HIGH
constexpr gpio_num_t ZERO_CROSS  = GPIO_NUM_5;   // isolated zero-cross input
constexpr gpio_num_t VALVE_HEADS = GPIO_NUM_6;
constexpr gpio_num_t VALVE_BODY  = GPIO_NUM_7;
constexpr gpio_num_t VALVE_WATER = GPIO_NUM_15;
constexpr gpio_num_t BUZZER      = GPIO_NUM_16;
constexpr gpio_num_t FLOOD       = GPIO_NUM_17;  // active LOW
constexpr gpio_num_t ONEWIRE     = GPIO_NUM_18;
constexpr gpio_num_t ENC_A       = GPIO_NUM_8;
constexpr gpio_num_t ENC_B       = GPIO_NUM_9;
constexpr gpio_num_t ENC_KEY     = GPIO_NUM_10;
constexpr gpio_num_t PRESSURE_ADC= GPIO_NUM_1;
constexpr gpio_num_t CURRENT_ADC = GPIO_NUM_2;
}

namespace vimax::limits {
constexpr float MAX_BOILER_C = 110.0f;
constexpr float MAX_COLUMN_C = 100.0f;
constexpr float MAX_PRESSURE_MMHG = 1200.0f;
constexpr uint32_t SENSOR_STALE_MS = 8000;
constexpr uint32_t CONTROL_PERIOD_MS = 250;
constexpr uint32_t TELEMETRY_PERIOD_MS = 1000;
}
