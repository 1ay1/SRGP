#define SRGP_BOSS 1
#include "srgp_private.h"

static char message[] =  "Unable to open the SRGP logfile!";

int SRGP_errorOccurred = 0;

errorHandlingMode srgp__curErrHndlMode = FATAL_ERRORS;

//Display config static data
static XWindowAttributes	srgpx__windowattrs;
static XSetWindowAttributes	srgpx__setwindowattrs;
static XSizeHints 		srgpx__sizehints;
static XWMHints			wm_hints;

/** SRGP_tracing
**/
void
SRGP_tracing (boolean please_trace)
{

   if ( ! SRGP_logStream) {
      /* LOGGING FILE IN LINE-BUFFERED MODE. */
      if (SRGP_logStream = fopen ("SRGPlogfile", "w")) {

      } else {
        fprintf (stderr, "%s\n", message);
        exit (1);
      }
   }

   SRGP_trace (SRGP_logStream, "---- TRACE DISABLED\n");
   srgp__traceDisabled = ! please_trace;
   SRGP_trace (SRGP_logStream, "++++ TRACE ENABLED\n");
}

void
SRGP_disableDebugAids ()
{
   srgp__userDebugAidsDisabled = TRUE;
}

void
SRGP_enableBlockedWait ()
{
   srgp__blockedWaitEnabled = TRUE;
}

static void
InitSRGP (char *name, int width, int height, int requested_planes, boolean debugasap)
{
   register cnt;
   
   
   unlink ("./SRGPlogfile");

   ALLOC_RECORDS (srgp__bitmapPatternTable, pattern_table_entry, MAX_PATTERN_INDEX+1);
   ALLOC_RECORDS (srgp__pixmapPatternTable, pixpat_table_entry, MAX_PATTERN_INDEX+1);
   ALLOC_RECORDS (srgp__fontTable, fontInfo, MAX_FONT_INDEX+1);
   ALLOC_RECORDS (srgp__cursorTable, cursorInfo, MAX_CURSOR_INDEX+1);
   ALLOC_RECORDS (srgp__canvasTable, canvas_spec, MAX_CANVAS_INDEX+1);
   ALLOC_RECORDS (Xformat_vertices, XPoint, MAX_POINTLIST_SIZE);
   ALLOC_RECORDS (srgp__colorLookup_table, srgp__colorTable_entry, MAX_COLORTABLE_SIZE + 1);
   
   srgp__curActiveCanvasId = 0;
   srgp__curActiveCanvasSpec.max_xcoord = (width-1);
   srgp__curActiveCanvasSpec.max_ycoord = (height-1);
   
   SRGP__initGraphicsDevice (name, requested_planes, debugasap);


   /*************** INIT. CANVAS TABLE TO SHOW THAT ONLY CANVAS 0 IS ALIVE */
   srgp__canvasTable[0].drawable = srgp__curActiveCanvasSpec.drawable;
   for (cnt=1; cnt <= MAX_CANVAS_INDEX; cnt++)
      srgp__canvasTable[cnt].drawable.win = NULL;
      
   
   SRGP__initCursorTable();

   SRGP__initDefaultPatterns();

   SRGP__initFont();

   SRGP__setCanvasDefaults();

   srgp__resizeCallback = NULL;

   /* INIT TRACING ET.AL. */
   srgp__traceDisabled = TRUE;
   SRGP_logStream = NULL;

   SRGP__initInputModule();
}

/*!*/
void SRGP_beginWithDebug 
   (char *name, int width, int height, int planes, boolean enable_trace)
/* FOR USE BY SYSTEM ADMINS ONLY */
{
   if (srgp__enabled)
      SRGP__error (ERR_ALREADY_ENABLED);
   else {
      srgp__enabled = TRUE;
      InitSRGP (name, width, height, planes, TRUE);
   }

   if (enable_trace) 
      SRGP_tracing (TRUE);
}

/*!*/
void SRGP_begin 
   (char *name, int width, int height, int planes, boolean enable_trace)
{
   if (srgp__enabled)
      SRGP__error (ERR_ALREADY_ENABLED);
   else {
      srgp__enabled = TRUE;
      InitSRGP (name, width, height, planes, FALSE);
   }

   if (enable_trace) 
      SRGP_tracing (TRUE);
}

/*!*/
void SRGP_registerResizeCallback (funcptr resizeCall)
{
   srgp__resizeCallback = resizeCall;
}

/*!*/
void SRGP_end ()
{
   SRGP_trace (SRGP_logStream, "SRGP_end\n");
   
   SRGP__cleanupGraphicsDevice();
   if (SRGP_logStream)
      fclose (SRGP_logStream);
   srgp__enabled = FALSE;
}



void SRGP_setErrorHandlingMode (errorHandlingMode newmode)
{
   srgp__curErrHndlMode = newmode;
}

/** ROUTINES ALLOWING APPLICATION TO CHANGE SIZES OF TABLES
These may only be called *before* SRGP is enabled.
**/

static boolean CheckNotEnabledYet (void)
{
   if (srgp__enabled) {
      SRGP__error (ERR_ALREADY_ENABLED);
      return FALSE;
   }
   else
      return TRUE;
}


void SRGP_setMaxCanvasIndex (int i)
{
if (CheckNotEnabledYet())
   MAX_CANVAS_INDEX = i;
}

void SRGP_setMaxPatternIndex (int i)
{
if (CheckNotEnabledYet())
   MAX_PATTERN_INDEX = i;
}

void SRGP_setMaxCursorIndex (int i)
{
if (CheckNotEnabledYet())
   MAX_CURSOR_INDEX = i;
}

void SRGP_setMaxFontIndex (int i)
{
if (CheckNotEnabledYet())
   MAX_FONT_INDEX = i;
}

void SRGP_setMaxPointlistSize (int i)
{
if (CheckNotEnabledYet())
   MAX_POINTLIST_SIZE = i;
}

void SRGP_setMaxStringSize (int i)
{
if (CheckNotEnabledYet())
   MAX_STRING_SIZE = i;
}



void
SRGP__reactToScreenResize (int www, int hhh)
{
   if (srgp__curActiveCanvasId == 0)
      srgp__canvasTable[0] = srgp__curActiveCanvasSpec;

   srgp__canvasTable[0].max_xcoord = www - 1;
   srgp__canvasTable[0].max_ycoord = hhh - 1;

   if (srgp__curActiveCanvasId == 0)
      srgp__curActiveCanvasSpec = srgp__canvasTable[0];
   
   /* The locator measure needs to be updated, since its y coord is a
      function of the max_ycoord of the screen canvas. */
   srgp__cur_locator_measure.position.y =
      srgp__canvasTable[0].max_ycoord - srgp__cur_Xcursor_y;
   
   if (srgp__resizeCallback)
      (*srgp__resizeCallback) (www, hhh);
}


void
SRGP_changeScreenCanvasSize (int newwidth, int newheight)
{
   SRGP__forceScreenResize (newwidth, newheight);
   SRGP__reactToScreenResize (newwidth, newheight);
}

/* X related Display and Graphics Init functions */
int SRGP__handlerForXerrors (Display *d, XErrorEvent *err)
{
   char msg[80];

   XGetErrorText (d, err->error_code, msg, 80);
   SRGP__error (ERR_X_SERVER, msg);
}

void SRGP__initGraphicsDevice 
   (char *name, int requested_planes, boolean debugasap)
{
   int cnt, i, j;
   int width, height;
   XEvent report;

   if ( (srgpx__display = XOpenDisplay(0)) == 0) {
      fprintf (stderr, "unable to open to x server.\n");
      exit(1);
   }
   
   if (debugasap)
      SRGP_enableSynchronous();
      
   XSetErrorHandler (SRGP__handlerForXerrors);

   srgpx__screen = DefaultScreen(srgpx__display);

   /* CREATE WINDOW FOR VIRTUAL SCREEN (canvas #0) */
   srgp__curActiveCanvasSpec.drawable.win =
      XCreateSimpleWindow (srgpx__display,
			   RootWindow(srgpx__display,srgpx__screen), 0,0,
			   width=srgp__curActiveCanvasSpec.max_xcoord+1,
			   height=srgp__curActiveCanvasSpec.max_ycoord+1,
			   1, 0L, 0L);

   SRGP__initColor (requested_planes);


   /* SET GRAPHICS CONTEXT */
   srgp__curActiveCanvasSpec.gc_frame =
      XCreateGC (srgpx__display, srgp__curActiveCanvasSpec.drawable.win,
		 0L, NULL);
   srgp__curActiveCanvasSpec.gc_fill =
      XCreateGC (srgpx__display, srgp__curActiveCanvasSpec.drawable.win,
		 0L, NULL);
   XSetForeground(srgpx__display, srgp__curActiveCanvasSpec.gc_fill,
	srgp__colorLookup_table[0].pixel_value);
   XSetForeground(srgpx__display, srgp__curActiveCanvasSpec.gc_fill,
	srgp__colorLookup_table[1].pixel_value);



   /* INIT INPUT */
   XSelectInput
      (srgpx__display, srgp__curActiveCanvasSpec.drawable.win, 
       ExposureMask|PropertyChangeMask);

   /* SET 0th CANVAS-WINDOW PROPERTIES */
   srgpx__sizehints.flags = PSize | PMinSize | PMaxSize;
   srgpx__sizehints.width = width;     srgpx__sizehints.height = height;
   srgpx__sizehints.min_width = width; srgpx__sizehints.min_height = height;
   srgpx__sizehints.max_width = width; srgpx__sizehints.max_height = height;
   XSetStandardProperties
      (srgpx__display, srgp__curActiveCanvasSpec.drawable.win,
       name, name, 0, 0, 0, &srgpx__sizehints);
   wm_hints.flags = InputHint;
   wm_hints.input = TRUE;
   XSetWMHints (srgpx__display, srgp__curActiveCanvasSpec.drawable.win,
		&wm_hints);

   /* MAP...  Wait for the expose and property events */
   XMapWindow (srgpx__display, srgp__curActiveCanvasSpec.drawable.win);
   i=0; j=0;
   do {
      XNextEvent (srgpx__display, &report);
      switch (report.type) {
       case Expose:
	 if (report.xexpose.count == 0) i++; break;
       case PropertyNotify:
	 j++; srgpx__starttime = srgpx__cur_time = report.xproperty.time; 
	 break;
      }
   } while (i==0 || j==0);

   /* No need for catching property events any more. */
   XSelectInput
      (srgpx__display, srgp__curActiveCanvasSpec.drawable.win, 
       ExposureMask);

   /* We set up for backing store. */
   srgpx__setwindowattrs.backing_store = Always;
   XChangeWindowAttributes
      (srgpx__display,
       srgp__curActiveCanvasSpec.drawable.win,
       CWBackingStore, &srgpx__setwindowattrs);
}

void SRGP__cleanupGraphicsDevice (void)
{
   XUnmapWindow (srgpx__display, srgp__curActiveCanvasSpec.drawable.win);
   XDestroyWindow (srgpx__display, srgp__curActiveCanvasSpec.drawable.win);
}



/** SRGP_refresh
**/

void
SRGP_refresh ()
{
   XSync (srgpx__display, FALSE);
   SRGP__handleRawEvents (FALSE,FALSE);
}



/** SRGP_enableSynchronous
For use by a system administrator only.
Puts X into synch. mode.
**/

void
SRGP_enableSynchronous ()
{
   XSynchronize (srgpx__display, 1);
   XSync (srgpx__display, FALSE);
}



/*!*/

static boolean resize_allowed = FALSE;

static void
InformWindowManagerOfResizeStrategy()
{
   if (resize_allowed) {
      srgpx__sizehints.flags = PMinSize | PMaxSize | PResizeInc;
      srgpx__sizehints.min_width = 5; srgpx__sizehints.min_height = 5;
      srgpx__sizehints.max_width = 5000; srgpx__sizehints.max_height = 5000;
      srgpx__sizehints.width_inc = 1; srgpx__sizehints.height_inc = 1;
      XSetStandardProperties
	 (srgpx__display, srgp__canvasTable[0].drawable.win,
	  NULL, NULL, 0, 0, 0, &srgpx__sizehints);
   }
   else {
      int width = srgp__canvasTable[0].max_xcoord + 1;
      int height = srgp__canvasTable[0].max_ycoord + 1;
      srgpx__sizehints.flags = PSize | PMinSize | PMaxSize;
      srgpx__sizehints.width = width;     srgpx__sizehints.height = height;
      srgpx__sizehints.min_width = width; srgpx__sizehints.min_height = height;
      srgpx__sizehints.max_width = width; srgpx__sizehints.max_height = height;
      XSetStandardProperties
	 (srgpx__display, srgp__curActiveCanvasSpec.drawable.win,
	  NULL, NULL, 0, 0, 0, &srgpx__sizehints);
   }
}


void
SRGP_allowResize (boolean allow)
{
   resize_allowed = allow;
   InformWindowManagerOfResizeStrategy();
}


void
SRGP__forceScreenResize (int newwidth, int newheight)
{
   XResizeWindow (srgpx__display, srgp__canvasTable[0].drawable.win,
		  newwidth, newheight);
   srgp__canvasTable[0].max_xcoord = newwidth - 1;
   srgp__canvasTable[0].max_ycoord = newheight - 1;
   InformWindowManagerOfResizeStrategy();
}

/* ^^^^ X related Display Init done ^^^^ */
