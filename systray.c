#include "systray.h"
#include <xcb/xcb.h>
#include <xcb/xcb_icccm.h>

void trayStartup(xcb_connection_t* connection, xcb_drawable_t hostWindow) {
    // Acquire manager selection "_NET_SYSTEM_TRAY_Sn" where n is screen number (probably 0?)

    // Announce acquired manager selection (should trigger already running clients to request embedding)

    // Listen for ClientMessages from clients.

}