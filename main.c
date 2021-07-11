#include <stdlib.h>
#include <stdio.h>

#include <xcb/xcb.h>
#include <string.h>

#include "dock.h"

xcb_atom_t getAtomWithName(xcb_connection_t *connection, char *name);
// {
//     xcb_intern_atom_reply_t *reply = xcb_intern_atom_reply(connection, xcb_intern_atom(connection, 0, strlen(name), name), NULL);
//     printf("atom id for %s is %d\n", name, reply->atom);
//     return reply->atom;
// }

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
      switch (propertyNotifyEvent->atom) {
        case 354: {        
          xcb_get_property_reply_t *reply = xcb_get_property_reply(connection, xcb_get_property(connection, 0, rootWindow, getAtomWithName(connection, "_NET_ACTIVE_WINDOW"), getAtomWithName(connection, "WINDOW"), 0, 1), NULL);
          xcb_window_t *activeWindow = xcb_get_property_value(reply);
          printf("  active window is now %d\n", *activeWindow);
          // free(activeWindow);

          xcb_get_property_reply_t *titleReply = xcb_get_property_reply(connection, xcb_get_property(connection, 0, *activeWindow, getAtomWithName(connection, "_NET_WM_NAME"), getAtomWithName(connection, "UTF8_STRING"), 0, 32), NULL);
          if (titleReply == NULL) {
            printf("null reply\n");
            break;
          }
          char *activeWindowTitle = xcb_get_property_value(titleReply);
          if (activeWindowTitle == NULL) {
            printf("null value\n");
            break;
          }
          printf("  active window title %s\n", activeWindowTitle);
          break;
        }
        default:
        break;
      }
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
      break;
    }
    default: {
      /* Unknown event type, ignore it */
      printf("unknown response type %d\n", event->response_type & ~0x80);
      break;
    }
    }
    /* Free the Generic Event */
    free (event);
  }

  return 0;
}
