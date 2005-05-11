/*  gimprc-blurbs.h  --  descriptions for gimprc properties  */

#ifndef __GIMP_RC_BLURBS_H__
#define __GIMP_RC_BLURBS_H__


/*  Not all strings defined here are used in the user interface
 *  (the preferences dialog mainly) and only those that are should
 *  be marked for translation.
 */

#define ACTIVATE_ON_FOCUS_BLURB \
N_("When enabled, an image will become the active image when its image " \
   "window receives the focus. This is useful for window managers using " \
   "\"click to focus\".")

#define BRUSH_PATH_BLURB \
"Sets the brush search path."

#define BRUSH_PATH_WRITABLE_BLURB ""

#define CANVAS_PADDING_MODE_BLURB \
N_("Specifies how the area around the image should be drawn.")

#define CANVAS_PADDING_COLOR_BLURB \
N_("Sets the canvas padding color used if the padding mode is set to " \
   "custom color.")

#define COLOR_MANAGEMENT_BLURB \
"Defines the color management behavior."

#define CONFIRM_ON_CLOSE_BLURB \
N_("Ask for confirmation before closing an image without saving.")

#define CURSOR_FORMAT_BLURB \
N_("Sets the pixel format of cursors the GIMP will use.")

#define CURSOR_MODE_BLURB \
N_("Sets the mode of cursor the GIMP will use.")

#define CURSOR_UPDATING_BLURB \
N_("Context-dependent cursors are cool.  They are enabled by default. " \
   "However, they require overhead that you may want to do without.")

#define DEFAULT_BRUSH_BLURB \
"Specify a default brush.  The brush is searched for in the " \
"specified brush path."

#define DEFAULT_DOT_FOR_DOT_BLURB \
N_("When enabled, this will ensure that each pixel of an image gets " \
   "mapped to a pixel on the screen.")

#define DEFAULT_FONT_BLURB \
"Specify a default font.  The font is searched for in the " \
"fontconfig font path."

#define DEFAULT_GRADIENT_BLURB \
"Specify a default gradient.  The gradient is searched for in the " \
"specified gradient path."

#define DEFAULT_GRID_BLURB \
"Specify a default image grid."

#define DEFAULT_IMAGE_BLURB \
"Sets the default image in the \"File/New\" dialog."

#define DEFAULT_PATTERN_BLURB \
"Specify a default pattern. The pattern is searched for in the " \
"specified pattern path."

#define DEFAULT_PALETTE_BLURB \
"Specify a default palette.  The palette is searched for in the " \
"specified palette path."

#define DEFAULT_SNAP_DISTANCE_BLURB \
N_("This is the distance in pixels where Guide and Grid snapping " \
   "activates.")

#define DEFAULT_THRESHOLD_BLURB \
N_("Tools such as fuzzy-select and bucket fill find regions based on a " \
   "seed-fill algorithm.  The seed fill starts at the initially selected " \
   "pixel and progresses in all directions until the difference of pixel " \
   "intensity from the original is greater than a specified threshold. " \
   "This value represents the default threshold.")

#define DEFAULT_VIEW_BLURB \
"Sets the default settings for the image view."

#define DEFAULT_FULLSCREEN_VIEW_BLURB \
"Sets the default settings used when an image is viewed in fullscreen mode."

#define DOCK_WINDOW_HINT_BLURB \
N_("The window type hint that is set on dock windows. This may affect " \
   "the way your window manager decorates and handles dock windows.")

#define ENVIRON_PATH_BLURB \
"Sets the environ search path."

#define FRACTALEXPLORER_PATH_BLURB \
"Where to search for fractals used by the Fractal Explorer plug-in."

#define GAMMA_CORRECTION_BLURB \
"This setting is ignored."
#if 0
"Sets the gamma correction value for the display. 1.0 corresponds to no " \
"gamma correction.  For most displays, gamma correction should be set " \
"to between 2.0 and 2.6. One important thing to keep in mind: Many images " \
"that you might get from outside sources will in all likelihood already " \
"be gamma-corrected.  In these cases, the image will look washed-out if " \
"the GIMP has gamma-correction turned on.  If you are going to work with " \
"images of this sort, turn gamma correction off by setting the value to 1.0."
#endif

#define GFIG_PATH_BLURB \
"Where to search for Gfig figures used by the Gfig plug-in."

#define GFLARE_PATH_BLURB \
"Where to search for gflares used by the GFlare plug-in."

#define GIMPRESSIONIST_PATH_BLURB \
"Where to search for data used by the Gimpressionist plug-in."

#define GLOBAL_BRUSH_BLURB \
N_("When enabled, the selected brush will be used for all tools.")

#define GLOBAL_FONT_BLURB \
"When enabled, the selected font will be used for all tools."

#define GLOBAL_GRADIENT_BLURB \
N_("When enabled, the selected gradient will be used for all tools.")

#define GLOBAL_PATTERN_BLURB \
N_("When enabled, the selected pattern will be used for all tools.")

#define GLOBAL_PALETTE_BLURB \
"When enabled, the selected palette will be used for all tools."

#define GRADIENT_PATH_BLURB \
"Sets the gradient search path."

#define GRADIENT_PATH_WRITABLE_BLURB ""

#define FONT_PATH_BLURB \
"Where to look for fonts."

#define FONT_PATH_WRITABLE_BLURB ""

#define HELP_BROWSER_BLURB \
N_("Sets the browser used by the help system.")

#define HELP_LOCALES_BLURB \
"Specifies the language preferences used by the help system. This is a " \
"colon-separated list of language identifiers with decreasing priority. " \
"If empty, the language is taken from the user's locale setting."

#define IMAGE_STATUS_FORMAT_BLURB \
N_("Sets the text to appear in image window status bars.")

#define IMAGE_TITLE_FORMAT_BLURB \
N_("Sets the text to appear in image window titles.")

#define INITIAL_ZOOM_TO_FIT_BLURB \
N_("When enabled, this will ensure that the full image is visible after a " \
   "file is opened, otherwise it will be displayed with a scale of 1:1.")

#define INSTALL_COLORMAP_BLURB \
N_("Install a private colormap; might be useful on 8-bit (256 colors) displays.")

#define INTERPOLATION_TYPE_BLURB \
N_("Sets the level of interpolation used for scaling and other " \
   "transformations.")

#define INTERPRETER_PATH_BLURB \
"Sets the interpreter search path."

#define LAST_OPENED_SIZE_BLURB \
N_("How many recently opened image filenames to keep on the File menu.")

#define MARCHING_ANTS_SPEED_BLURB \
N_("Speed of marching ants in the selection outline.  This value is in " \
   "milliseconds (less time indicates faster marching).")

#define MAX_NEW_IMAGE_SIZE_BLURB  \
N_("GIMP will warn the user if an attempt is made to create an image that " \
   "would take more memory than the size specified here.")

#define MENU_MNEMONICS_BLURB  \
N_("When enabled, GIMP will show mnemonics in menus.")

#define MIN_COLORS_BLURB  \
N_("Generally only a concern for 8-bit displays, this sets the minimum " \
   "number of system colors allocated for the GIMP.")

#define MODULE_PATH_BLURB \
"Sets the module search path."

#define MONITOR_RES_FROM_GDK_BLURB \
"When enabled, the GIMP will use the monitor resolution from the " \
"windowing system."

#define MONITOR_XRESOLUTION_BLURB \
N_("Sets the monitor's horizontal resolution, in dots per inch.  If set to " \
   "0, forces the X server to be queried for both horizontal and vertical " \
   "resolution information.")

#define MONITOR_YRESOLUTION_BLURB \
N_("Sets the monitor's vertical resolution, in dots per inch.  If set to " \
   "0, forces the X server to be queried for both horizontal and vertical " \
   "resolution information.")

#define MOVE_TOOL_CHANGES_ACTIVE_BLURB \
N_("If enabled, the move tool changes the active layer or path when a layer " \
   "or path is being picked. This used to be the default behaviour in older " \
   "versions.")

#define NAVIGATION_PREVIEW_SIZE_BLURB \
N_("Sets the size of the navigation preview available in the lower right " \
   "corner of the image window.")

#define NUM_PROCESSORS_BLURB \
N_("On multiprocessor machines, if GIMP has been compiled with --enable-mp " \
   "this sets how many processors GIMP should use simultaneously.")

#define PALETTE_PATH_BLURB \
"Sets the palette search path."

#define PALETTE_PATH_WRITABLE_BLURB ""

#define PATTERN_PATH_BLURB \
"Sets the pattern search path."

#define PATTERN_PATH_WRITABLE_BLURB ""

#define PERFECT_MOUSE_BLURB \
N_("When enabled, the X server is queried for the mouse's current position " \
   "on each motion event, rather than relying on the position hint.  This " \
   "means painting with large brushes should be more accurate, but it may " \
   "be slower.  Perversely, on some X servers enabling this option results " \
   "in faster painting.")

#define PLUG_IN_PATH_BLURB \
"Sets the plug-in search path."

#define PLUGINRC_PATH_BLURB \
"Sets the pluginrc search path."

#define LAYER_PREVIEWS_BLURB \
N_("Sets whether GIMP should create previews of layers and channels. " \
   "Previews in the layers and channels dialog are nice to have but they " \
   "can slow things down when working with large images.")

#define LAYER_PREVIEW_SIZE_BLURB \
N_("Sets the preview size used for layers and channel previews in newly " \
   "created dialogs.")

#define RESIZE_WINDOWS_ON_RESIZE_BLURB \
N_("When enabled, the image window will automatically resize itself, " \
   "whenever the physical image size changes.")

#define RESIZE_WINDOWS_ON_ZOOM_BLURB \
N_("When enabled, the image window will automatically resize itself, " \
   "when zooming into and out of images.")

#define RESTORE_SESSION_BLURB \
N_("Let GIMP try to restore your last saved session on each startup.")

#define SAVE_DEVICE_STATUS_BLURB \
N_("Remember the current tool, pattern, color, and brush across GIMP " \
   "sessions.")

#define SAVE_SESSION_INFO_BLURB \
N_("Save the positions and sizes of the main dialogs when the GIMP exits.")

#define SAVE_TOOL_OPTIONS_BLURB \
N_("Save the tool options when the GIMP exits.")

#define SCRIPT_FU_PATH_BLURB \
"This path will be searched for scripts when the Script-Fu plug-in is run."

#define SHOW_BRUSH_OUTLINE_BLURB \
N_("When enabled, all paint tools will show a preview of the current " \
   "brush's outline.")

#define SHOW_HELP_BUTTON_BLURB \
N_("When enabled, dialogs will show a help button that gives access to " \
   "the related help page.  Without this button, the help page can still " \
   "be reached by pressing F1.")

#define SHOW_PAINT_TOOL_CURSOR_BLURB \
N_("When enabled, the cursor will be shown over the image while " \
    "using a paint tool.")

#define SHOW_MENUBAR_BLURB \
N_("When enabled, the menubar is visible by default. This can also be " \
   "toggled with the \"View->Show Menubar\" command.")

#define SHOW_RULERS_BLURB \
N_("When enabled, the rulers are visible by default. This can also be " \
   "toggled with the \"View->Show Rulers\" command.")

#define SHOW_SCROLLBARS_BLURB \
N_("When enabled, the scrollbars are visible by default. This can also be " \
   "toggled with the \"View->Show Scrollbars\" command.")

#define SHOW_STATUSBAR_BLURB \
N_("When enabled, the statusbar is visible by default. This can also be " \
   "toggled with the \"View->Show Statusbar\" command.")

#define SHOW_SELECTION_BLURB \
N_("When enabled, the selection is visible by default. This can also be " \
   "toggled with the \"View->Show Selection\" command.")

#define SHOW_LAYER_BOUNDARY_BLURB \
N_("When enabled, the layer boundary is visible by default. This can also " \
   "be toggled with the \"View->Show Layer Boundary\" command.")

#define SHOW_GUIDES_BLURB \
N_("When enabled, the guides are visible by default. This can also be " \
   "toggled with the \"View->Show Guides\" command.")

#define SHOW_GRID_BLURB \
N_("When enabled, the grid is visible by default. This can also be toggled " \
   "with the \"View->Show Grid\" command.")

#define SHOW_SAMPLE_POINTS_BLURB \
N_("When enabled, the sample points are visible by default. This can also be " \
   "toggled with the \"View->Show Sample Points\" command.")

#define SHOW_TIPS_BLURB \
N_("Enable to display a handy GIMP tip on startup.")

#define SHOW_TOOL_TIPS_BLURB \
N_("Enable to display tooltips.")

#define STINGY_MEMORY_USE_BLURB \
N_("There is always a tradeoff between memory usage and speed.  In most " \
   "cases, the GIMP opts for speed over memory.  However, if memory is a " \
   "big issue, try to enable this setting.")

#define SWAP_PATH_BLURB \
N_("Sets the swap file location. The gimp uses a tile based memory " \
   "allocation scheme. The swap file is used to quickly and easily " \
   "swap tiles out to disk and back in. Be aware that the swap file " \
   "can easily get very large if the GIMP is used with large images. " \
   "Also, things can get horribly slow if the swap file is created on " \
   "a directory that is mounted over NFS.  For these reasons, it may " \
   "be desirable to put your swap file in \"/tmp\".")

#define TEAROFF_MENUS_BLURB \
N_("When enabled, menus can be torn off.")

#define TRANSIENT_DOCKS_BLURB \
N_("When enabled, dock windows (the toolbox and palettes) are set to be " \
   "transient to the active image window. Most window managers will " \
   "keep the dock windows above the image window then, but it may also " \
   "have other effects.")

#define CAN_CHANGE_ACCELS_BLURB \
N_("When enabled, you can change keyboard shortcuts for menu items " \
   "by hitting a key combination while the menu item is highlighted.")

#define SAVE_ACCELS_BLURB \
N_("Save changed keyboard shortcuts when the GIMP exits.")

#define RESTORE_ACCELS_BLURB \
N_("Restore saved keyboard shortcuts on each GIMP startup.")

#define TEMP_PATH_BLURB \
N_("Sets the temporary storage directory. Files will appear here " \
   "during the course of running the GIMP.  Most files will disappear " \
   "when the GIMP exits, but some files are likely to remain, so it is " \
   "best if this directory not be one that is shared by other users.")

#define THEME_BLURB \
"The name of the theme to use."

#define THEME_PATH_BLURB \
"Sets the theme search path."

#define THUMBNAIL_SIZE_BLURB \
N_("Sets the size of the thumbnail shown in the Open dialog. Note that " \
   "GIMP can not create thumbnails if layer previews are disabled.")

#define THUMBNAIL_FILESIZE_LIMIT_BLURB \
N_("The thumbnail in the Open dialog will be automatically updated " \
   "if the file being previewed is smaller than the size set here.")

#define TILE_CACHE_SIZE_BLURB \
N_("The tile cache is used to make sure the GIMP doesn't thrash " \
   "tiles between memory and disk. Setting this value higher will " \
   "cause the GIMP to use less swap space, but will also cause " \
   "the GIMP to use more memory. Conversely, a smaller cache size " \
   "causes the GIMP to use more swap space and less memory.")

#define TOOLBOX_COLOR_AREA_BLURB NULL
#define TOOLBOX_FOO_AREA_BLURB NULL
#define TOOLBOX_IMAGE_AREA_BLURB NULL

#define TOOLBOX_WINDOW_HINT_BLURB \
N_("The window type hint that is set on the toolbox. This may affect " \
   "how your window manager decorates and handles the toolbox window.")

#define TRANSPARENCY_TYPE_BLURB \
N_("Sets the manner in which transparency is displayed in images.")

#define TRANSPARENCY_SIZE_BLURB \
N_("Sets the size of the checkerboard used to display transparency.")

#define TRUST_DIRTY_FLAG_BLURB \
N_("When enabled, the GIMP will not save if the image is unchanged since " \
   "opening it.")

#define UNDO_LEVELS_BLURB \
N_("Sets the minimal number of operations that can be undone. More undo " \
   "levels are kept available until the undo-size limit is reached.")

#define UNDO_SIZE_BLURB \
N_("Sets an upper limit to the memory that is used per image to keep " \
   "operations on the undo stack. Regardless of this setting, at least " \
   "as many undo-levels as configured can be undone.")

#define UNDO_PREVIEW_SIZE_BLURB \
N_("Sets the size of the previews in the Undo History.")

#define USE_HELP_BLURB  \
N_("When enabled, pressing F1 will open the help browser.")

#define WEB_BROWSER_BLURB \
N_("Sets the external web browser to be used.  This can be an absolute " \
   "path or the name of an executable to search for in the user's PATH. " \
   "If the command contains '%s' it will be replaced with the URL, else " \
   "the URL will be appended to the command with a space separating the " \
   "two.")


#endif  /* __GIMP_RC_BLURBS_H__ */
