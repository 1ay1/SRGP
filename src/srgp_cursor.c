#include "srgp_private.h"


void
SRGP_loadCursor (int index, int shape)
{
   DEBUG_AIDS{
      SRGP_trace (SRGP_logStream, "SRGP_loadCursor %d %d\n", index, shape);
      srgp_check_system_state();
      srgp_check_cursor_index(index);
      LeaveIfNonFatalErr();
   }

   srgp__cursorTable[index] = XCreateFontCursor (srgpx__display, shape);

   SRGP__updateLocatorCursorShape ();
}

#define cursortype Cursor

void SRGP__initCursorTable (void)
{
   register i;
 
   for (i=0; i<=MAX_CURSOR_INDEX; i++)
      srgp__cursorTable[i] = (cursortype)NULL;
}
