#include "srgp_private.h"


#define DFIXED(yy)\
(srgp__curActiveCanvasSpec.max_ycoord - (yy))
#define SFIXED(yy)\
(srgp__canvasTable[source_canvas_id].max_ycoord - (yy))


void
SRGP_copyPixel (canvasID source_canvas_id, 
		rectangle source_rect, 
		point dest_corner)
{
   register int height;

   DEBUG_AIDS{
      SRGP_trace (SRGP_logStream, "SRGP_copyPixel: %d (%d,%d)->(%d,%d) -----> (%d,%d)\n",
		  source_canvas_id, ExplodeRect(source_rect), ExplodePt(dest_corner));
      srgp_check_system_state();
      srgp_check_extant_canvas (source_canvas_id);
      srgp_check_rectangle
	 (source_rect.bottom_left.x, source_rect.bottom_left.y,
	  source_rect.top_right.x, source_rect.top_right.y);
      LeaveIfNonFatalErr();
   }


   height = (source_rect.top_right.y - source_rect.bottom_left.y + 1);
   XCopyArea (srgpx__display,
	      srgp__canvasTable[source_canvas_id].drawable.bitmap,
	      srgp__curActiveCanvasSpec.drawable.bitmap,
	      srgp__curActiveCanvasSpec.gc_fill,
	      source_rect.bottom_left.x, SFIXED(source_rect.top_right.y),
	      (source_rect.top_right.x - source_rect.bottom_left.x + 1),
	      height,
	      dest_corner.x, DFIXED(dest_corner.y + height - 1));

}
