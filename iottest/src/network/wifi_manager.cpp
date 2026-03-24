#include "network/wifi_manager.h"

#include <WiFi.h>
#include "core/config.h"

WifiManager::WifiManager(const String& wifiSsid, const String& wifiPassword)
    : ssid(wifiSsid), password(wifiPassword), lastReconnectAt(0) {
}

void WifiManager::begin() {
    WiFi.mode(WIFI_STA);
    WiFi.setAutoReconnect(true);
    WiFi.persistent(false);
    WiFi.begin(ssid.c_str(), password.c_str());
    lastReconnectAt = millis();
}

void WifiManager::loop() {
    if (WiFi.status() == WL_CONNECTED) {
        return;
    }

    unsigned long now = millis();
    if (now - lastReconnectAt < WIFI_RECONNECT_INTERVAL_MS) {
        return;
    }

    WiFi.disconnect();
    WiFi.begin(ssid.c_str(), password.c_str());
    lastReconnectAt = now;
}

bool WifiManager::isConnected() const {
    return WiFi.status() == WL_CONNECTED;
}

WifiSnapshot WifiManager::getSnapshot() const {
    WifiSnapshot snapshot;
    snapshot.connected = isConnected();
    snapshot.ssid = ssid;
    snapshot.ip = snapshot.connected ? WiFi.localIP().toString() : String("");
    snapshot.rssi = snapshot.connected ? WiFi.RSSI() : 0;
    return snapshot;
}
