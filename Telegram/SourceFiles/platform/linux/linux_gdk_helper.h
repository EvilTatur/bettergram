/*
This file is part of Bettergram.

For license and copyright information please follow this link:
https://github.com/bettergram/bettergram/blob/master/LEGAL
*/
#pragma once

#include <QtCore/QObject>

#ifndef TDESKTOP_DISABLE_GTK_INTEGRATION
extern "C" {
#undef signals
#include <gtk/gtk.h>
#include <gdk/gdk.h>
#define signals public
} // extern "C"

namespace Platform {
namespace internal {

void GdkHelperLoad(QLibrary &lib);
bool GdkHelperLoaded();
void XSetTransientForHint(GdkWindow *window, quintptr winId);

} // namespace internal
} // namespace Platform
#endif // !TDESKTOP_DISABLE_GTK_INTEGRATION
