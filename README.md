# SRGP
An SRGP rewrite.

# To Do
### state.c
- [ ] void SRGP_begin (char *name, int w, int h, int planes, boolean trace);
- [ ] void SRGP_beginWithDebug (char *name, int w, int h, int planes, boolean trace);
- [ ] void SRGP_disableDebugAids (void);
- [ ] void SRGP_enableBlockedWait (void);
- [ ] void SRGP_setErrorHandlingMode (errorHandlingMode);
- [ ] void SRGP_enableSynchronous (void);
- [ ] void SRGP_tracing (boolean);
- [ ] void SRGP_allowResize (boolean);
- [ ] void SRGP_registerResizeCallback (funcptr);
- [ ] void SRGP_changeScreenCanvasSize (int newwidth, int newheight);
- [ ] void SRGP_end (void);