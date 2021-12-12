#include "srgp_private.h"

static char message[] =  "Unable to open the SRGP logfile!";

int SRGP_errorOccurred = 0;

errorHandlingMode srgp__curErrHndlMode = FATAL_ERRORS;

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

