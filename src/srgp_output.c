#include "srgp_private.h"


#define TURN_OFF_RUBBER_ECHO_IF_ANY \
   SRGP__disableLocatorRubberEcho()

#define TURN_ON_RUBBER_ECHO_IF_ANY \
   SRGP__enableLocatorRubberEcho()



/** Functions creating geometric data 
**/

srgp__point SRGP_defPoint (int x, int y)
{
   srgp__point pt;

   pt.x = x;
   pt.y = y;
   return pt;
}

rectangle SRGP_defRectangle (int left_x, int bottom_y, int right_x, int top_y)
{
   rectangle rect;

   rect.bottom_left.x = left_x;
   rect.bottom_left.y = bottom_y;
   rect.top_right.x = right_x;
   rect.top_right.y = top_y;
   return rect;
}




/** AUDIO
**/

void
SRGP_beep()
{
   XBell (srgpx__display, 0);
}

/** POINTS 
**/

void
SRGP_pointCoord (int x, int y)
{
   DEBUG_AIDS{
      SRGP_trace (SRGP_logStream, "SRGP_pointCoord: %d,%d\n", x,y);
      srgp_check_system_state();
      LeaveIfNonFatalErr();
   }

   TURN_OFF_RUBBER_ECHO_IF_ANY;
   XDrawPoint
      (srgpx__display,
       srgp__curActiveCanvasSpec.drawable.xid,
       srgp__curActiveCanvasSpec.gc_frame,
       x, FIXED(y));
   TURN_ON_RUBBER_ECHO_IF_ANY;
}

void
SRGP_point (srgp__point pt)
{
   DEBUG_AIDS{
      SRGP_trace (SRGP_logStream, "SRGP_point: %d,%d\n", ExplodePt(pt));
   }

   PUSH_TRACE;
   SRGP_pointCoord (pt.x, pt.y);
   POP_TRACE;
}

void
SRGP_polyPoint (int vertex_count, srgp__point *vertices)
{
   register int i;

   DEBUG_AIDS{
      SRGP_trace (SRGP_logStream, "SRGP_polyPoint  %d 0x%x\n", 
	       vertex_count, vertices);
      srgp_check_system_state();
      srgp_check_polymarker_list_size(vertex_count);
      LeaveIfNonFatalErr();
   }

   TURN_OFF_RUBBER_ECHO_IF_ANY;

   for (i=0; i<vertex_count; i++) {
      Xformat_vertices[i].x = vertices[i].x;
      Xformat_vertices[i].y = FIXED(vertices[i].y);
   } 
   XDrawPoints
      (srgpx__display,
       srgp__curActiveCanvasSpec.drawable.xid,
       srgp__curActiveCanvasSpec.gc_frame,
       Xformat_vertices, vertex_count, CoordModeOrigin);

   TURN_ON_RUBBER_ECHO_IF_ANY;
}


void
SRGP_polyPointCoord (int vertex_count, int *x_coords, int *y_coords)
{
   register int i;

   DEBUG_AIDS{
      SRGP_trace (SRGP_logStream, "SRGP_polyPointCoord  %d 0x%x,0x%x\n",
		  vertex_count, x_coords, y_coords);
      srgp_check_system_state();
      srgp_check_polymarker_list_size(vertex_count);
      LeaveIfNonFatalErr();
   }

   TURN_OFF_RUBBER_ECHO_IF_ANY;
   
   for (i=0; i < vertex_count; i++) {
      Xformat_vertices[i].x = x_coords[i];
      Xformat_vertices[i].y = FIXED(y_coords[i]);
   } 
   XDrawPoints
      (srgpx__display,
       srgp__curActiveCanvasSpec.drawable.xid,
       srgp__curActiveCanvasSpec.gc_frame,
       Xformat_vertices, vertex_count, CoordModeOrigin);

   TURN_ON_RUBBER_ECHO_IF_ANY;
}




/** MARKERS
**/

extern int srgp__marker_radius;  /* found in marker.c */
static int linewidthtemp, saveLineWidth;

static void
GetReadyToPaintMarkers (void)
{
   TURN_OFF_RUBBER_ECHO_IF_ANY;
   PUSH_TRACE;

   srgp__marker_radius = srgp__curActiveCanvasSpec.attributes.marker_size >> 1;

   saveLineWidth = srgp__curActiveCanvasSpec.attributes.line_width;
   SRGP_setLineWidth
      (((linewidthtemp=(srgp__marker_radius>>3))==0)?1:linewidthtemp);
}


static void
FinishPaintingMarkers (void)
{
   SRGP_setLineWidth (saveLineWidth);

   POP_TRACE;
   TURN_ON_RUBBER_ECHO_IF_ANY;
}   


   
void 
SRGP_markerCoord (int x, int y)
{
   DEBUG_AIDS{
      SRGP_trace (SRGP_logStream, "SRGP_markerCoord: %d,%d\n", x,y);
      srgp_check_system_state();
      LeaveIfNonFatalErr();
   }

   GetReadyToPaintMarkers();
   switch (srgp__curActiveCanvasSpec.attributes.marker_style) {
    case MARKER_CIRCLE: SRGP__drawCircleMarker (x,y); break;
    case MARKER_SQUARE: SRGP__drawSquareMarker (x,y); break;
    case MARKER_X: SRGP__drawXMarker (x,y); break;
   }
   FinishPaintingMarkers();
}

   
void 
SRGP_marker (srgp__point pt)
{
   DEBUG_AIDS{
      SRGP_trace (SRGP_logStream, "SRGP_marker: %d,%d\n", pt.x, pt.y);
      srgp_check_system_state();
      LeaveIfNonFatalErr();
   }

   GetReadyToPaintMarkers();
   switch (srgp__curActiveCanvasSpec.attributes.marker_style) {
    case MARKER_CIRCLE: SRGP__drawCircleMarker (pt.x,pt.y); break;
    case MARKER_SQUARE: SRGP__drawSquareMarker (pt.x,pt.y); break;
    case MARKER_X: SRGP__drawXMarker (pt.x,pt.y); break;
   }
   FinishPaintingMarkers();
}

   
void 
SRGP_polyMarker (int vertex_count, srgp__point *vertices)
{
   DEBUG_AIDS{
      SRGP_trace (SRGP_logStream, "SRGP_polyMarker: %d  %lx\n", 
		  vertex_count, vertices);
      srgp_check_polymarker_list_size(vertex_count);
      srgp_check_system_state();
      LeaveIfNonFatalErr();
   }

   GetReadyToPaintMarkers();
   while (vertex_count--) {
      switch (srgp__curActiveCanvasSpec.attributes.marker_style) {
       case MARKER_CIRCLE: 
	 SRGP__drawCircleMarker (vertices->x,vertices->y); break;
       case MARKER_SQUARE: 
	 SRGP__drawSquareMarker (vertices->x,vertices->y); break;
       case MARKER_X: 
	 SRGP__drawXMarker (vertices->x,vertices->y); break;
      }
      vertices++;
   }
   FinishPaintingMarkers();
}

   
void 
SRGP_polyMarkerCoord (int vertex_count, int *xlist, int *ylist)
{
   DEBUG_AIDS{
      SRGP_trace (SRGP_logStream, "SRGP_polyMarkerCoord: %d  %lx %lx\n", 
		  vertex_count, xlist, ylist);
      srgp_check_polymarker_list_size(vertex_count);
      srgp_check_system_state();
      LeaveIfNonFatalErr();
   }

   GetReadyToPaintMarkers();
   while (vertex_count--) {
      switch (srgp__curActiveCanvasSpec.attributes.marker_style) {
       case MARKER_CIRCLE: SRGP__drawCircleMarker (*xlist, *ylist); break;
       case MARKER_SQUARE: SRGP__drawSquareMarker (*xlist, *ylist); break;
       case MARKER_X: SRGP__drawXMarker (*xlist, *ylist); break;
      }
      xlist++; ylist++;
   }
   FinishPaintingMarkers();
}

   

/** LINES
X11 handles dashes, etc. itself.
Macintosh:  we must simulate all line styles other than continuous.
**/

void
SRGP_lineCoord (int x1, int y1, int x2, int y2)
{
   DEBUG_AIDS{
      SRGP_trace (SRGP_logStream, "SRGP_lineCoord: %d,%d --> %d,%d\n", x1,y1,x2,y2);
      srgp_check_system_state();
      LeaveIfNonFatalErr();
   }

   TURN_OFF_RUBBER_ECHO_IF_ANY;
   XDrawLine
      (srgpx__display,
       srgp__curActiveCanvasSpec.drawable.xid,
       srgp__curActiveCanvasSpec.gc_frame,
       x1, FIXED(y1), x2, FIXED(y2));
   TURN_ON_RUBBER_ECHO_IF_ANY;
}

void
SRGP_line (srgp__point pt1, srgp__point pt2)
{
   DEBUG_AIDS{
      SRGP_trace (SRGP_logStream, "SRGP_line: %d,%d --> %d,%d\n", ExplodePt(pt1), ExplodePt(pt2));
   }

   PUSH_TRACE;
   SRGP_lineCoord (pt1.x, pt1.y,  pt2.x, pt2.y);
   POP_TRACE;
}



/** RECTANGLES
**/

void
SRGP_rectangleCoord (int left_x, int bottom_y, int right_x, int top_y)
{
   DEBUG_AIDS{
      SRGP_trace (SRGP_logStream, "SRGP_rectangleCoord: (%d,%d)->(%d,%d)\n",
		  left_x, bottom_y, right_x, top_y);
      srgp_check_system_state();
      srgp_check_rectangle (left_x, bottom_y, right_x, top_y);
      LeaveIfNonFatalErr();
   }

   TURN_OFF_RUBBER_ECHO_IF_ANY;
   XDrawRectangle
      (srgpx__display,
       srgp__curActiveCanvasSpec.drawable.xid,
       srgp__curActiveCanvasSpec.gc_frame,
       left_x, FIXED(top_y),
       (right_x-left_x),
       (top_y-bottom_y));
   TURN_ON_RUBBER_ECHO_IF_ANY;
}

void
SRGP_rectangle (rectangle rect)
{
   DEBUG_AIDS{
      SRGP_trace (SRGP_logStream, "SRGP_rectangle: (%d,%d)->(%d,%d)\n", ExplodeRect(rect));
   }

   PUSH_TRACE;
   SRGP_rectangleCoord
      (rect.bottom_left.x, rect.bottom_left.y,
       rect.top_right.x, rect.top_right.y);
   POP_TRACE;
}

void
SRGP_rectanglePt (srgp__point bottom_left, srgp__point top_right)
{
   DEBUG_AIDS{
      SRGP_trace (SRGP_logStream, "SRGP_rectanglePt: (%d,%d)->(%d,%d)\n",
		  ExplodePt(bottom_left), ExplodePt(top_right));
   }

   PUSH_TRACE;
   SRGP_rectangleCoord
      (bottom_left.x, bottom_left.y,
       top_right.x, top_right.y);
   POP_TRACE;
}
   






/** FRAMED POLYGONS
**/

void
SRGP_polyLine (int vertex_count, srgp__point *vertices)
{
   register int i;

   DEBUG_AIDS{
      SRGP_trace (SRGP_logStream, "SRGP_polyLine  %d 0x%x\n", vertex_count, vertices);
      srgp_check_system_state();
      srgp_check_polyline_list_size(vertex_count);
      LeaveIfNonFatalErr();
   }

   TURN_OFF_RUBBER_ECHO_IF_ANY;
   for (i=0; i < vertex_count; i++) {
      Xformat_vertices[i].x = vertices[i].x;
      Xformat_vertices[i].y = FIXED(vertices[i].y);
   } 
   XDrawLines
      (srgpx__display,
       srgp__curActiveCanvasSpec.drawable.xid,
       srgp__curActiveCanvasSpec.gc_frame,
       Xformat_vertices, vertex_count, CoordModeOrigin);
   TURN_ON_RUBBER_ECHO_IF_ANY;
}


void
SRGP_polyLineCoord (int vertex_count, int *x_coords, int *y_coords)
{
   register int i;

   DEBUG_AIDS{
      SRGP_trace (SRGP_logStream, "SRGP_polyLineCoord  %d 0x%x,0x%x\n",
		  vertex_count, x_coords, y_coords);
      srgp_check_system_state();
      srgp_check_polyline_list_size(vertex_count);
      LeaveIfNonFatalErr();
   }

   TURN_OFF_RUBBER_ECHO_IF_ANY;
   for (i=0; i < vertex_count; i++) {
      Xformat_vertices[i].x = x_coords[i];
      Xformat_vertices[i].y = FIXED(y_coords[i]);
   } 
   XDrawLines
      (srgpx__display,
       srgp__curActiveCanvasSpec.drawable.xid,
       srgp__curActiveCanvasSpec.gc_frame,
       Xformat_vertices, vertex_count, CoordModeOrigin);
   TURN_ON_RUBBER_ECHO_IF_ANY;
}


void
SRGP_polygon (int vertex_count, srgp__point *vertices)
{
   DEBUG_AIDS{
      SRGP_trace (SRGP_logStream, "SRGP_polygon  %d 0x%x\n", 
		  vertex_count, vertices);
      srgp_check_system_state();
      srgp_check_polygon_list_size(vertex_count);
      LeaveIfNonFatalErr();
   }

   PUSH_TRACE;
   SRGP_polyLine (vertex_count, vertices);
   /* draw the line between the first vertex and the last vertex */
   SRGP_line (vertices[0], vertices[vertex_count-1]);
   POP_TRACE;
}


void
SRGP_polygonCoord (int vertex_count, int *x_coords, int *y_coords)
{
   DEBUG_AIDS{
      SRGP_trace (SRGP_logStream, "SRGP_polygonCoord  %d 0x%x,0x%x\n",
		  vertex_count, x_coords, y_coords);
      srgp_check_system_state();
      srgp_check_polygon_list_size(vertex_count);
      LeaveIfNonFatalErr();
   }

   PUSH_TRACE;
   SRGP_polyLineCoord (vertex_count, x_coords, y_coords);
   /* draw the line between the first vertex and the last vertex */
   SRGP_lineCoord
      (x_coords[0], y_coords[0],
       x_coords[vertex_count-1], y_coords[vertex_count-1]);
   POP_TRACE;
}



/** FILLED RECTANGLES AND POLYGONS
**/

void
SRGP_fillRectangleCoord (int left_x, int bottom_y, int right_x, int top_y)
{
   DEBUG_AIDS{
      SRGP_trace (SRGP_logStream, "SRGP_fillRectangleCoord (%d,%d)->(%d,%d)\n",
		  left_x, bottom_y, right_x, top_y);
      srgp_check_system_state();
      srgp_check_rectangle(left_x, bottom_y, right_x, top_y);
      LeaveIfNonFatalErr();
   }

   
   TURN_OFF_RUBBER_ECHO_IF_ANY;
   XFillRectangle
      (srgpx__display,
       srgp__curActiveCanvasSpec.drawable.xid,
       srgp__curActiveCanvasSpec.gc_fill,
       left_x, FIXED(top_y),
       (right_x-left_x+1), (top_y-bottom_y+1));
   TURN_ON_RUBBER_ECHO_IF_ANY;  
}

void
SRGP_fillRectangle (rectangle rect)
{
   DEBUG_AIDS{
      SRGP_trace (SRGP_logStream, "SRGP_fillRectangle (%d,%d)->(%d,%d)\n", 
		  ExplodeRect(rect));
   }

   PUSH_TRACE;
   SRGP_fillRectangleCoord
      (rect.bottom_left.x, rect.bottom_left.y,
       rect.top_right.x, rect.top_right.y);
   POP_TRACE;
}

void
SRGP_fillRectanglePt (srgp__point bottom_left, srgp__point top_right)
{
   DEBUG_AIDS{
      SRGP_trace (SRGP_logStream, "SRGP_fillRectanglePt (%d,%d)->(%d,%d)\n",
		  ExplodePt(bottom_left), ExplodePt(top_right));
   }

   PUSH_TRACE;
   SRGP_fillRectangleCoord
      (bottom_left.x, bottom_left.y,
       top_right.x, top_right.y);
   POP_TRACE;
}
  

void
SRGP_fillPolygon (int vertex_count, srgp__point *vertices)
{
   register int i;
#ifdef THINK_C
   PolyHandle p;
#endif

   DEBUG_AIDS{
      SRGP_trace (SRGP_logStream, "SRGP_fillPolygon:  %d  0x%x\n", 
		  vertex_count, vertices);
      srgp_check_system_state();
      srgp_check_polygon_list_size(vertex_count);
      LeaveIfNonFatalErr();
   }

   TURN_OFF_RUBBER_ECHO_IF_ANY;

   for (i=0; i < vertex_count; i++) {
      Xformat_vertices[i].x = vertices[i].x;
      Xformat_vertices[i].y = FIXED(vertices[i].y);
   }
   XFillPolygon
      (srgpx__display,
       srgp__curActiveCanvasSpec.drawable.xid,
       srgp__curActiveCanvasSpec.gc_fill,
       Xformat_vertices, vertex_count, Complex, CoordModeOrigin);
   TURN_ON_RUBBER_ECHO_IF_ANY;
}

 
void
SRGP_fillPolygonCoord (int vertex_count, int *x_coords, int *y_coords)
{
   register int i;

   DEBUG_AIDS{
      SRGP_trace (SRGP_logStream, "SRGP_fillPolygonCoord:  %d  0x%x,0x%x\n",
		  vertex_count, x_coords, y_coords);
      srgp_check_system_state();
      srgp_check_polygon_list_size(vertex_count);
      LeaveIfNonFatalErr();
   }

   TURN_OFF_RUBBER_ECHO_IF_ANY;
   
   for (i=0; i < vertex_count; i++) {
      Xformat_vertices[i].x = x_coords[i];
      Xformat_vertices[i].y = FIXED(y_coords[i]);
   }
   XFillPolygon
      (srgpx__display,
       srgp__curActiveCanvasSpec.drawable.xid,
       srgp__curActiveCanvasSpec.gc_fill,
       Xformat_vertices, vertex_count, Complex, CoordModeOrigin);

   TURN_ON_RUBBER_ECHO_IF_ANY;
}



/** ELLIPSES
**/

static int xangle1, xangle2;

ComputeXangles (double start, double end)
{
   if (start <= end) {
      xangle1 = (int)(start*64);
      xangle2 = (int)((end-start)*64);
   }
   else {
      xangle1 = (int) (end*64);
      xangle2 = (int) ((start-360.0-end)*64);
   }
}
   
void
SRGP_ellipseArc (rectangle bounding_rect, double start, double end)
{


   DEBUG_AIDS{
      SRGP_trace (SRGP_logStream, "SRGP_ellipse:  (%d,%d)->(%d,%d)\n",
		  ExplodeRect(bounding_rect));
      srgp_check_system_state();
      srgp_check_rectangle
	 (bounding_rect.bottom_left.x, bounding_rect.bottom_left.y,
	  bounding_rect.top_right.x, bounding_rect.top_right.y);
      LeaveIfNonFatalErr();
   }

   TURN_OFF_RUBBER_ECHO_IF_ANY;
   ComputeXangles (start, end);
   XDrawArc
      (srgpx__display,
       srgp__curActiveCanvasSpec.drawable.xid,
       srgp__curActiveCanvasSpec.gc_frame,
       bounding_rect.bottom_left.x, FIXED(bounding_rect.top_right.y),
       (bounding_rect.top_right.x - bounding_rect.bottom_left.x),
       (bounding_rect.top_right.y - bounding_rect.bottom_left.y),
       xangle1, xangle2);
   TURN_ON_RUBBER_ECHO_IF_ANY;
}


void
SRGP_ellipse (rectangle bounding_rect)
{
   DEBUG_AIDS{
      SRGP_trace (SRGP_logStream, "SRGP_ellipse:  (%d,%d)->(%d,%d)\n",
		  ExplodeRect(bounding_rect));
      srgp_check_system_state();
      srgp_check_rectangle
	 (bounding_rect.bottom_left.x, bounding_rect.bottom_left.y,
	  bounding_rect.top_right.x, bounding_rect.top_right.y);
      LeaveIfNonFatalErr();
   }

   PUSH_TRACE;
   SRGP_ellipseArc (bounding_rect, 0.0, 360.0);
   POP_TRACE;
}




void
SRGP_fillEllipseArc (rectangle bounding_rect, double start, double end)
{
   DEBUG_AIDS{
      SRGP_trace (SRGP_logStream, "SRGP_fillEllipseArc:  (%d,%d)->(%d,%d)\n",
		  ExplodeRect(bounding_rect));
      srgp_check_system_state();
      srgp_check_rectangle
	 (bounding_rect.bottom_left.x, bounding_rect.bottom_left.y,
	  bounding_rect.top_right.x, bounding_rect.top_right.y);
      LeaveIfNonFatalErr();
   }

   TURN_OFF_RUBBER_ECHO_IF_ANY;
   ComputeXangles (start, end);
   XFillArc
      (srgpx__display,
       srgp__curActiveCanvasSpec.drawable.xid,
       srgp__curActiveCanvasSpec.gc_fill,
       bounding_rect.bottom_left.x, FIXED(bounding_rect.top_right.y),
       (bounding_rect.top_right.x - bounding_rect.bottom_left.x),
       (bounding_rect.top_right.y - bounding_rect.bottom_left.y),
       xangle1, xangle2);
   TURN_ON_RUBBER_ECHO_IF_ANY;
}



void
SRGP_fillEllipse (rectangle bounding_rect)
{
   DEBUG_AIDS{
      SRGP_trace (SRGP_logStream, "SRGP_fillEllipse:  (%d,%d)->(%d,%d)\n",
		  ExplodeRect(bounding_rect));
      srgp_check_system_state();
      srgp_check_rectangle
	 (bounding_rect.bottom_left.x, bounding_rect.bottom_left.y,
	  bounding_rect.top_right.x, bounding_rect.top_right.y);
      LeaveIfNonFatalErr();
   }

   PUSH_TRACE;
   SRGP_fillEllipseArc (bounding_rect, 0.0, 360.0);
   POP_TRACE;
}



/** TEXT
X11 implementation notes:
   Unfortunately, we don't keep a GC around just for text.
   Neither the frame nor the fill GC are necessarily correct for text;
   text is always drawn in SOLID style.

   Basically, if we find that the frame GC currently has a non-solid X-fill-style,
   we temporarily change the frame GC's X-fill-style to SOLID and then restore.

   This could be woefully inefficient if the application is indeed using
   a non-solid pen-style!
**/

void
SRGP_text (srgp__point origin, char *str)
{
   boolean restoration_needed = FALSE;

   DEBUG_AIDS{
      SRGP_trace (SRGP_logStream, "SRGP_text:  %d,%d  %s\n", 
		  origin.x, origin.y, str);
      srgp_check_system_state();
      LeaveIfNonFatalErr();
   }

   TURN_OFF_RUBBER_ECHO_IF_ANY;

   if (srgp__curActiveCanvasSpec.attributes.pen_style != SOLID) {
      restoration_needed = TRUE;
      XSetFillStyle (srgpx__display,
		     srgp__curActiveCanvasSpec.gc_frame, SOLID);
   }

   XDrawString
      (srgpx__display,
       srgp__curActiveCanvasSpec.drawable.xid,
       srgp__curActiveCanvasSpec.gc_frame,
       origin.x, FIXED(origin.y), str, strlen(str));

   if (restoration_needed)
      XSetFillStyle (srgpx__display,
		     srgp__curActiveCanvasSpec.gc_frame,
		     srgp__curActiveCanvasSpec.attributes.pen_style);

   TURN_ON_RUBBER_ECHO_IF_ANY;
}
