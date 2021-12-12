#include "srgp_private.h"

static
char *(srgp_errmsgs[]) =
  {
   "UNUSED",
   "The canvas table is full: your attempt to create a canvas is ignored.\n", //1
   "You may not delete the screen canvas -- its life is too valuable!\n",  //2
   "You may not delete the canvas which is currently active.\n", //3
   "SRGP is already enabled!  You can't reinitialize or change table sizes.\n", //4
   "SRGP is not enabled yet!  Have you heard about SRGP_begin() ?\n", //5
   "You sent a bad rectangle (%d,%d,%d,%d) (either your l>r, or your b>t).\n", //6
   "You sent a bad font index (%d) (either negative or too large).\n", //7
   "You sent a bad font index (%d) (that entry of font-table is undefined).\n", //8
   "You sent a bad count (%d) (it was negative).\n", //9
   "You sent a bad line-style spec (%d) (please use our named constants).\n", //10
   "You sent a bad marker-style spec (%d) (please use our named constants).\n", //11
   "You sent a bad write-mode spec (%d) (please use our named constants).\n", //12
   "SRGP_setColorTable: The %s of the range of pixel values you specified (%d)\n\tlies outside this machine's LUT.\n", //13
   "You sent a bad fill-style spec (%d) (please use our named constants).\n", //14
   "You sent a bad pattern index (%d).\n", //15
   "You sent a bad cursor index (%d).\n", //16
   "No known canvas possesses the index you sent.\n", //17
   "You sent a bad input device spec (%d).\n", //18
   "You sent a bad input mode spec (%d).\n", //19
   "You did a 'get' on a device that was NOT responsible\n\tfor the last wait_event exit.\n", //20
   "Whoops!  Event queue is full!  We tossed an event into oblivion...\n", //21
   "You sent a bad locator-echo type (%d) (please use our named constants).\n", //22
   "You sent a bad keyboard-processing mode (%d) (please use our named constants).\n", //23
   "You sent a list of %d points (either too many or too few).\n", //24
   "You sent a bad font filename (%s).\n", //25
   "I ran out of dynamic memory while allocating tables or creating a new canvas.\n", //26
   "NOT IMPLEMENTED YET!!!!\n", //27
   "X SERVER ERROR\n  (examining the core will be useful only if using X in synchronous mode):\n\t %s\n", //28
   "You sent a bad line-width spec (%d).\n", //29
   "You tried to get me to use an EMPTY entry in the pattern table.\n", //30
   "You sent a negative or zero marker size (%d).\n", //31
   "There is no more memory available for pix/bitmap pattern entries!\n" //32

   "UNUSED"
};

static char logmessage[] =
"%s FATAL ERROR:\n\
   %s\
\n\nI AM ABOUT TO INTENTIONALLY CRASH SO YOU CAN LOOK\n\
      AT THE ACTIVATION STACK USING A DEBUGGER.\n\n\
If your application is an SRGP application, please remember:\n\
   If the error message above says\n\
   that you sent a bad argument to a certain function,\n\
   you should run your program with tracing ON in order\n\
   to see exactly what you sent.\n\
If you already had tracing enabled, remember to look in\n\
   'SRGPlogfile' for the tracing messages.\n\n";
   
char **srgp__identifierOfMessages = srgp_errmsgs;

static int zero = 0;

char *srgp__identifierOfPackage = "SRGP";

void
SRGP__error (int errtype, ...)
{
   int item;
   char processedmessage[400];
   char *rawmessage;

   /* SIMPLE HANDLING??? */   
   if (srgp__curErrHndlMode == NON_FATAL_ERRORS) {
      SRGP_errorOccurred = errtype;
      return;
   }

   va_list args;
   va_start(args, errtype);

   /* DETERMINE THE MESSAGE CORRESPONDING TO THE ERROR TYPE */
   rawmessage = srgp__identifierOfMessages[errtype];

   sprintf (processedmessage, rawmessage, args);

   fprintf (stderr, logmessage, srgp__identifierOfPackage, processedmessage);

   va_end(args);
   abort ();
}
