#ifndef DEVICE_STATES_LIST_H
#define DEVICE_STATES_LIST_H

#include "Features.h"
#if ENABLE_STATES

#include <Arduino.h>
#include <ArduinoJson.h>

#include <functional>

#include "logs/BetterLogger.h"
#include "observable/ObservableObjects.h"
#include "utils/List.h"

static const char * DEVICE_STATES_LIST_TAG = "device_states_list";

namespace Observable {
namespace DeviceState {
class DeviceStatesList : public List<DeviceState> {
 public:
  bool add(const char *name,
           Observable::ObservableObject<const char *>::ValueProviderFunction
               valueProvider) {
    if (findState(name) != nullptr) {
      LOGGER.warning(DEVICE_STATES_LIST_TAG,
                     "State with name %s already exist! Skipping...", name);
      return false;
    }
    DeviceState *state = new DeviceState();
    state->name = name;
    state->valueProvider = valueProvider;
    if (append(state) > -1) {
      LOGGER.debug(DEVICE_STATES_LIST_TAG, "Added new device state %s", name);
      return true;
    } else {
      if (state != nullptr) {
        delete state;
      }
      LOGGER.error(DEVICE_STATES_LIST_TAG, "Dailed to add new device state %s", name);
      return false;
    }
  };
  JsonDocument getValues() {
    JsonDocument doc;
    forEach([&](DeviceState *current) {
      doc[current->name] = current->valueProvider();
    });
    return doc;
  };
  const DeviceState *findState(const char *name) {
    return findValue(
        [&](DeviceState *current) { return strcmp(current->name, name) == 0; });
  };
};
}  // namespace DeviceState
}  // namespace Observable

#endif
#endif