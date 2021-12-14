#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/Xos.h>
#include <X11/cursorfont.h>
#include <stdarg.h>

#include <malloc.h>


/* These are included for backwards compatibility */
#ifndef SPHIGS_USING_SRGP
#define srgp__point		               point
#endif
#define srgp__rectangle		            rectangle
#define attribute_group		            attributeGroup
#define srgp__attribute_group	         attributeGroup
#define locator_measure	               locatorMeasure
#ifndef SPHIGS_USING_SRGP
#define srgp__locator_measure	         locatorMeasure
#endif
#define deluxe_locator_measure	      deluxeLocatorMeasure
#ifndef SPHIGS_USING_SRGP
#define srgp__deluxe_locator_measure	deluxeLocatorMeasure
#endif
#define deluxe_keyboard_measure	      deluxeKeyboardMeasure
#define srgp__deluxe_keyboard_measure	deluxeKeyboardMeasure

#define REPORT_ERROR   SRGP__error
#define NON_FATAL 0
#define FATAL     1


#include "srgp_sphigs.h"
#include "srgp_errtypes.h"
#include "srgp_public.h"
#include "macros.h"

#ifdef SRGP_BOSS
#define DECLARE
boolean	srgp__traceDisabled=TRUE;
boolean	srgp__userDebugAidsDisabled=FALSE;
boolean srgp__enabled=FALSE;
boolean srgp__blockedWaitEnabled=FALSE;
#else
#define DECLARE extern
extern boolean srgp__traceDisabled;
extern boolean srgp__userDebugAidsDisabled;
extern boolean srgp__enabled;
extern boolean srgp__blockedWaitEnabled;
#endif



/** THE CANVAS DATABASE
Each canvas is described with a "canvas_spec" record, containing
the dimensions of the canvas, the current value of all the
attributes (stored in a group), and the vdi bitmap index.

When canvas X becomes active, a copy of the database spec for X
is put into the global variable "current_active_canvas_spec".  
This global's fields are changed
as activity occurs as long as X remains
active; the database spec's fields therefore becomes obsolete. 
When canvas X becomes inactive, the global's value is copied
into the database spec for X, thus updating the database, correcting
the temporary obsolescence.
**/

typedef union {
    XID    	xid;     /* FOR ANY CANVAS */
    Window 	win;     /* ONLY FOR 0th CANVAS */
    Pixmap 	bitmap;  /* FOR ALL OTHER CANVASES */
} drawable_type;

#define srgpx__screenwin		(srgp__canvasTable[0].drawable.win)

typedef struct {
   int              max_xcoord, max_ycoord;
   attributeGroup 	attributes;
   drawable_type	drawable;
   GC 		        gc_frame, gc_fill;
} canvas_spec;

DECLARE int		srgp__curActiveCanvasId;
DECLARE canvas_spec	srgp__curActiveCanvasSpec;
DECLARE canvas_spec	*srgp__canvasTable;


/** THE PATTERN DATABASES
In X11:  Each pattern index maps into a pointer to an X Pixmap.
In Mac:  bitpats and pixpats will have to be quite different!
**/

typedef Pixmap	        	pattern_table_entry, pixpat_table_entry;

DECLARE pattern_table_entry	*srgp__bitmapPatternTable;
DECLARE pixpat_table_entry	*srgp__pixmapPatternTable;

DECLARE void SRGP__initDefaultPatterns (void);



/** THE CURSOR DATABASE
Each entry maps to a cursor resource.
**/


typedef Cursor	cursorInfo;

DECLARE cursorInfo	*srgp__cursorTable;

DECLARE void SRGP__initCursorTable (void);

/** THE FONT DATABASE
X11: A fontInfo item that is NULL represents an unused entry.
Mac: txFont number -1 represents an unused entry.
Initially, only one entry (the one indexed 0) is used.
**/

typedef XFontStruct			*fontInfo;
#define SRGP_DEFAULT_FONT_0 		"8x13"

DECLARE fontInfo	*srgp__fontTable;

void SRGP__initFont (void);


/** INPUT
**/

/* locator measure */
DECLARE deluxeLocatorMeasure srgp__cur_locator_measure, srgp__get_locator_measure;
DECLARE int srgp__cur_Xcursor_x, srgp__cur_Xcursor_y;   /* IN X/Mac COORDS */
DECLARE int 	srgp__cur_locator_button_mask;

DECLARE boolean srgp__dirty_location;  
/* "dirty" bool is used only when: 1) in sample mode and 2) rubber echo off */

/* locator echo */
DECLARE int 	srgp__cur_locator_echo_type;
DECLARE point 	srgp__cur_locator_echo_anchor;
DECLARE int 	srgp__cur_cursor;

/* keyboard measure */
DECLARE srgp__deluxe_keyboard_measure 
                srgp__cur_keyboard_measure,
                srgp__get_keyboard_measure;
DECLARE int	srgp__cur_keyboard_measure_length;  /* not buffer length! */
DECLARE int 	srgp__cur_keyboard_processing_mode;

/* keyboard echo */
DECLARE int 	srgp__cur_keyboard_echo_font;
DECLARE int 	srgp__cur_keyboard_echo_color;
DECLARE point	srgp__cur_keyboard_echo_origin;

/* TIMESTAMP FOR EVENT THAT IS SUBJECT TO GET */
DECLARE int srgp__get_timestamp;

DECLARE int srgp__cur_mode[4];   /* one for each device, including NO_DEVICE */
DECLARE int srgp__device_at_head_of_queue;

void	SRGP__initInputModule (void);
void	SRGP__initInputDrivers (void);
void	SRGP__activateDevice (int device);
void    SRGP__cleanupGraphicsDevice (void);
void	SRGP__deactivateDevice (int device);
int  	SRGP__handleRawEvents (boolean in_waitEvent_call, boolean forever);
void	SRGP__initEchoModule (void);
void	SRGP__enableLocatorRubberEcho (void);
void	SRGP__disableLocatorRubberEcho (void);
void	SRGP__updateLocatorRubberEcho (void);
void	SRGP__updateLocatorRubberAnchor (void);
void	SRGP__enableLocatorCursorEcho (void);
void	SRGP__disableLocatorCursorEcho (void);
void	SRGP__updateLocatorCursorShape (void);
void	SRGP__updateRawCursorPosition (void);
void	SRGP__enableKeyboardEcho (void);
void	SRGP__disableKeyboardEcho (void);
void	SRGP__updateKeyboardEcho (void);
void	SRGP__updateKeyboardEchoAttributes (void);
void    SRGP__updateLocationKnowledge (void);
void    SRGP__updateInputSelectionMask (void);


/** MARKERS **/

void SRGP__drawSquareMarker (int x, int y);
void SRGP__drawCircleMarker (int x, int y);
void SRGP__drawXMarker (int x, int y);

DECLARE errorHandlingMode srgp__curErrHndlMode;


/** COLOR **/
DECLARE int 		srgp__available_depth;    /* usually 24, 8 or 1 */
DECLARE unsigned long 	srgp__max_pixel_value;    /* based on avail_depth */
DECLARE int		srgp__application_depth;  /* specified by appl */
DECLARE int		srgp__base_colorindex;    /* explained in color_X11.c */

DECLARE int		srgp__visual_class;
#define XBLACK		srgp__colorLookup_table[SRGP_BLACK].pixel_value
#define XWHITE		srgp__colorLookup_table[SRGP_WHITE].pixel_value
#define XCOLOR(I)	(I == 0 ? XWHITE : (I == 1 ? XBLACK : srgp__colorLookup_table[SRGP_BLACK].pixel_value))
 
#define COLORINDEX(c) SRGP__getColorIndex(c)

//Maintain a Look Up Table for colors
typedef struct __srgp__colorTable_entry {
   char name[100];
   unsigned long pixel_value;
   boolean set;
} srgp__colorTable_entry;

DECLARE srgp__colorTable_entry *srgp__colorLookup_table;

void SRGP__initColor (int requested_planes);
void SRGP__cleanupColor (void);
void SRGP__activateApplColorTable (void);
void SRGP__deactivateApplColorTable (void);
unsigned int SRGP__getColorIndex(unsigned long pixel_value);


/** LOW-LEVEL GRAPHICS **/
void SRGP__initGraphicsDevice 
   (char *name, int requested_planes, boolean debugasap);
void SRGP__cleanupMacMemory (void);

/** STORAGE OF VERTEX LISTS
Needed by the X version of output.c
**/
DECLARE XPoint *Xformat_vertices;

DECLARE funcptr         srgp__resizeCallback;


DECLARE Display        *srgpx__display;
DECLARE int 		srgpx__screen;
DECLARE Colormap 	srgpx__colormap;


/** TIMESTAMPS **/

#define rawgranularity  1000  /* per second */

DECLARE Time		srgpx__starttime, srgpx__cur_time;

/** TRACING AND DEBUGGING MACROS
**/

void SRGP__error (int errtype, ...);

#define DEBUG_AIDS      if(!srgp__userDebugAidsDisabled)

#define SRGP_trace	if(!srgp__traceDisabled)fprintf

#define LeaveIfNonFatalErr()   \
   if(SRGP_errorOccurred)return

#define srgp_check_system_state()	\
   if(!srgp__enabled)  SRGP__error(ERR_NOT_ENABLED)

#define srgp_check_rectangle(LX, BY, RX, TY)		\
   if(!((LX<=RX)&&(BY<=TY)))  SRGP__error(ERR_BAD_RECT, LX, BY, RX, TY)

#define srgp_check_font_index(F)	\
   if(!((F>=0)&&(F<=MAX_FONT_INDEX)))	\
      SRGP__error(ERR_BAD_FONT_INDEX,F)

#define srgp_check_extant_font(F)	\
   if(!((F>=0)&&(F<=MAX_FONT_INDEX)&&(srgp__fontTable[F])))	\
      SRGP__error(ERR_UNKNOWN_FONT,F)

#define srgp_check_poly_item_count(C)	\
   if(!(F>0))  SRGP__error(ERR_BAD_POLY_ITEM_COUNT,C)

#define srgp_check_marker_style(MS)		\
   if(!((MS>=0)&&(MS<NUMBER_OF_MARKER_STYLES)))  \
      SRGP__error(ERR_BAD_MARKER_STYLE,MS)

#define srgp_check_marker_size(MS)		\
   if(!(MS>0))\
      SRGP__error(ERR_BAD_MARKER_SIZE,MS)

#define srgp_check_line_style(LS)		\
   if(!((LS>=CONTINUOUS)&&(LS<=DOT_DASHED)))  \
      SRGP__error(ERR_BAD_LINE_STYLE,LS)

#define srgp_check_line_width(LW)		\
   if(!(LW>0))\
      SRGP__error(ERR_BAD_LINE_WIDTH,LW)

#define srgp_check_write_mode(WM)		\
   if(!((WM>=WRITE_REPLACE)&&(WM<=WRITE_AND)))\
      SRGP__error(ERR_BAD_WRITE_MODE,WM)

#define SRGP_correct_color(C)		\
   if(C>srgp__max_pixel_value) \
      C = srgp__max_pixel_value;

#define srgp_check_pixel_value(C,STR)		\
   if(C>srgp__max_pixel_value) \
      SRGP__error(ERR_BAD_PIXEL_VALUE,STR,C)

#define srgp_check_fill_style(F)	\
   if(!((F>=SOLID)&&(F<=BITMAP_PATTERN_OPAQUE)))\
      SRGP__error(ERR_BAD_FILL_STYLE,F)

#define srgp_check_pattern_table_entry(E) \
   if(E==0) SRGP__error(ERR_BAD_PATTERN_TABLE_ENTRY)

#define srgp_check_pattern_index(P)	\
   if(!((P>=0)&&(P<=MAX_PATTERN_INDEX)))  SRGP__error(ERR_BAD_PATTERN_ID,P)

#define srgp_check_cursor_index(P)	\
   if(!((P>=0)&&(P<=MAX_CURSOR_INDEX)))  SRGP__error(ERR_BAD_CURSOR_ID,P)

#define srgp_check_extant_canvas(CV)	\
   if(!(((CV>=0)||(CV<=MAX_CANVAS_INDEX))&&\
        (srgp__canvasTable[CV].drawable.bitmap)))\
      SRGP__error(ERR_CANVAS_NOT_OPEN,CV)

#define srgp_check_device(D)	\
   if(!((D>=KEYBOARD)&&(D<=LOCATOR)))  SRGP__error(ERR_BAD_DEVICE,D)

#define srgp_check_mode(D)	\
   if(!((D>=INACTIVE)&&(D<=EVENT)))  SRGP__error(ERR_BAD_MODE,D)

#define srgp_check_polygon_list_size(D)	\
   if((D<3)||(D>MAX_POINTLIST_SIZE))  SRGP__error(ERR_POINTLIST_SIZE,D)

#define srgp_check_polymarker_list_size(D)	\
   if((D<0)||(D>MAX_POINTLIST_SIZE))  SRGP__error(ERR_POINTLIST_SIZE,D)

#define srgp_check_polyline_list_size(D)	\
   if((D<2)||(D>MAX_POINTLIST_SIZE))  SRGP__error(ERR_POINTLIST_SIZE,D)

#define srgp_check_event_type(EVTYP)  \
   if(!(srgp__device_at_head_of_queue==EVTYP))  SRGP__error(ERR_BAD_GET)

#define srgp_check_locator_echo_type(MS)	\
   if(!((MS>=NO_ECHO)&&(MS<=RUBBER_RECT))) \
      SRGP__error(ERR_BAD_LOC_ECHO_TYPE,MS)


/** MACROS
**/

#define FIXED(yy)	\
    (srgp__curActiveCanvasSpec.max_ycoord-(yy))
#define SCREENFIXED(yy)	\
    (srgp__canvasTable[0].max_ycoord-(yy))

#define PUSH_TRACE	 srgp__userDebugAidsDisabled++
#define POP_TRACE  	 srgp__userDebugAidsDisabled--
