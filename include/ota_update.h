#ifndef OTA_UPDATE_H
#define OTA_UPDATE_H

#include <Arduino.h>

bool isNewFirmwareVersion(const String &currentVersion, const String &remoteVersion);
void checkForFirmwareUpdate(bool force = false);

#endif // OTA_UPDATE_H
