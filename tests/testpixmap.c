#include "srgp.h"

#include <stdio.h>

static char bitpat_0 = {
   0xf0, 0xf0, 0xf0, 0xf0, 0x0f, 0x0f, 0x0f, 0x0f,
};

static char bitpat_106 = {
   0xf0, 0xf0, 0xf0, 0xf0, 0x0f, 0x0f, 0x0f, 0x0f
};

static int pixpat_0 = {
   2, 2, 2, 2, 3, 3, 3, 3,
   2, 2, 2, 2, 3, 3, 3, 3,
   2, 2, 2, 2, 3, 3, 3, 3,
   2, 2, 2, 2, 3, 3, 3, 3,
   4, 4, 4, 4, 5, 5, 5, 5,
   4, 4, 4, 4, 5, 5, 5, 5,
   4, 4, 4, 4, 5, 5, 5, 5,
   4, 4, 4, 4, 5, 5, 5, 5,
};

main(void)
{
   FILE *f;

   SRGP_begin ("Testing pixmap patterns", 800,800,24,FALSE);
   SRGP_enableSynchronous ();

   SRGP_loadCommonColor (2, "black");
   SRGP_loadCommonColor (3, "blue");
   SRGP_loadCommonColor (4, "green");
   SRGP_loadCommonColor (5, "cyan");
   SRGP_loadCommonColor (6, "red");
   SRGP_loadCommonColor (7, "magenta");
   SRGP_loadCommonColor (8, "yellow");
   SRGP_loadCommonColor (9, "white");

   SRGP_setInputMode (LOCATOR, EVENT);

   f = fopen ("pixpatdefs", "r");
   if (SRGP_loadPixmapPatternsFromFile (f)) {
      fprintf (stderr, "SOMETHING BAD HAPPENED.\n"); 
      exit(1);
   }
   fclose(f);

   SRGP_setFillPixmapPattern (0);
   SRGP_setFillStyle (PIXMAP_PATTERN);
   SRGP_fillEllipse (SRGP_defRectangle(5,5, 500,200));
   SRGP_waitEvent (INDEFINITE);

   f = fopen ("bitpatdefs", "r");
   if (SRGP_loadBitmapPatternsFromFile (f)) {
      fprintf (stderr, "SOMETHING BAD HAPPENED.\n"); 
      exit(1);
   }
   fclose(f);
   SRGP_setFillBitmapPattern (106);
   SRGP_setFillStyle (BITMAP_PATTERN_OPAQUE);
   SRGP_fillEllipse (SRGP_defRectangle(5,5, 500,200));
   SRGP_waitEvent (INDEFINITE);

   SRGP_setColor (4);
   SRGP_setFillBitmapPattern (0);
   SRGP_setFillStyle (BITMAP_PATTERN_OPAQUE);
   SRGP_fillEllipse (SRGP_defRectangle(5,5, 500,200));
   SRGP_waitEvent (INDEFINITE);
}
