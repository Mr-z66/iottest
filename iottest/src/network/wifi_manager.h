#ifndef WIFI_MANAGER_H
#define WIFI_MANAGER_H

#include <Arduino.h>
#include "core/gateway_models.h"

class WifiManager {
private:
    String ssid;
    String password;
    unsigned long lastReconnectAt;

public:
    WifiManager(const String& wifiSsid, const String& wifiPassword);
    void begin();
    void loop();
    bool isConnected() const;
    WifiSnapshot getSnapshot() const;
};

#endif // WIFI_MANAGER_H
