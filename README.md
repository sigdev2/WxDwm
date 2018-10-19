# WXDWM - wxWidgets Desktop Window Manager

## Conception

Is tiled window manager based on C++ and wxWidgets for Windows OS.
Programm has constructor for arrange windows by binary tree and store windows sets functionl.

It's helpful for maximal rational use free space of your monitor and
maximal fast sorting of windows.

## Constructor mode

To switch to constructor mode use ***Alt + A*** hot key. All windows will be arranged by monitors and auto tiled.

Use ***Alt + Z*** to exit from constructor mode or ***Alt + Q*** to exit from programm and restore all windows decorations.

In every used monitor in constructor mode the program is build binary tree structure with monitor work area as tree root,
all windows as leafs and automatical created intermediate nodes. Each intermediate node is the division of the parental
space according to the principle of dichotomy. Orientation choose automatically. If parent height larger than width then
is horizontally, else - vertically. Summary on screen we see grid of windows where all cells is
proportionaly two and use all free space.

To arrange use windows only with strict parameters:
 - Not Maximized
 - Not Minimized
 - Not FullScreen
 - Not Hidden
 - Not Tool Window
 - Not Diaplogs or sub Windows
 - Not Iconic

Minimize, maximize or close window is exclude his from structure.

In constructor mode you may use te next global hotkeys:
 - ***Shift+Alt+/*** - reflect all trees on all monitors.

You can choose which monitors to arrange:
 - ***Shift+Alt+0*** - use all monitors
 - ***Shift+Alt+<1-9>*** - toggle participating monitors

Or use hotkeys for *Active window*:
 - ***Alt+/*** - reflect two binary nodes of *Active window* or monitor where is mause pointer.
 - ***Alt+=*** - set orientation of *Active window* automatically.
 - ***Alt++*** - set orientation of *Active window* vertically.
 - ***Alt+-*** - set orientation of *Active window* horizontally.

Change screen mode, connect/disconnect displays or change used monitors call rearrange in constructor mode.

By mouse contol you may:
 - Resizing the window will move the region separator. Too much size change will be canceled.
 - Moving window from an not arranged monitor to arranged monitors areas adds it to the structure.
The reverse action remove window from the arranged structure. Dragging within arranged monitors areas
exchanges windows positions.

With hotkeys ***Alt+Ctrl+<0-9>*** you may store all active windows on all monitors. And restore stored sets by ***Alt+<0-9>***

## Command line interface

WxDWM v0.1

WxWidgets based desktop window manager. For \<ARG> read in description.

wxdwm.exe help [restore] [orientation] [pipe] [config] [toggledecoration] [scr
eenmode] [store] [toggle] [restoreall] [reflect] [reflectall] [arrange] [dearr
ange] [saveconfig] [loadconfig] [configjson] [savestored] [loadstored] [loadst
oredsjon]

Options:

    help, h                             - Show help
    [optional] pipe, p <ARG>            - Pipe name for commands
    [optional] config, c <ARG>          - Use this config 'file'

## Pipe interface

For connect to pipe server run application with option *pipe <pipe_name>*.

To talk to the app:
 - Connect to app pipe.
 - Send first query with size of query and size_t type length.
 - Send query as string with sended size.
 - Whait response.
 - Read firs response with size_t size of response.
 - Read response as string with the length of the previous size response.

Response may be 'true' or empty if is success, or with error text.

Options for query:

    help, h                             - Show help
    [optional] restore, r <ARG>         - Restore windows set by index
    [optional] orientation, o <ARG>     - 'h' or 'v' for vertical or horizontal orientation current window
    [optional] pipe, p <ARG>            - Pipe name for commands
    [optional] config, c <ARG>          - Use this config 'file'
    [optional] toggledecoration, td     - Toggle decoration of current window
    [optional] screenmode, sm <ARG>     - Toggle screen using arrange on multipy sreens by number 'n' of binary or combination of screen indexes
    [optional] store, s <ARG>           - Store windows set by index
    [optional] toggle, t <ARG>          - Toggle hide/show of stored windows set by 'index'
    [optional] restoreall, ra           - Restore all windows set by index
    [optional] reflect, rf              - Reflect current selected grid sector
    [optional] reflectall, rfa          - Reflect current all grid
    [optional] arrange, a               - Arrange
    [optional] dearrange, d             - Dearrange
    [optional] saveconfig, sc           - Save config
    [optional] loadconfig, lc           - Load cnfig
    [optional] configjson, cj <ARG>     - Use thie config 'json'
    [optional] savestored, ss           - Save stored windows sets
    [optional] loadstored, ls <ARG>     - Load stored windows sets, with 'run' or 'no'
    [optional] loadstoredjson, lj <ARG> - Load stored windows sets from 'json', its always run new instance of windows

Options may be combinated, but has priority:
help->pipe->toggledecoration->orientation->store->restore->restoreall->toggle->screenmode->arrange->reflectall->saveconfig->loadconfig->configjson->savestored->loadstored->loadstoredjson

## TODO by priority

 - window to panel
 - tile window headers
 - layouts- tabbed, accordion, grid
 - indicate frame or icon arrange status
 - May be rewrite to C# ?