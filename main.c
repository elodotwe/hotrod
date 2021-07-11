#include <stdlib.h>
#include <stdio.h>

#include <xcb/xcb.h>

#include "dock.h"

int main()
{
  xcb_connection_t *connection = xcb_connect (NULL, NULL);
  xcb_screen_t *screen = xcb_setup_roots_iterator(xcb_get_setup(connection)).data;
  xcb_drawable_t rootWindow = screen->root;

  uint32_t mask = XCB_EVENT_MASK_PROPERTY_CHANGE;
  xcb_change_window_attributes(connection, rootWindow, XCB_CW_EVENT_MASK, &mask);
  xcb_flush(connection); // window attributes won't be changed until we explicitly flush
  
  buildDock(connection, screen, rootWindow);
  buildDock(connection, screen, rootWindow);
  buildDock(connection, screen, rootWindow);

  xcb_generic_event_t *event;
  while ((event = xcb_wait_for_event (connection))) {
    // Most significant bit is set when the event comes from a SendEvent request. We don't care
    // if it was or wasn't, so we flip that bit off before checking what the response type was.
    switch (event->response_type & ~0x80) {
    case XCB_PROPERTY_NOTIFY: {
      xcb_property_notify_event_t *propertyNotifyEvent = (void *) event;
      xcb_get_atom_name_reply_t *atomNameReply = xcb_get_atom_name_reply(
          connection,
          xcb_get_atom_name(connection, propertyNotifyEvent->atom),
          NULL
        );
      char *name = xcb_get_atom_name_name(atomNameReply);
      printf("property %d, %s\n", propertyNotifyEvent->atom, name);
      break;
    }
    case XCB_CLIENT_MESSAGE: {
      xcb_client_message_event_t *clientMessageEvent = (void *) event;
      xcb_get_atom_name_reply_t *atomNameReply = xcb_get_atom_name_reply(
          connection,
          xcb_get_atom_name(connection, clientMessageEvent->type),
          NULL
        );
      char *name = xcb_get_atom_name_name(atomNameReply);
      printf("client message type %d, %s\n", clientMessageEvent->type, name);
    }
    default: {
      /* Unknown event type, ignore it */
      printf("unknown response type %d\n", event->response_type);
      break;
    }
    }
    /* Free the Generic Event */
    free (event);
  }

  return 0;
}
