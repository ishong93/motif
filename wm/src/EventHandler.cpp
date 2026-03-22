#include <motif/wm/EventHandler.h>
#include <motif/wm/WindowManager.h>
#include <motif/wm/Decoration.h>
#include <motif/wm/KeyFocus.h>
#include <motif/wm/Menu.h>
#include <motif/wm/EWMH.h>

#include <X11/Xutil.h>
#include <X11/keysym.h>

namespace motif::wm {

EventHandler::EventHandler(WindowManager& wm) : wm_(wm) {}

void EventHandler::handleEvent(XEvent& event) {
    // Route menu events first
    if (wm_.menu().isVisible() && event.xany.window == wm_.menu().menuWindow()) {
        switch (event.type) {
            case Expose:
                wm_.menu().handleExpose();
                return;
            case ButtonPress:
                wm_.menu().handleButtonPress(event.xbutton.x, event.xbutton.y);
                return;
            case MotionNotify:
                wm_.menu().handleMotionNotify(event.xmotion.x, event.xmotion.y);
                return;
        }
    }

    switch (event.type) {
        case MapRequest:
            handleMapRequest(event.xmaprequest);
            break;
        case UnmapNotify:
            handleUnmapNotify(event.xunmap);
            break;
        case DestroyNotify:
            handleDestroyNotify(event.xdestroywindow);
            break;
        case ConfigureRequest:
            handleConfigureRequest(event.xconfigurerequest);
            break;
        case ButtonPress:
            handleButtonPress(event.xbutton);
            break;
        case ButtonRelease:
            handleButtonRelease(event.xbutton);
            break;
        case MotionNotify:
            handleMotionNotify(event.xmotion);
            break;
        case EnterNotify:
            handleEnterNotify(event.xcrossing);
            break;
        case Expose:
            handleExpose(event.xexpose);
            break;
        case PropertyNotify:
            handlePropertyNotify(event.xproperty);
            break;
        case ClientMessage:
            handleClientMessage(event.xclient);
            break;
        case KeyPress:
            handleKeyPress(event.xkey);
            break;
        default:
            break;
    }
}

void EventHandler::handleMapRequest(XMapRequestEvent& ev) {
    wm_.manageWindow(ev.window);
}

void EventHandler::handleUnmapNotify(XUnmapEvent& ev) {
    auto* client = wm_.findClient(ev.window);
    if (client && ev.window == client->client) {
        wm_.unmanageWindow(ev.window);
    }
}

void EventHandler::handleDestroyNotify(XDestroyWindowEvent& ev) {
    wm_.unmanageWindow(ev.window);
}

void EventHandler::handleConfigureRequest(XConfigureRequestEvent& ev) {
    auto* client = wm_.findClient(ev.window);

    if (client) {
        // Managed window — allow position/size changes within our frame
        if (ev.value_mask & (CWWidth | CWHeight)) {
            wm_.resizeClient(client, ev.width, ev.height);
        }
    } else {
        // Unmanaged window — forward the request as-is
        XWindowChanges changes;
        changes.x = ev.x;
        changes.y = ev.y;
        changes.width = ev.width;
        changes.height = ev.height;
        changes.border_width = ev.border_width;
        changes.sibling = ev.above;
        changes.stack_mode = ev.detail;
        XConfigureWindow(wm_.display(), ev.window, ev.value_mask, &changes);
    }
}

void EventHandler::handleButtonPress(XButtonEvent& ev) {
    auto* client = wm_.findClient(ev.window);

    // Root window click
    if (ev.window == wm_.root()) {
        if (wm_.menu().isVisible()) {
            wm_.menu().hide();
        }
        if (ev.button == 3) {
            wm_.menu().showRootMenu(ev.x_root, ev.y_root);
        }
        return;
    }

    if (!client) return;

    // Focus on click
    wm_.focusClient(client);

    // Close button
    if (ev.window == client->closeBtn) {
        wm_.decoration().drawCloseButton(client, true);
        return;
    }

    // Maximize button
    if (ev.window == client->maximizeBtn) {
        wm_.decoration().drawMaximizeButton(client, true);
        return;
    }

    // Minimize button
    if (ev.window == client->minimizeBtn) {
        wm_.decoration().drawMinimizeButton(client, true);
        return;
    }

    // Title bar — start move, or show window menu on right-click
    if (ev.window == client->titleBar) {
        if (ev.button == 1) {
            dragMode_ = DragMode::Moving;
            dragWindow_ = client->client;
            dragStartX_ = ev.x_root;
            dragStartY_ = ev.y_root;
            dragOrigX_ = client->x;
            dragOrigY_ = client->y;

            XGrabPointer(wm_.display(), client->titleBar, True,
                         ButtonReleaseMask | PointerMotionMask,
                         GrabModeAsync, GrabModeAsync, None, None, CurrentTime);
        } else if (ev.button == 3) {
            wm_.menu().showWindowMenu(client->client, ev.x_root, ev.y_root);
        }
        return;
    }

    // Frame border — start resize
    if (ev.window == client->frame && ev.button == 1) {
        dragMode_ = DragMode::Resizing;
        dragWindow_ = client->client;
        dragStartX_ = ev.x_root;
        dragStartY_ = ev.y_root;
        dragOrigW_ = client->width;
        dragOrigH_ = client->height;

        XGrabPointer(wm_.display(), client->frame, True,
                     ButtonReleaseMask | PointerMotionMask,
                     GrabModeAsync, GrabModeAsync, None, None, CurrentTime);
    }
}

void EventHandler::handleButtonRelease(XButtonEvent& ev) {
    auto* client = wm_.findClient(ev.window);

    if (dragMode_ != DragMode::None) {
        XUngrabPointer(wm_.display(), CurrentTime);
        dragMode_ = DragMode::None;
        dragWindow_ = None;
        return;
    }

    if (!client) return;

    // Close button release
    if (ev.window == client->closeBtn && ev.button == 1) {
        wm_.decoration().drawCloseButton(client, false);
        if (ev.x >= 0 && ev.x < wm_.config().buttonSize &&
            ev.y >= 0 && ev.y < wm_.config().buttonSize) {
            wm_.closeClient(client);
        }
        return;
    }

    // Maximize button release
    if (ev.window == client->maximizeBtn && ev.button == 1) {
        wm_.decoration().drawMaximizeButton(client, false);
        if (ev.x >= 0 && ev.x < wm_.config().buttonSize) {
            if (client->maximized) {
                wm_.restoreClient(client);
            } else {
                wm_.maximizeClient(client);
            }
        }
        return;
    }

    // Minimize button release
    if (ev.window == client->minimizeBtn && ev.button == 1) {
        wm_.decoration().drawMinimizeButton(client, false);
        if (ev.x >= 0 && ev.x < wm_.config().buttonSize) {
            wm_.iconifyClient(client);
        }
    }
}

void EventHandler::handleMotionNotify(XMotionEvent& ev) {
    // Compress motion events
    while (XPending(wm_.display())) {
        XEvent next;
        XPeekEvent(wm_.display(), &next);
        if (next.type == MotionNotify && next.xmotion.window == ev.window) {
            XNextEvent(wm_.display(), &next);
            ev = next.xmotion;
        } else {
            break;
        }
    }

    if (dragMode_ == DragMode::Moving && dragWindow_ != None) {
        auto* client = wm_.findClient(dragWindow_);
        if (client) {
            int dx = ev.x_root - dragStartX_;
            int dy = ev.y_root - dragStartY_;
            wm_.moveClient(client, dragOrigX_ + dx, dragOrigY_ + dy);
        }
    } else if (dragMode_ == DragMode::Resizing && dragWindow_ != None) {
        auto* client = wm_.findClient(dragWindow_);
        if (client) {
            int dx = ev.x_root - dragStartX_;
            int dy = ev.y_root - dragStartY_;
            int newW = std::max(50, dragOrigW_ + dx);
            int newH = std::max(50, dragOrigH_ + dy);
            wm_.resizeClient(client, newW, newH);
        }
    }
}

void EventHandler::handleEnterNotify(XCrossingEvent& ev) {
    if (wm_.config().focusPolicy == WMConfig::FocusPolicy::FocusFollowsMouse) {
        auto* client = wm_.findClient(ev.window);
        if (client) {
            wm_.focusClient(client);
        }
    }
}

void EventHandler::handleExpose(XExposeEvent& ev) {
    if (ev.count != 0) return;

    auto* client = wm_.findClient(ev.window);
    if (!client) return;

    if (ev.window == client->titleBar) {
        wm_.decoration().drawTitleBar(client);
    } else if (ev.window == client->closeBtn) {
        wm_.decoration().drawCloseButton(client);
    } else if (ev.window == client->maximizeBtn) {
        wm_.decoration().drawMaximizeButton(client);
    } else if (ev.window == client->minimizeBtn) {
        wm_.decoration().drawMinimizeButton(client);
    } else if (ev.window == client->frame) {
        wm_.decoration().drawBorder(client);
    }
}

void EventHandler::handlePropertyNotify(XPropertyEvent& ev) {
    auto* client = wm_.findClient(ev.window);
    if (!client) return;

    if (ev.atom == XA_WM_NAME) {
        char* name = nullptr;
        if (XFetchName(wm_.display(), client->client, &name) && name) {
            client->title = name;
            XFree(name);
            wm_.decoration().drawTitleBar(client);
        }
    }
}

void EventHandler::handleClientMessage(XClientMessageEvent& ev) {
    // Delegate EWMH client messages (_NET_WM_STATE, _NET_ACTIVE_WINDOW, etc.)
    if (wm_.ewmh().handleClientMessage(ev)) {
        return;
    }
}

void EventHandler::handleKeyPress(XKeyEvent& ev) {
    KeySym keysym = XLookupKeysym(&ev, 0);

    // Alt+Tab: cycle windows
    if (keysym == XK_Tab && (ev.state & Mod1Mask)) {
        if (ev.state & ShiftMask) {
            wm_.keyFocus().focusPrev();
        } else {
            wm_.keyFocus().focusNext();
        }
        return;
    }

    // Alt+F4: close focused window
    if (keysym == XK_F4 && (ev.state & Mod1Mask)) {
        auto* focused = wm_.keyFocus().focusedClient();
        if (focused) {
            wm_.closeClient(focused);
        }
        return;
    }
}

} // namespace motif::wm
