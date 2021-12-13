#include "srgp_private.h"
#include <string.h>

void
SRGP__initFont (void)
{
   register i;
   
   srgp__fontTable[0] = XLoadQueryFont (srgpx__display, SRGP_DEFAULT_FONT_0);
   if (srgp__fontTable[0] == (XFontStruct*)NULL) {
      fprintf (stderr, "System administration problem!\n\
	X could not find font named %s\n\
	System administrator should edit srgplocal.h and change the\n\
	definition of SRGP_DEFAULT_FONT_0.\n",
	SRGP_DEFAULT_FONT_0);
      exit(1);
   }
   for (i=1; i <= MAX_FONT_INDEX; i++)
      srgp__fontTable[i] = (XFontStruct*)NULL;
}


void
SRGP_loadFont (int font_index, char *name)
{
   DEBUG_AIDS{
      srgp_check_system_state();
      SRGP_trace (SRGP_logStream, "SRGP_loadFont %d '%s'\n", font_index, name);
      srgp_check_font_index(font_index);
      LeaveIfNonFatalErr();
   }

   /***** SOME DAY WE SHOULD CALL XUnloadFont TO CLEAN UP *****/

   if ( !
       (srgp__fontTable[font_index] =
	 XLoadQueryFont (srgpx__display, name))) {
      SRGP__error (ERR_BAD_FONT_FILENAME, name);
      return;
   }
      
   if (font_index == srgp__curActiveCanvasSpec.attributes.font)
      XSetFont
	 (srgpx__display,
	  srgp__curActiveCanvasSpec.gc_frame,
	  srgp__fontTable[font_index]->fid);
}





void
SRGP_inquireTextExtent (char *str, int *width, int *height, int *descent)
{
   register XFontStruct *fontstruct;
   register char *cp;  /* traverses through the string */
   XCharStruct overall;
   int dir, asc, desc;
   
   fontstruct = srgp__fontTable[srgp__curActiveCanvasSpec.attributes.font];
   XTextExtents (fontstruct, str, strlen(str), &dir, &asc, &desc, &overall);
   *height = asc;
   *descent = desc;
   *width = overall.width;
}


