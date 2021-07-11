#include <stdio.h>
#include <xcb/xcb.h>
#include <xcb/xcb_icccm.h>
#include <string.h>
#include <time.h>

#include "dock.h"
#define DOCK_HEIGHT  30

xcb_gcontext_t graphicsContext;
xcb_drawable_t dockWindow;

xcb_atom_t getAtomWithName(xcb_connection_t *connection, char *name)
{
    xcb_intern_atom_reply_t *reply = xcb_intern_atom_reply(connection, xcb_intern_atom(connection, 0, strlen(name), name), NULL);
    printf("atom id for %s is %d\n", name, reply->atom);
    return reply->atom;
}

void drawClock(xcb_connection_t *connection, xcb_drawable_t dockWindow, xcb_gcontext_t graphicsContext) {
    char* text;
    time_t currentTime;
    struct tm *localTime;
    currentTime = time(NULL);
    localTime = localtime(&currentTime);
    text = asctime(localTime);
    printf("asctime returned '%s'\n", text);
    text[strlen(text)-1] = '\0'; // trim the newline off
    xcb_image_text_8(connection, strlen(text), dockWindow, graphicsContext, 10,20, text);
    xcb_flush(connection);
}

void buildDock(xcb_connection_t *connection, xcb_screen_t *screen, xcb_drawable_t rootWindow)
{
    graphicsContext = xcb_generate_id(connection);
    uint32_t mask = XCB_GC_FOREGROUND | XCB_GC_BACKGROUND | XCB_GC_GRAPHICS_EXPOSURES;
    uint32_t values[3];
    values[0] = screen->black_pixel;
    values[1] = screen->white_pixel;
    values[2] = 0;
    xcb_create_gc(connection, graphicsContext, rootWindow, mask, values);

    /* Ask for our window's Id */
    dockWindow = xcb_generate_id(connection);

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

    xcb_size_hints_t sizeHints = { 0 };
    sizeHints.flags = XCB_ICCCM_SIZE_HINT_P_POSITION | XCB_ICCCM_SIZE_HINT_P_SIZE;
    
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

    drawClock(connection, dockWindow, graphicsContext);

}

void exposed(xcb_connection_t *connection) {
    printf("drawing\n");
    drawClock(connection, dockWindow, graphicsContext);

}