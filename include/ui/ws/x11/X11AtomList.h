/*
 * X11AtomList.h
 *
 *  Created on: 11 дек. 2016 г.
 *      Author: sadko
 */

#ifndef WM_PREDEFINED_ATOM
    #error "This file should not be included directly"
#endif /* WM_PREDEFINED_ATOM */

// Predefined atom
WM_PREDEFINED_ATOM(XA_PRIMARY)
WM_PREDEFINED_ATOM(XA_SECONDARY)
WM_PREDEFINED_ATOM(XA_ARC)
WM_PREDEFINED_ATOM(XA_ATOM)
WM_PREDEFINED_ATOM(XA_BITMAP)
WM_PREDEFINED_ATOM(XA_CARDINAL)
WM_PREDEFINED_ATOM(XA_COLORMAP)
WM_PREDEFINED_ATOM(XA_CURSOR)
WM_PREDEFINED_ATOM(XA_CUT_BUFFER0)
WM_PREDEFINED_ATOM(XA_CUT_BUFFER1)
WM_PREDEFINED_ATOM(XA_CUT_BUFFER2)
WM_PREDEFINED_ATOM(XA_CUT_BUFFER3)
WM_PREDEFINED_ATOM(XA_CUT_BUFFER4)
WM_PREDEFINED_ATOM(XA_CUT_BUFFER5)
WM_PREDEFINED_ATOM(XA_CUT_BUFFER6)
WM_PREDEFINED_ATOM(XA_CUT_BUFFER7)
WM_PREDEFINED_ATOM(XA_DRAWABLE)
WM_PREDEFINED_ATOM(XA_FONT)
WM_PREDEFINED_ATOM(XA_INTEGER)
WM_PREDEFINED_ATOM(XA_PIXMAP)
WM_PREDEFINED_ATOM(XA_POINT)
WM_PREDEFINED_ATOM(XA_RECTANGLE)
WM_PREDEFINED_ATOM(XA_RESOURCE_MANAGER)
WM_PREDEFINED_ATOM(XA_RGB_COLOR_MAP)
WM_PREDEFINED_ATOM(XA_RGB_BEST_MAP)
WM_PREDEFINED_ATOM(XA_RGB_BLUE_MAP)
WM_PREDEFINED_ATOM(XA_RGB_DEFAULT_MAP)
WM_PREDEFINED_ATOM(XA_RGB_GRAY_MAP)
WM_PREDEFINED_ATOM(XA_RGB_GREEN_MAP)
WM_PREDEFINED_ATOM(XA_RGB_RED_MAP)
WM_PREDEFINED_ATOM(XA_STRING)
WM_PREDEFINED_ATOM(XA_VISUALID)
WM_PREDEFINED_ATOM(XA_WINDOW)
WM_PREDEFINED_ATOM(XA_WM_COMMAND)
WM_PREDEFINED_ATOM(XA_WM_HINTS)
WM_PREDEFINED_ATOM(XA_WM_CLIENT_MACHINE)
WM_PREDEFINED_ATOM(XA_WM_ICON_NAME)
WM_PREDEFINED_ATOM(XA_WM_ICON_SIZE)
WM_PREDEFINED_ATOM(XA_WM_NAME)
WM_PREDEFINED_ATOM(XA_WM_NORMAL_HINTS)
WM_PREDEFINED_ATOM(XA_WM_SIZE_HINTS)
WM_PREDEFINED_ATOM(XA_WM_ZOOM_HINTS)
WM_PREDEFINED_ATOM(XA_MIN_SPACE)
WM_PREDEFINED_ATOM(XA_NORM_SPACE)
WM_PREDEFINED_ATOM(XA_MAX_SPACE)
WM_PREDEFINED_ATOM(XA_END_SPACE)
WM_PREDEFINED_ATOM(XA_SUPERSCRIPT_X)
WM_PREDEFINED_ATOM(XA_SUPERSCRIPT_Y)
WM_PREDEFINED_ATOM(XA_SUBSCRIPT_X)
WM_PREDEFINED_ATOM(XA_SUBSCRIPT_Y)
WM_PREDEFINED_ATOM(XA_UNDERLINE_POSITION)
WM_PREDEFINED_ATOM(XA_UNDERLINE_THICKNESS)
WM_PREDEFINED_ATOM(XA_STRIKEOUT_ASCENT)
WM_PREDEFINED_ATOM(XA_STRIKEOUT_DESCENT)
WM_PREDEFINED_ATOM(XA_ITALIC_ANGLE)
WM_PREDEFINED_ATOM(XA_X_HEIGHT)
WM_PREDEFINED_ATOM(XA_QUAD_WIDTH)
WM_PREDEFINED_ATOM(XA_WEIGHT)
WM_PREDEFINED_ATOM(XA_POINT_SIZE)
WM_PREDEFINED_ATOM(XA_RESOLUTION)
WM_PREDEFINED_ATOM(XA_COPYRIGHT)
WM_PREDEFINED_ATOM(XA_NOTICE)
WM_PREDEFINED_ATOM(XA_FONT_NAME)
WM_PREDEFINED_ATOM(XA_FAMILY_NAME)
WM_PREDEFINED_ATOM(XA_FULL_NAME)
WM_PREDEFINED_ATOM(XA_CAP_HEIGHT)
WM_PREDEFINED_ATOM(XA_WM_CLASS)
WM_PREDEFINED_ATOM(XA_WM_TRANSIENT_FOR)
WM_PREDEFINED_ATOM(XA_LAST_PREDEFINED)

// Additional types
WM_ATOM(UTF8_STRING)
WM_ATOM(CLIPBOARD)
WM_ATOM(TARGETS)
WM_ATOM(INCR)

// Additional atoms
WM_ATOM(WM_PROTOCOLS)
WM_ATOM(WM_DELETE_WINDOW)
WM_ATOM(WM_STATE)
WM_ATOM(WM_TAKE_FOCUS)
WM_ATOM(WM_TRANSIENT_FOR)

// Motif WM Hints
WM_ATOM(_MOTIF_WM_HINTS)

// Root window properties
WM_ATOM(_NET_SUPPORTED)
WM_ATOM(_NET_CLIENT_LIST)
WM_ATOM(_NET_CLIENT_LIST_STACKING)
WM_ATOM(_NET_NUMBER_OF_DESKTOPS)
WM_ATOM(_NET_DESKTOP_GEOMETRY)
WM_ATOM(_NET_DESKTOP_VIEWPORT)
WM_ATOM(_NET_CURRENT_DESKTOP)
WM_ATOM(_NET_DESKTOP_NAMES)
WM_ATOM(_NET_ACTIVE_WINDOW)
WM_ATOM(_NET_WORKAREA)
WM_ATOM(_NET_SUPPORTING_WM_CHECK)
WM_ATOM(_NET_VIRTUAL_ROOTS)
WM_ATOM(_NET_DESKTOP_LAYOUT)
WM_ATOM(_NET_SHOWING_DESKTOP)

// Other Root window messages
WM_ATOM(_NET_CLOSE_WINDOW)
WM_ATOM(_NET_MOVERESIZE_WINDOW)
WM_ATOM(_NET_WM_MOVERESIZE)
WM_ATOM(_NET_RESTACK_WINDOW)
WM_ATOM(_NET_REQUEST_FRAME_EXTENTS)

// Application Window Properties
WM_ATOM(_NET_WM_NAME)
WM_ATOM(_NET_WM_VISIBLE_NAME)
WM_ATOM(_NET_WM_ICON_NAME)
WM_ATOM(_NET_WM_VISIBLE_ICON_NAME)
WM_ATOM(_NET_WM_DESKTOP)
WM_ATOM(_NET_WM_WINDOW_TYPE)
WM_ATOM(_NET_WM_STATE)
WM_ATOM(_NET_WM_ALLOWED_ACTIONS)
WM_ATOM(_NET_WM_STRUT)
WM_ATOM(_NET_WM_STRUT_PARTIAL)
WM_ATOM(_NET_WM_ICON_GEOMETRY)
WM_ATOM(_NET_WM_ICON)
WM_ATOM(_NET_WM_PID)
WM_ATOM(_NET_WM_HANDLED_ICONS)
WM_ATOM(_NET_WM_USER_TIME)
WM_ATOM(_NET_WM_USER_TIME_WINDOW)
WM_ATOM(_NET_FRAME_EXTENTS)
WM_ATOM(_NET_WM_OPAQUE_REGION)
WM_ATOM(_NET_WM_BYPASS_COMPOSITOR)

// Window types
WM_ATOM(_NET_WM_WINDOW_TYPE_DESKTOP)
WM_ATOM(_NET_WM_WINDOW_TYPE_DOCK)
WM_ATOM(_NET_WM_WINDOW_TYPE_TOOLBAR)
WM_ATOM(_NET_WM_WINDOW_TYPE_MENU)
WM_ATOM(_NET_WM_WINDOW_TYPE_UTILITY)
WM_ATOM(_NET_WM_WINDOW_TYPE_SPLASH)
WM_ATOM(_NET_WM_WINDOW_TYPE_DIALOG)
WM_ATOM(_NET_WM_WINDOW_TYPE_DROPDOWN_MENU)
WM_ATOM(_NET_WM_WINDOW_TYPE_POPUP_MENU)
WM_ATOM(_NET_WM_WINDOW_TYPE_TOOLTIP)
WM_ATOM(_NET_WM_WINDOW_TYPE_NOTIFICATION)
WM_ATOM(_NET_WM_WINDOW_TYPE_COMBO)
WM_ATOM(_NET_WM_WINDOW_TYPE_DND)
WM_ATOM(_NET_WM_WINDOW_TYPE_NORMAL)

// Window state
WM_ATOM(_NET_WM_STATE_MODAL)
WM_ATOM(_NET_WM_STATE_STICKY)
WM_ATOM(_NET_WM_STATE_MAXIMIZED_VERT)
WM_ATOM(_NET_WM_STATE_MAXIMIZED_HORZ)
WM_ATOM(_NET_WM_STATE_SHADED)
WM_ATOM(_NET_WM_STATE_SKIP_TASKBAR)
WM_ATOM(_NET_WM_STATE_SKIP_PAGER)
WM_ATOM(_NET_WM_STATE_HIDDEN)
WM_ATOM(_NET_WM_STATE_FULLSCREEN)
WM_ATOM(_NET_WM_STATE_ABOVE)
WM_ATOM(_NET_WM_STATE_BELOW)
WM_ATOM(_NET_WM_STATE_DEMANDS_ATTENTION)
WM_ATOM(_NET_WM_STATE_FOCUSED)
WM_ATOM(_NET_WM_STATE_STAYS_ON_TOP)

// Window actions
WM_ATOM(_NET_WM_ACTION_MOVE)
WM_ATOM(_NET_WM_ACTION_RESIZE)
WM_ATOM(_NET_WM_ACTION_MINIMIZE)
WM_ATOM(_NET_WM_ACTION_SHADE)
WM_ATOM(_NET_WM_ACTION_STICK)
WM_ATOM(_NET_WM_ACTION_MAXIMIZE_HORZ)
WM_ATOM(_NET_WM_ACTION_MAXIMIZE_VERT)
WM_ATOM(_NET_WM_ACTION_FULLSCREEN)
WM_ATOM(_NET_WM_ACTION_CHANGE_DESKTOP)
WM_ATOM(_NET_WM_ACTION_CLOSE)
WM_ATOM(_NET_WM_ACTION_ABOVE)
WM_ATOM(_NET_WM_ACTION_BELOW)