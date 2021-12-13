#include "srgp_private.h"
#include <sys/types.h>
#include <sys/time.h>
#include <memory.h>
#include <string.h>
#include <ctype.h>

/** DEVICE INDEPENDENCE
This file contains functions that are device-independent,
   except for the "sleeping" arrangements for SRGP_waitEvent.
Refer to inputraw.c and echo.c for the low-level "drivers" 
   for each type of implementation.

**/


static void
ComputeTimestamp (srgp_timestamp *ts)
{
   Time tdiff =  srgpx__cur_time - srgpx__starttime;

   ts->seconds = tdiff / rawgranularity;
   ts->ticks = (tdiff % rawgranularity) / ((double)rawgranularity) * 60;
}



/** SRGP __ initInputModule **/
void
SRGP__initInputModule ()
{
   /* DEFAULT KEYBOARD ECHO POSITION IS MIDDLE OF SCREEN WINDOW. */
   srgp__cur_keyboard_echo_origin = 
      SRGP_defPoint(srgp__canvasTable[0].max_xcoord >> 1,
		    srgp__canvasTable[0].max_ycoord >> 1);
   srgp__cur_locator_echo_anchor = srgp__cur_keyboard_echo_origin;

   /* DEFAULT KEYBOARD ATTRIBUTES */
   srgp__cur_keyboard_echo_font = 0;
   srgp__cur_keyboard_echo_color = SRGP_BLACK;

   SRGP__disableLocatorCursorEcho();

   /* INITIALIZE CURSOR TABLE. */
   PUSH_TRACE;
   /* SRGP_loadCursor (0, XC_arrow);    "device" dependent!!! */
   POP_TRACE;
   srgp__cur_cursor = 0;

   /* INITIALIZE ACTIVITY INFORMATION. */
   srgp__cur_mode[LOCATOR]=srgp__cur_mode[KEYBOARD]=INACTIVE;
   srgp__cur_locator_echo_type = CURSOR;
   srgp__cur_keyboard_processing_mode = EDIT;

   /* INITIALIZE MEASURES AND MASKS */
   srgp__cur_locator_measure.button_chord[LEFT_BUTTON] = UP;
   srgp__cur_locator_measure.button_chord[MIDDLE_BUTTON] = UP;
   srgp__cur_locator_measure.button_chord[RIGHT_BUTTON] = UP;
   srgp__cur_locator_measure.position = srgp__cur_locator_echo_anchor;

   ALLOC_RECORDS (srgp__cur_keyboard_measure.buffer, char, MAX_STRING_SIZE+1);
   srgp__cur_keyboard_measure.buffer_length = MAX_STRING_SIZE+1;
   srgp__cur_keyboard_measure.buffer[0] = '\0';
   ALLOC_RECORDS (srgp__get_keyboard_measure.buffer, char, MAX_STRING_SIZE+1);
   srgp__get_keyboard_measure.buffer_length = MAX_STRING_SIZE+1;
   srgp__get_keyboard_measure.buffer[0] = '\0';

   srgp__cur_locator_button_mask = LEFT_BUTTON_MASK;

   SRGP__initInputDrivers ();
   SRGP__initEchoModule ();
}
   



/** SRGP set input mode
The device is first deactivated, then its cur_mode status is updated,
then it is activated
**/
void
SRGP_setInputMode (inputDevice device, inputMode mode)
{
   DEBUG_AIDS{
      SRGP_trace (SRGP_logStream, "SRGP_setInputMode  %d into %d\n", 
		  device, mode);
      srgp_check_system_state();
      srgp_check_device (device);
      srgp_check_mode (mode);
      LeaveIfNonFatalErr();
   }

   if (mode == srgp__cur_mode[device])
      /* NOTHING IS TO BE DONE. */
      return;

   if (mode == INACTIVE) {
      /* WE ARE BRINGING AN ACTIVE DEVICE TO INACTIVITY. */
      SRGP__deactivateDevice (device);
      srgp__cur_mode[device] = INACTIVE;
   }

   else {
      if ((device==LOCATOR)&&(srgp__cur_mode[LOCATOR]==INACTIVE))
	 SRGP__updateRawCursorPosition();
      srgp__cur_mode[device] = mode;
      SRGP__activateDevice (device);
   }
}






/** SRGP wait event
The event in the queue which satisfies the wait is removed
and placed at the tail of the free-list.  It is there that
the "get" routines look.

Returns the identifier of the device which issued the event.
   (NO_DEVICE if none).
**/

inputDevice
SRGP_waitEvent (maximum_wait_time)
int maximum_wait_time;
{
   struct timeval tp, tzp, expiretime, timeout, *timeout_ptr;
   int fd;
   fd_set readfds;

   int return_value;
   boolean do_continue_wait, do_continue_search;
   boolean forever = (maximum_wait_time < 0);


   DEBUG_AIDS{
      srgp_check_system_state();
      LeaveIfNonFatalErr();
   }


#define MILLION 1000000
   FD_ZERO (&readfds);
   FD_SET (fd=ConnectionNumber(srgpx__display), &readfds);

   /* INITIALIZE (if necessary) TIMING INFORMATION */
   if (maximum_wait_time > 0) {
      long maxwait_sec, maxwait_ticks;
      gettimeofday (&tp,&tzp);
      maxwait_sec = maximum_wait_time / 60;
      maxwait_ticks = maximum_wait_time % 60;
      expiretime.tv_usec = tp.tv_usec + (maxwait_ticks*MILLION/60);
      expiretime.tv_sec = tp.tv_sec + maxwait_sec + 
	                  (expiretime.tv_usec / MILLION);
      expiretime.tv_usec %= MILLION;
   }

   do_continue_wait = TRUE;

   return_value = NO_DEVICE;  /* timeout is what we assume will happen */


   /**** LOOP *****/
   do {
      return_value = SRGP__handleRawEvents (TRUE,forever);

      if ((return_value != NO_DEVICE) || (maximum_wait_time == 0))
	 do_continue_wait = FALSE;

      /* Otherwise, perform a wait state */
      else {
	 if (!forever) {
	    gettimeofday (&tp,&tzp);
	    /* Perform subtraction: expiretime - tp */
	    if (expiretime.tv_usec < tp.tv_usec) {
	       /* perform a borrow */
	       expiretime.tv_sec--;
	       expiretime.tv_usec += 1000000;
	    }
	    timeout_ptr = &timeout;
	    timeout.tv_usec = expiretime.tv_usec - tp.tv_usec;
	    timeout.tv_sec  = expiretime.tv_sec  - tp.tv_sec;
	    if ((timeout.tv_sec < 0) || (timeout.tv_usec < 0))
	       /* Whoops!  We've already expired! */
	       do_continue_wait = FALSE;
	 }
	 else /* (maximum_wait_time is negative representing infinity) */
	    timeout_ptr = NULL;

	 if (do_continue_wait)
	    if (XPending(srgpx__display) == 0) {
	       if (select (fd+1, &readfds, NULL, NULL, timeout_ptr))
		  /* An event occurred before the timeout! */
		  ;  /* so do nothing */
	       else 
		  do_continue_wait = FALSE;
	    }
      }
   }
   while (do_continue_wait);

   srgp__device_at_head_of_queue = return_value;

   return return_value;
}



void
SRGP_getLocator (srgp__locator_measure *measure)
{
   DEBUG_AIDS{
      srgp_check_system_state();
      srgp_check_event_type (LOCATOR);
      LeaveIfNonFatalErr();
   }

   /* this assignment statement is very risky!  God help me! */
   *measure = *((srgp__locator_measure*)(&srgp__get_locator_measure));
}


void
SRGP_getDeluxeLocator (srgp__deluxe_locator_measure *measure)
{
   DEBUG_AIDS{
      srgp_check_system_state();
      srgp_check_event_type (LOCATOR);
      LeaveIfNonFatalErr();
   }

   *measure = srgp__get_locator_measure;
   ComputeTimestamp (&(measure->timestamp));
}



void
SRGP_getKeyboard (char *measure, int bufsize)
{
   DEBUG_AIDS{
      srgp_check_system_state();
      srgp_check_event_type (KEYBOARD);
      LeaveIfNonFatalErr();
   }
   strncpy (measure, srgp__get_keyboard_measure.buffer, bufsize-1);
   *(measure+bufsize-1) = '\0';
}


void
SRGP_getDeluxeKeyboard (srgp__deluxe_keyboard_measure *measure)
{
   DEBUG_AIDS{
      srgp_check_system_state();
      srgp_check_event_type (KEYBOARD);
      LeaveIfNonFatalErr();
   }
   strncpy (measure->buffer, srgp__get_keyboard_measure.buffer, 
	    measure->buffer_length-1);
   *(measure->buffer+measure->buffer_length-1) = '\0';
   bcopy (srgp__get_keyboard_measure.modifier_chord,
	  measure->modifier_chord,
	  sizeof(measure->modifier_chord));
   measure->position = srgp__get_keyboard_measure.position;
   
   ComputeTimestamp (&(measure->timestamp));
}




/** MEASURE SETTING **/

void
SRGP_setLocatorMeasure (srgp__point position)
{
   DEBUG_AIDS{
      SRGP_trace (SRGP_logStream, "SRGP_setLocatorMeasure %d,%d\n", 
		  position.x, position.y);
      srgp_check_system_state();
      LeaveIfNonFatalErr();
   }

   SRGP__handleRawEvents (FALSE,FALSE);

   srgp__cur_locator_measure.position = position;

   SRGP__updateRawCursorPosition ();
}


/*!*/
void
SRGP_setKeyboardMeasure (str)
char *str;
{
   DEBUG_AIDS{
      SRGP_trace (SRGP_logStream, "SRGP_setKeyboardMeasure %s\n", str);
      srgp_check_system_state();
      LeaveIfNonFatalErr();
   }
   SRGP__handleRawEvents (FALSE,FALSE);

   strncpy (srgp__cur_keyboard_measure.buffer, str, MAX_STRING_SIZE);
   *(srgp__cur_keyboard_measure.buffer+MAX_STRING_SIZE) = '\0';
   srgp__cur_keyboard_measure_length = strlen(srgp__cur_keyboard_measure.buffer);

   SRGP__updateKeyboardEcho ();
}




/** ATTRIBUTES
The routines allow the application to control the echoing
associated with the keyboard and the locator devices.
**/

/*!*/

void
SRGP_setLocatorEchoType (echoType value)
{
   DEBUG_AIDS{
      SRGP_trace (SRGP_logStream, "SRGP_setLocatorEchoType %d\n", value);
      srgp_check_system_state();
      srgp_check_locator_echo_type(value);
      LeaveIfNonFatalErr();
   }

   SRGP__handleRawEvents (FALSE,FALSE);

   SRGP__disableLocatorRubberEcho();
   SRGP__disableLocatorCursorEcho();

   srgp__cur_locator_echo_type = value;

   SRGP__enableLocatorRubberEcho();
   SRGP__enableLocatorCursorEcho();

   SRGP__updateInputSelectionMask();
}

   
/*!*/

void
SRGP_setLocatorEchoCursorShape (int id)
{
   DEBUG_AIDS{
      SRGP_trace (SRGP_logStream, "SRGP_setLocatorEchoCursorShape %d\n", id);
      srgp_check_system_state();
      LeaveIfNonFatalErr();
   }
   SRGP__handleRawEvents (FALSE,FALSE);

   srgp__cur_cursor = id;

   SRGP__updateLocatorCursorShape ();
}


/*!*/

void
SRGP_setLocatorEchoRubberAnchor (srgp__point position)
{
   DEBUG_AIDS{
      SRGP_trace (SRGP_logStream, "SRGP_setLocatorEchoRubberAnchor %d,%d\n",
		  position.x, position.y);
      srgp_check_system_state();
      LeaveIfNonFatalErr();
   }

   SRGP__handleRawEvents (FALSE,FALSE);

   srgp__cur_locator_echo_anchor = position;

   SRGP__updateLocatorRubberAnchor ();
}


/*!*/


void
SRGP_setLocatorButtonMask (int value)
{
   DEBUG_AIDS{
      SRGP_trace (SRGP_logStream, "SRGP_setLocatorButtonMask %d\n", value);
   }

   srgp__cur_locator_button_mask = value;
}


/*!*/


void
SRGP_setKeyboardProcessingMode (keyboardMode value)
{
   DEBUG_AIDS{
      SRGP_trace (SRGP_logStream, "SRGP_setKeyboardProcessingMode %d\n", 
		  value);
      srgp_check_system_state();
      LeaveIfNonFatalErr();
   }
   SRGP__handleRawEvents (FALSE,FALSE);

   if (srgp__cur_mode[KEYBOARD] != INACTIVE)
      SRGP__deactivateDevice (KEYBOARD);

   srgp__cur_keyboard_processing_mode = value;

   if (srgp__cur_mode[KEYBOARD] != INACTIVE)
      SRGP__activateDevice (KEYBOARD);
}


/*!*/


void
SRGP_setKeyboardEchoColor (int value)
{
   DEBUG_AIDS{
      SRGP_trace (SRGP_logStream, "SRGP_setKeyboardEchoColor %d\n", value);
      srgp_check_system_state();
      LeaveIfNonFatalErr();
   }
   SRGP__handleRawEvents (FALSE,FALSE);

   srgp__cur_keyboard_echo_color = value;
   SRGP__updateKeyboardEchoAttributes();
}


/*!*/


void
SRGP_setKeyboardEchoOrigin (srgp__point position)
{
   DEBUG_AIDS{
      SRGP_trace (SRGP_logStream, "SRGP_setKeyboardEchoOrigin %d,%d\n", 
		  position.x, position.y);
      srgp_check_system_state();
      LeaveIfNonFatalErr();
   }
   SRGP__handleRawEvents (FALSE,FALSE);

   srgp__cur_keyboard_echo_origin = position;
   SRGP__updateKeyboardEchoAttributes();
}


/*!*/


void
SRGP_setKeyboardEchoFont (int fontindex)
{
   DEBUG_AIDS{
      SRGP_trace (SRGP_logStream, "SRGP_setKeyboardEchoFont %d\n", fontindex);
      srgp_check_system_state();
      LeaveIfNonFatalErr();
   }
   SRGP__handleRawEvents (FALSE,FALSE);

   srgp__cur_keyboard_echo_font = fontindex;
   SRGP__updateKeyboardEchoAttributes ();
}




/** SAMPLERS **/

void
SRGP_sampleLocator (srgp__locator_measure *measure)
{
   DEBUG_AIDS{
      srgp_check_system_state();
      LeaveIfNonFatalErr();
   }
   SRGP__handleRawEvents (FALSE,FALSE);
   if (srgp__dirty_location) 
      SRGP__updateLocationKnowledge();
   *measure = *((srgp__locator_measure*)(&srgp__cur_locator_measure));
}


void
SRGP_sampleDeluxeLocator (srgp__deluxe_locator_measure *measure)
{
   DEBUG_AIDS{
      srgp_check_system_state();
      LeaveIfNonFatalErr();
   }
   SRGP__handleRawEvents (FALSE,FALSE);
   if (srgp__dirty_location) 
      SRGP__updateLocationKnowledge();
   *measure = srgp__cur_locator_measure;
   ComputeTimestamp (&(measure->timestamp));
}

/*!*/

void
SRGP_sampleKeyboard (char *measure, int bufsize)
{
   DEBUG_AIDS{
      srgp_check_system_state();
      LeaveIfNonFatalErr();
   }
   SRGP__handleRawEvents (FALSE,FALSE);
   strncpy (measure, srgp__cur_keyboard_measure.buffer, bufsize-1);
   *(measure+bufsize-1) = '\0';
}

void
SRGP_sampleDeluxeKeyboard (srgp__deluxe_keyboard_measure *measure)
{
   DEBUG_AIDS{
      srgp_check_system_state();
      LeaveIfNonFatalErr();
   }
   SRGP__handleRawEvents (FALSE,FALSE);
   strncpy (measure->buffer, srgp__cur_keyboard_measure.buffer, 
	    measure->buffer_length-1);
   *(measure->buffer+measure->buffer_length-1) = '\0';
   bcopy (srgp__cur_keyboard_measure.modifier_chord,
	  measure->modifier_chord,
	  sizeof(measure->modifier_chord));
   measure->position = srgp__get_keyboard_measure.position;
   ComputeTimestamp (&(measure->timestamp));
}


/* X related Input functions */
/** LOW-LEVEL "DRIVERS"

   SRGP__activateDevice (deviceID)
      Must be called when device is going from INACTIVE to active, OR
         when device is changed from one active mode to the other.

   SRGP__deactivateDevice (deviceID)
      Must be called only when device is going from active to inactive.


   SRGP__handleRawEvents (boolean in_waitEvent_call)
      This function nevers enters a wait state.
      It examines all the events on the "raw"
         queue: the queue of the underlying graphics package
	 (e.g., X11, Mac).
      Exception: it may not handle all the raw events.
         It exits as soon as it sees a valid trigger situation.
      It returns a device ID IF AND ONLY IF...
	 1) the appl. is in a call to SRGP_waitEvent(), AND
	 2) a valid trigger for a device currently in Event mode
	    has been encountered.
      IF it does return a device ID, THEN...
         It automatically sets the proper value for either
	     srgp__get_locator_measure or
	     srgp__get_keyboard_measure
	 in preparation for the application's ensuing call to 
	     SRGP_get...()
      Another exception: it may "pass over" some raw events and
         just leave them in the raw queue.
      It will pass over a raw event IF AND ONLY IF...
	 1) the appl. is not in a call to SRGP_waitEvent(), AND
	 2) the event is a valid trigger for a device
	    currently in Event mode.
      Another possibility is that it will discard a raw event
         without processing it at all.
      It will discard a raw event IF AND ONLY IF...
         The event is for a device that is currently inactive.
**/

#define BACKSPACE_KEY	 127
#define CARRIAGE_RETURN  13

#define KEYS		(KeyPressMask)
#define MOTION_HINT	(PointerMotionHintMask|PointerMotionMask)
#define MOTION_ALL	(PointerMotionMask)
#define BUTTONS (ButtonPressMask|ButtonReleaseMask|OwnerGrabButtonMask)
#define DEFAULT	(StructureNotifyMask|EnterWindowMask)

/** X SELECT INPUT MASKS
  I am always looking for keyboard presses (X events) because
  some keys are always active, allowing the user to press F6
  (for example) to turn on tracing at run-time, etc.
  
  When the locator is in sample mode w/o rubber echo, I need
      pointer-motion *hints* only.
  When the locator is using rubber echo, I need all pointer motion events.
  When the locator is in event mode and not echoed rubberly,
      I only need button-event reports.
**/

static unsigned long 
selectinputmask 
 [3] /*inputMode*/
 [2] /*boolean:rubberon?*/ = {
   /* input mode: INACTIVE */   {0L, 0L},
   /* input mode: SAMPLE */   	{MOTION_ALL|BUTTONS, MOTION_ALL|BUTTONS},
   /* input mode: EVENT */   	{BUTTONS, MOTION_ALL|BUTTONS}
};

#define SelectInput(LOCMODE,RUBBERON)   \
   XSelectInput (srgpx__display, srgpx__screenwin, \
		 selectinputmask[LOCMODE][RUBBERON?1:0]|KEYS|DEFAULT)


void
SRGP__initInputDrivers()
{
   /* INITIALIZE X INPUT */
   SelectInput (INACTIVE,FALSE);
}



void
SRGP__updateInputSelectionMask()
{
   SelectInput (srgp__cur_mode[LOCATOR], 
		srgp__cur_locator_echo_type > CURSOR);
}




/** RAW-LEVEL DEACTIVATION OF A DEVICE
Responsible for erasing echo, and resetting device's measure to the
   hardwired default.
Upon entry, the device's cur_mode is its old value (has not been
   changed yet)!  And this procedure does not change it!
**/

void
SRGP__deactivateDevice (int device)
{
   XEvent xev;

   switch (device) {
      
    case LOCATOR:
      SRGP__disableLocatorRubberEcho();
      SRGP__disableLocatorCursorEcho();
      srgp__cur_locator_measure.position = 
         SRGP_defPoint(srgp__canvasTable[0].max_xcoord>>1,
		       srgp__canvasTable[0].max_ycoord>>1);
      SelectInput(INACTIVE,FALSE);
      /* Delete all currently queued locator-related raw events. */
      while (XCheckMaskEvent(srgpx__display, MOTION_ALL|BUTTONS, &xev));
      break;
       
    case KEYBOARD:
      SRGP__disableKeyboardEcho();
      srgp__cur_keyboard_measure.buffer[0] = '\0';
      bzero (srgp__cur_keyboard_measure.modifier_chord,
	     sizeof(srgp__cur_keyboard_measure.modifier_chord));
      break;
   }
}





/** RAW-LEVEL ACTIVATION OF A DEVICE
Called whenever:
  a device is placed into EVENT or SAMPLE mode...
     a) when previously inactive
     b) when previously active but in a different mode
Responsible for initiating echo and setting X's selection mask.
Upon entry, the device's echo info and mode has already been set
  to their new values.
**/

void
SRGP__activateDevice (int device)
{
   switch (device) {
    case LOCATOR:
      SRGP__disableLocatorCursorEcho();
      SRGP__disableLocatorRubberEcho();
      SRGP__enableLocatorCursorEcho();
      SRGP__enableLocatorRubberEcho();
      SRGP__updateInputSelectionMask();
      break;
       
    case KEYBOARD:
      SRGP__enableKeyboardEcho();
      break;
   }
}





void
SRGP__updateRawCursorPosition ()
{
   srgp__cur_Xcursor_x = srgp__cur_locator_measure.position.x;
   srgp__cur_Xcursor_y = 
      SCREENFIXED(srgp__cur_locator_measure.position.y);
   XWarpPointer
      (srgpx__display,
       None, srgpx__screenwin,
       0,0,0,0,
       srgp__cur_Xcursor_x, srgp__cur_Xcursor_y);
   SRGP__updateLocatorRubberEcho();
}




static   XEvent xevent;
static   int xstrcount;
static   char buffer[50];
static   KeySym keysym;
static   unsigned long mask;
static   boolean in_wait_event;



void
SRGP__updateLocationKnowledge ()
{
   Window rw, cw;
   int xr, yr;
   unsigned int keys_buttons;

   XQueryPointer (srgpx__display, srgpx__screenwin,
		  &rw, &cw, &xr, &yr,
		  &srgp__cur_Xcursor_x, &srgp__cur_Xcursor_y, 
		  &keys_buttons);
   srgp__cur_locator_measure.position.x = srgp__cur_Xcursor_x;
   srgp__cur_locator_measure.position.y = SCREENFIXED(srgp__cur_Xcursor_y);
   
   srgp__dirty_location = FALSE;
}




static inputDevice HandleXButtonEvent (buttonStatus TRANSITION_TYPE) 
{
   int which_button;
   boolean do_return_event_notice = FALSE;


   which_button = xevent.xbutton.button - 1;

   if (srgp__cur_mode[LOCATOR] != EVENT)
      goto change_cur_measure;

   if (((srgp__cur_locator_button_mask >> which_button) & 1) == 0)
      goto change_cur_measure;

   if ( ! in_wait_event) {
      XPutBackEvent (srgpx__display, &xevent);
      mask &=  ~(BUTTONS);
      return NO_DEVICE;
   }

   do_return_event_notice = TRUE;

 change_cur_measure:
   srgpx__cur_time = xevent.xbutton.time;
   srgp__cur_locator_measure.button_chord[which_button] = TRANSITION_TYPE;
   srgp__cur_locator_measure.button_of_last_transition = which_button;
   srgp__cur_locator_measure.modifier_chord[SHIFT] =
      (xevent.xbutton.state&ShiftMask?TRUE:FALSE); 
   srgp__cur_locator_measure.modifier_chord[CONTROL] =  
      (xevent.xbutton.state&ControlMask?TRUE:FALSE); 
   srgp__cur_locator_measure.modifier_chord[META] =  
      (xevent.xbutton.state&Mod1Mask?TRUE:FALSE); 
   srgp__cur_locator_measure.position.x = 
      xevent.xbutton.x; 
   srgp__cur_locator_measure.position.y = 
      srgp__canvasTable[0].max_ycoord - xevent.xbutton.y; 
   if (do_return_event_notice) {
      srgp__get_locator_measure = srgp__cur_locator_measure; 
      return LOCATOR; 
   }
   return NO_DEVICE;
}



static inputDevice HandleRawModeKeyEvent (void)
{
   boolean do_return_event_notice = FALSE;
   
   if (srgp__cur_mode[KEYBOARD] != EVENT)
      goto change_cur_measure;
   
   if ( ! in_wait_event) {
      XPutBackEvent (srgpx__display, &xevent);
      mask &=  ~(KEYS);
      return NO_DEVICE;
   }

   do_return_event_notice = TRUE;
   
 change_cur_measure:
   srgp__cur_keyboard_measure.buffer[0] = *buffer;
   srgp__cur_keyboard_measure.buffer[1] = '\0';
   srgp__cur_keyboard_measure.modifier_chord[SHIFT] =
      (xevent.xkey.state&ShiftMask?TRUE:FALSE);
   srgp__cur_keyboard_measure.modifier_chord[CONTROL] =
      (xevent.xkey.state&ControlMask?TRUE:FALSE);
   srgp__cur_keyboard_measure.modifier_chord[META] =
      (xevent.xkey.state&Mod1Mask?TRUE:FALSE);
   if (do_return_event_notice) {
      strcpy (srgp__get_keyboard_measure.buffer, 
	      srgp__cur_keyboard_measure.buffer);
      bcopy (srgp__cur_keyboard_measure.modifier_chord,
	     srgp__get_keyboard_measure.modifier_chord,
	     sizeof(srgp__get_keyboard_measure.modifier_chord));
      srgp__get_keyboard_measure.position =
	 srgp__cur_keyboard_measure.position;
      return KEYBOARD;
   }
   return NO_DEVICE;
}


static inputDevice HandleProcModeKeyEvent (void)
{
   boolean do_return_event_notice = FALSE;
   
   switch (buffer[0]) {
    case CARRIAGE_RETURN:
      if (srgp__cur_mode[KEYBOARD] != EVENT)
	 goto erase_cur_measure;
      if (in_wait_event)
	 do_return_event_notice = TRUE;
      else {
	 XPutBackEvent (srgpx__display, &xevent);
	 mask &=  ~KEYS;
	 return NO_DEVICE;
      }
      if (do_return_event_notice) {
	 strcpy (srgp__get_keyboard_measure.buffer, 
		 srgp__cur_keyboard_measure.buffer);
	 srgp__get_keyboard_measure.position =
	    srgp__cur_keyboard_measure.position;
      }
    erase_cur_measure:
      srgp__cur_keyboard_measure.buffer[0] = '\0';
      srgp__cur_keyboard_measure_length = 0;
      SRGP__updateKeyboardEcho();
      if (do_return_event_notice)
	 return KEYBOARD;
      break;
      
    case BACKSPACE_KEY:
      if (srgp__cur_keyboard_measure_length > 0) {
	 srgp__cur_keyboard_measure_length =
	    srgp__cur_keyboard_measure_length - 1;
	 srgp__cur_keyboard_measure.buffer
	    [srgp__cur_keyboard_measure_length] =
	       '\0';
	 SRGP__updateKeyboardEcho();
      }
      break;
      
    default:
      /* CHECK: IS THE KEY PRINTABLE ASCII? */
      if ((isprint(*buffer)) &&
	  (srgp__cur_keyboard_measure_length < MAX_STRING_SIZE)) {
	 srgp__cur_keyboard_measure.buffer
	    [srgp__cur_keyboard_measure_length] = 
	       *buffer;
	 srgp__cur_keyboard_measure_length++;
	 srgp__cur_keyboard_measure.buffer
	    [srgp__cur_keyboard_measure_length] = 
	       '\0';
	 SRGP__updateKeyboardEcho();
      }
      break;
   }
   return NO_DEVICE;
}
   





/** SRGP__handleRawEvents
      This function nevers enters a wait state, unless it has been
         called as a result of SRGP_waitEvent(FOREVER).
      It examines all the events on the "raw"
         queue: the queue of the underlying graphics package
	 (e.g., X11, Mac).
      Exception: it may not handle all the raw events.
         It exits as soon as it sees a valid trigger situation.
      It returns a device ID IF AND ONLY IF...
	 1) the appl. is in a call to SRGP_waitEvent(), AND
	 2) a valid trigger for a device currently in Event mode
	    has been encountered.
      IF it does return a device ID, THEN...
         It automatically sets the proper value for either
	     srgp__get_locator_measure or
	     srgp__get_keyboard_measure
	 in preparation for the application's ensuing call to 
	     SRGP_get...()
      Another exception: it may "pass over" some raw events and
         just leave them in the raw queue.
      It will pass over a raw event IF AND ONLY IF...
	 1) the appl. is not in a call to SRGP_waitEvent(), AND
	 2) the event is a valid trigger for a device
	    currently in Event mode.
      Another possibility is that it will discard a raw event
         without processing it at all.
      It will discard a raw event IF AND ONLY IF...
         The event is for a device that is currently inactive.
**/

int
   SRGP__handleRawEvents (boolean inwaitevent, boolean forever)
{
   inputDevice id;
   
   in_wait_event = inwaitevent;
   mask = (KEYS|MOTION_ALL|DEFAULT|BUTTONS);
   
   while (1) {
      
      if (forever && inwaitevent) {
         XMaskEvent (srgpx__display, mask, &xevent);
      }
      else {
         if ( ! XCheckMaskEvent (srgpx__display, mask, &xevent))
	    return NO_DEVICE;
      }
      
      switch (xevent.type) {
	 
       case EnterNotify:
	 /* IF ON COLOR SYSTEM:
	    When cursor enters screen canvas, install SRGP colormap. */
	 if ((srgp__available_depth > 1) && 
	     (srgp__available_depth == srgp__application_depth))
	    XInstallColormap (srgpx__display, srgpx__colormap);
	 break;
	 
       case MotionNotify:
	 /* WE CAN ASSUME LOCATOR IS ACTIVE IF WE GET HERE. */
	 srgpx__cur_time = xevent.xmotion.time;
	 if (xevent.xmotion.is_hint)
	    /* WE CAN ASSUME RUBBER-ECHO IS OFF IF WE GET HERE. */
	    /* WE CAN ALSO ASSUME LOCATOR IS IN SAMPLE MODE IF WE GET HERE. */
	    srgp__dirty_location = TRUE;
	 else {
	    srgp__cur_Xcursor_x = xevent.xmotion.x;
	    srgp__cur_Xcursor_y = xevent.xmotion.y;
	    srgp__cur_locator_measure.position.x = srgp__cur_Xcursor_x;
	    srgp__cur_locator_measure.position.y = 
	       SCREENFIXED(srgp__cur_Xcursor_y);
	    SRGP__updateLocatorRubberEcho();
	 }
	 break;
	 
       case ButtonPress:
	 if (id = HandleXButtonEvent (DOWN))
	    return id;
	 break;
	 
       case ButtonRelease:
	 if (id = HandleXButtonEvent (UP))
	    return id;
	 break;
	 
       case MappingNotify:
	 XRefreshKeyboardMapping (&xevent.xmapping);
	 break;
	 
       case ConfigureNotify:
	 /* WE ONLY WISH TO REACT IF TRULY INVOLVED RESIZING (not just move) */
	 if (xevent.xconfigure.width == srgp__canvasTable[0].max_xcoord+1) 
	    if (xevent.xconfigure.height == srgp__canvasTable[0].max_ycoord+1) 
	       break;
	 
	 /* IF WE GET HERE, we wish to change the dimensions as recorded in
	    the canvas table, being careful if the screen canvas is currently
	    active. */
	 SRGP__reactToScreenResize
	    (xevent.xconfigure.width, xevent.xconfigure.height);
	 break;
	 
       case KeyPress:
	 xstrcount = XLookupString (&xevent.xkey, buffer, 50, &keysym, 0);
	 /* HERE, CHECK FOR keysym==FKEY for interesting FKEYS. */
	 if (srgp__cur_mode[KEYBOARD] == INACTIVE) 
	    break;
	 if (xstrcount != 1)
	    break;
	 
	 srgpx__cur_time = xevent.xkey.time;
	 srgp__cur_keyboard_measure.position.x = xevent.xkey.x;
	 srgp__cur_keyboard_measure.position.y = SCREENFIXED(xevent.xkey.y);
	 
	 if (srgp__cur_keyboard_processing_mode == RAW) {
	    if (id = HandleRawModeKeyEvent())
	       return id;
	 }
	 else {
	    if (id = HandleProcModeKeyEvent())
	       return id;
	 }
      }
   }
}
