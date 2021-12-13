#include "srgp_private.h"

/** THIS FILE IS FOR X11 IMPLEMENTATION ONLY
**/

void 
SRGP__initColor (requested_planes)
{
   srgp__visual_class = DefaultVisual(srgpx__display, srgpx__screen)->class;
   srgp__available_depth = DefaultDepth(srgpx__display, srgpx__screen);

   int depth_count;
   int supports_requested_depth = 0;

   char error_msg_buf[500];
   char supported_depths_str_buff[10];

   int *all_depths = XListDepths(srgpx__display, srgpx__screen, &depth_count);
   
   for(int i = 0; i < depth_count; i++) {
      if(requested_planes == all_depths[i]) {
         supports_requested_depth = 1;
      }
      sprintf(supported_depths_str_buff, "%d", all_depths[i]);
      if(i == 0) {
         strcpy(error_msg_buf, supported_depths_str_buff);
      } else {
         strcat(error_msg_buf, ", ");
         strcat(error_msg_buf, supported_depths_str_buff);
      }
   }
   
   if(!supports_requested_depth) {
      fprintf(stderr, "The requested depth(planes) of %d is not supported\n",
      "Supported depths: %s\n", requested_planes, error_msg_buf);
      abort();
   }

   XVisualInfo srgp_visual_info;
   Status res = XMatchVisualInfo(srgpx__display, srgpx__screen,
   requested_planes, TrueColor, &srgp_visual_info);

   if(!res) {
      fprintf(stderr, "There is no visual with that depth %d. Try some other depth.\n",
      "Supported depths: %s\n", requested_planes, error_msg_buf);
      abort();
   }

   srgp__application_depth = requested_planes;
   srgp__max_pixel_value = (1 << srgp__application_depth) - 1;

   srgpx__colormap = XCreateColormap(srgpx__display, srgp__curActiveCanvasSpec.drawable.win,
   srgp_visual_info.visual, AllocNone);

   XSetWindowColormap (srgpx__display, 
	srgp__curActiveCanvasSpec.drawable.win,
	srgpx__colormap);

   srgp__base_colorindex = 0;

   //get the black and white pixel values
   XFlush(srgpx__display);
   XColor black, black_e, white, white_e;
   XAllocNamedColor(srgpx__display, srgpx__colormap, "Black", &black, &black_e);
   XAllocNamedColor(srgpx__display, srgpx__colormap, "White", &white, &white_e);
   XFlush(srgpx__display);

   SRGP_BLACK = black.pixel;
   SRGP_WHITE = white.pixel;

   printf("%d ---  %d\n", black.pixel, white.pixel);

   //Initialize the Color Table with the 2 values of Black and White
   strcpy(srgp__colorLookup_table[0].name, "black");
   srgp__colorLookup_table[0].pixel_value = SRGP_BLACK;
   strcpy(srgp__colorLookup_table[1].name, "white");
   srgp__colorLookup_table[1].pixel_value = SRGP_WHITE;
   srgp__total_loaded_colors = 2;

   /*** DONE FOR ALL CONFIGURATIONS. */
   XSetWindowBackground (srgpx__display, 
			 srgp__curActiveCanvasSpec.drawable.win, SRGP_WHITE);
   XSetWindowBorder (srgpx__display, 
		     srgp__curActiveCanvasSpec.drawable.win, SRGP_BLACK);

   //Free things
   XFree(all_depths);
}



void SRGP_loadColorTable
   (int startentry, int count,
    unsigned short *redi, 
    unsigned short *greeni,
    unsigned short *bluei)
{
   int sane_start_entry = srgp__total_loaded_colors;
   int endi = sane_start_entry + count;

   int sane_count = count;
   int sane_endi = endi;


   if(endi > MAX_COLORTABLE_SIZE) {
      fprintf(stderr, "Be easy with the color allocation.\n",
      "You can only allocate %d colors in one program.\n",
      "You have %d left\n", MAX_COLORTABLE_SIZE, MAX_COLORTABLE_SIZE - srgp__total_loaded_colors);
      sane_endi = MAX_COLORTABLE_SIZE;
      sane_count = sane_endi - (srgp__total_loaded_colors - 1);
   }

   int count_ratio = count/sane_count;

   XColor x_color_structs[sane_count];

   for(int i = 0; i < sane_count; i++) {
      x_color_structs[i].blue = bluei[i];
      x_color_structs[i].green = greeni[i];
      x_color_structs[i].red = redi[i];
   }

   for(int i = 0; i < sane_count; i++) {
      XAllocColor(srgpx__display, srgpx__colormap, x_color_structs + i);
   }

   for(int i = 0; i < sane_count; i++) {
      strcpy(srgp__colorLookup_table[srgp__total_loaded_colors].name, "rgb");
      srgp__colorLookup_table[srgp__total_loaded_colors].pixel_value = x_color_structs[i].pixel +  ((i/2)*count_ratio);

      srgp__total_loaded_colors++;
   }

      DEBUG_AIDS{
      SRGP_trace (SRGP_logStream, "SRGP_loadColorTable  %d  %d  %x %x %x\n",
		  startentry, sane_count, redi, greeni, bluei);

      /* PERFORM CHECKING LEGALITY OF THE RANGE OF INDICES. */
      srgp_check_pixel_value (srgp__colorLookup_table[sane_start_entry].pixel_value, "start");
      srgp_check_pixel_value (srgp__colorLookup_table[sane_endi].pixel_value, "end");
   }
}




void
SRGP_inquireColorTable 
   (int startentry, int count,
    unsigned short *redi, 
    unsigned short *greeni,
    unsigned short *bluei)
{
   static XColor *x_color_structs = NULL;
   static int cursize_of_x_cs_array = 0;  /* number of XColor structs */
   register int i,j;
   int endi;
   register XColor *xcurcs;


   /* LEAVE IMMEDIATELY IF EXECUTING ON BILEVEL DISPLAY */
   if (srgp__available_depth == 1 || srgp__visual_class == StaticGray)
      return;

   endi = startentry + count;

   DEBUG_AIDS{
      /* PERFORM CHECKING LEGALITY OF THE RANGE OF INDICES. */
      srgp_check_pixel_value (startentry, "start");
      srgp_check_pixel_value (endi-1, "end");
   }


   /* !!!!!! LATER, THIS SHOULD USE SAME ARRAY AS IN color_X11.c */

   /* DYNAMICALLY (RE)ALLOCATE ARRAY OF XColor STRUCTURES */
   if (cursize_of_x_cs_array < count) {
      if (x_color_structs)
	 free ((char*)x_color_structs);
      x_color_structs = (XColor*) malloc (sizeof(XColor)*count);
      cursize_of_x_cs_array = count;
   }

   for (i=startentry, xcurcs=x_color_structs; i<endi; i++,xcurcs++) {
      xcurcs->pixel = COLORINDEX(i);
      xcurcs->flags = -1;
   }

   XQueryColors (srgpx__display, srgpx__colormap, x_color_structs, count);


   /* COPY INTENSITY VALUES INTO USER'S ARRAY. */
   for (j=0, xcurcs=x_color_structs; j<count; j++,xcurcs++){
      redi[j] = xcurcs->red;
      greeni[j] = xcurcs->green;
      bluei[j] = xcurcs->blue;
   }
}




void
SRGP_loadCommonColor (entry, name)
int entry;
char *name;   /* Null-terminated string of characters */
{
   /* IGNORE IF MONOCHROME */
   if (srgp__application_depth == 1)
      return;
   
   //check if the max color limit has reached
   if(srgp__total_loaded_colors == MAX_COLORTABLE_SIZE) {
      fprintf(stderr, "The color table if full, adding nothing!\n", 
      "You can only add %d colors at a time!\n", MAX_COLORTABLE_SIZE);
      return;
   }

   DEBUG_AIDS{
      SRGP_trace (SRGP_logStream, "SRGP_loadCommonColor  %d  %s\n", entry, name);
      srgp_check_pixel_value (entry, "start/end");
   }

   XColor new, new_e;
   Status res = XAllocNamedColor(srgpx__display, srgpx__colormap, name, &new, &new_e);

   if(!res) {
      fprintf(stderr, "Counldn't add color: %s, adding nothing!\n", name);
      return;
   }

   //Initialize the Color Table with the 2 values of Black and White
   strcpy(srgp__colorLookup_table[srgp__total_loaded_colors].name, name);
   srgp__colorLookup_table[srgp__total_loaded_colors].pixel_value = new_e.pixel;

   srgp__total_loaded_colors++;
}