#include "observable/ObservableObject.h"

template<>
ObservableType ObservableObject<NUMBER_SENSOR_TYPE>::type() const {
  return OBS_SENSOR;
}

template<>
ObservableType ObservableObject<TEXT_SENSOR_TYPE>::type() const {
  return OBS_STATE;
}