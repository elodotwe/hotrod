#include <stdio.h>
#include <xcb/xcb.h>
#include <string.h>

#include "dock.h"
#define DOCK_HEIGHT  30
xcb_atom_t getAtomWithName(xcb_connection_t *connection, char *name)
{
    xcb_intern_atom_reply_t *reply = xcb_intern_atom_reply(connection, xcb_intern_atom(connection, 0, strlen(name), name), NULL);
    printf("atom id for %s is %d\n", name, reply->atom);
    return reply->atom;
}

typedef struct {
    long flags;    /* marks which fields in this structure are defined */
    int x, y;    /* Obsolete */
    int width, height;    /* Obsolete */
    int min_width, min_height;
    int max_width, max_height;
    int width_inc, height_inc;
    struct {
           int x;    /* numerator */
           int y;    /* denominator */
    } min_aspect, max_aspect;
    int base_width, base_height;
    int win_gravity;
    /* this structure may be extended in the future */
} XSizeHints;

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
                      0, screen->height_in_pixels - DOCK_HEIGHT,                          /* x, y                */
                      screen->width_in_pixels, DOCK_HEIGHT,                      /* width, height       */
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

    xcb_atom_t windowTypeValues[] = {getAtomWithName(connection, "_NET_WM_WINDOW_TYPE_DOCK")};
    xcb_change_property(
        connection,
        XCB_PROP_MODE_REPLACE,
        dockWindow,
        getAtomWithName(connection, "_NET_WM_WINDOW_TYPE"),
        getAtomWithName(connection, "ATOM"),
        32,
        1,
        windowTypeValues
    );

    uint32_t strutValues[] = {
        0, //left
        0, //right
        0, //top
        DOCK_HEIGHT, //bottom
        0, //left start y
        0, //left end y
        0, //right start y
        0, //right end y
        0, //top start y
        0, //top end y
        0, //bottom start y
        screen->width_in_pixels //bottom end y
    };
    xcb_change_property(
        connection,
        XCB_PROP_MODE_REPLACE,
        dockWindow,
        getAtomWithName(connection, "_NET_WM_STRUT_PARTIAL"),
        getAtomWithName(connection, "CARDINAL"),
        32,
        12,
        strutValues
    );

    //uint32_t sizing[] = {0, 400, screen->width_in_pixels, 50};
    //xcb_configure_window(connection, dockWindow, XCB_CONFIG_WINDOW_X | XCB_CONFIG_WINDOW_Y | XCB_CONFIG_WINDOW_WIDTH | XCB_CONFIG_WINDOW_HEIGHT, sizing);
    //xcb_translate_coordinates(connection, dockWindow, rootWindow, 0, 50);

    XSizeHints sizeHints = { 0 };
    long PPosition = 4;
    long PSize = 8;
    sizeHints.flags = PPosition | PSize;
    // sizeHints.x = 0;
    // sizeHints.y = screen->height_in_pixels - 50;
    // sizeHints.height = 50;
    // sizeHints.width = screen->width_in_pixels;

    
    xcb_change_property(
        connection,
        XCB_PROP_MODE_REPLACE,
        dockWindow,
        XCB_ATOM_WM_NORMAL_HINTS,
        XCB_ATOM_WM_SIZE_HINTS,
        32,
        sizeof(sizeHints) / 4,
        &sizeHints
    );
    
    xcb_flush(connection);
    /* Map the window on the screen */
    xcb_map_window(connection, dockWindow);

    /* We flush the request */
    xcb_flush(connection);
}