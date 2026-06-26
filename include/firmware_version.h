#ifndef FIRMWARE_VERSION_H
#define FIRMWARE_VERSION_H

#if __has_include("build_version.h")
#include "build_version.h"
#endif

#ifndef APP_VERSION
#define APP_VERSION "dev"
#endif

#ifndef OTA_MANIFEST_URL
#define OTA_MANIFEST_URL "https://github.com/EnderKC/E_paper_esp12s_2.9/releases/latest/download/manifest.json"
#endif

#endif // FIRMWARE_VERSION_H
