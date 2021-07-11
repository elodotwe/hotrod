#include <stdio.h>
#include <xcb/xcb.h>
#include <string.h>

#include "dock.h"

xcb_atom_t getAtomWithName(xcb_connection_t *connection, char *name)
{
    xcb_intern_atom_reply_t *reply = xcb_intern_atom_reply(connection, xcb_intern_atom(connection, 0, strlen(name), name), NULL);
    printf("atom id for %s is %d\n", name, reply->atom);
    return reply->atom;
}

void buildDock(xcb_connection_t *connection, xcb_screen_t *screen, xcb_drawable_t rootWindow)
{
    xcb_gcontext_t foreground = xcb_generate_id(connection);
    uint32_t mask = XCB_GC_FOREGROUND | XCB_GC_GRAPHICS_EXPOSURES;
    uint32_t values[2];
    values[0] = screen->black_pixel;
    values[1] = 0;
    xcb_create_gc(connection, foreground, rootWindow, mask, values);

    /* Ask for our window's Id */
    xcb_drawable_t dockWindow = xcb_generate_id(connection);

    /* Create the window */
    mask = XCB_CW_BACK_PIXEL | XCB_CW_EVENT_MASK;
    values[0] = screen->white_pixel;
    values[1] = XCB_EVENT_MASK_EXPOSURE;
    xcb_create_window(connection,                    /* Connection          */
                      XCB_COPY_FROM_PARENT,          /* depth               */
                      dockWindow,                    /* window Id           */
                      screen->root,                  /* parent window       */
                      0, 0,                          /* x, y                */
                      150, 150,                      /* width, height       */
                      10,                            /* border_width        */
                      XCB_WINDOW_CLASS_INPUT_OUTPUT, /* class               */
                      screen->root_visual,           /* visual              */
                      mask, values);                 /* masks */

    xcb_atom_t protocolsValues[] = {getAtomWithName(connection, "WM_DELETE_WINDOW")};
    xcb_change_property(
        connection,
        XCB_PROP_MODE_REPLACE,
        dockWindow,
        getAtomWithName(connection, "WM_PROTOCOLS"),
        getAtomWithName(connection, "ATOM"),
        32,
        1,
        protocolsValues);

    /* Map the window on the screen */
    xcb_map_window(connection, dockWindow);

    /* We flush the request */
    xcb_flush(connection);
}