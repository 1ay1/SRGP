#include "srgp_private.h"

int srgp__marker_radius;

void SRGP__drawSquareMarker (int x, int y)
{
   XDrawRectangle
      (srgpx__display,
       srgp__curActiveCanvasSpec.drawable.xid,
       srgp__curActiveCanvasSpec.gc_frame,
       x-srgp__marker_radius, FIXED(y)-srgp__marker_radius,
       srgp__curActiveCanvasSpec.attributes.marker_size,
       srgp__curActiveCanvasSpec.attributes.marker_size);
}

void SRGP__drawCircleMarker (int x, int y)
{
   XDrawArc
      (srgpx__display,
       srgp__curActiveCanvasSpec.drawable.xid,
       srgp__curActiveCanvasSpec.gc_frame,
       x-srgp__marker_radius, FIXED(y)-srgp__marker_radius,
       srgp__curActiveCanvasSpec.attributes.marker_size,
       srgp__curActiveCanvasSpec.attributes.marker_size,
       0, 23040);  /* anglestart=0; anglesweep=360 */
}

void SRGP__drawXMarker (int x, int y)
{
   int lx, rx, ty, by;  /* IN LOW-LEVEL COORD SYS */

   lx = x - (srgp__marker_radius>>1);
   rx = lx + srgp__curActiveCanvasSpec.attributes.marker_size;
   ty = FIXED(y) - (srgp__marker_radius>>1);
   by = ty + srgp__curActiveCanvasSpec.attributes.marker_size;
   
   XDrawLine
      (srgpx__display,
       srgp__curActiveCanvasSpec.drawable.xid,
       srgp__curActiveCanvasSpec.gc_frame,
       lx,ty, rx,by);
   XDrawLine
      (srgpx__display,
       srgp__curActiveCanvasSpec.drawable.xid,
       srgp__curActiveCanvasSpec.gc_frame,
       lx,by, rx,ty);
}

