#ifndef GLOBAL_H
#define GLOBAL_H

#if QT_VERSION < 0x050000
#include <QDesktopServices>
#else
#include <QStandardPaths>
#endif

namespace PODO {

#ifdef Q_OS_LINUX
#define PD_DEFAULT_APPS_PATH     "/podo/apps"
#else
#if QT_VERSION < 0x050000
#define PD_DEFAULT_APPS_PATH     QDesktopServices::storageLocation(QDesktopServices::DocumentsLocation) + "/podo/apps"
#else
#define PD_DEFAULT_APPS_PATH     QStandardPaths::standardLocations(QStandardPaths::DocumentsLocation).at(0) + "/podo/apps"
#endif // QT_VERSION < 0x050000
#endif // Q_OS_LINUX

#define PD_DEFAULT_START_APP        "pdlauncher"
#define PD_DEFAULT_START_WAYLAND    "pdwindowcompositor"
#define PD_DEFAULT_WEBENGINE_APP    PD_DEFAULT_APPS_PATH + "/pdwebengineview/pdwebengineview"


}

#endif // GLOBAL_H
