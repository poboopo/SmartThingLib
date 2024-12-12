#include "sensors/Sensor.h"

template<>
SensorType Sensor<NUMBER_SENSOR_TYPE>::type() const {
  return OBS_SENSOR;
}

template<>
SensorType Sensor<TEXT_SENSOR_TYPE>::type() const {
  return OBS_STATE;
}