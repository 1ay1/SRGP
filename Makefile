.POSIX:
.SUFFIXES:

AR      = ar
CC      = cc
CFLAGS  = -std=c99 -fPIC -Isrc/
CFLAGS += -Wall -Wextra -Wpedantic
CFLAGS += -g -Og -march=native -mtune=native
CFLAGS += -DX11
LDLIBS  = -lX11

sources =             	\
  src/geom.c          	\
  src/srgp_attrib.c   	\
  src/srgp_canvas.c   	\
  src/srgp_color.c  	\
  src/srgp_cursor.c 	\
  src/srgp_echo.c   	\
  src/srgp_error.c    	\
  src/srgp_font.c     	\
  src/srgp_input.c    	\
  src/srgp_inquire.c  	\
  src/srgp_marker.c   	\
  src/srgp_output.c   	\
  src/srgp_pattern.c  	\
  src/srgp_raster.c   	\
  src/srgp_state.c    	\

objects = $(sources:.c=.o)

tests =		              	\
	show_patterns         	\
	testcolor_and_resize	\
	testeditkeyboard    	\
	test_keyboard         	\
	test_locator          	\
	testmodifiers         	\
	live_chart		\
	testpaint             	\
	testpixmap            	\
	testrubber            	\
	testtimestamp         	\
	X_demo_anim


all: libsrgp.a libsrgp.so $(tests)
libsrgp.a: $(objects)
	$(AR) rcs $@ $(objects)
libsrgp.so: $(objects)
	$(CC) -shared -o $@ $(objects) $(LDLIBS)

src/geom.o: src/geom.c
src/srgp_attrib.o: src/srgp_attrib.c
src/srgp_canvas.o: src/srgp_canvas.c
src/srgp_color.o: src/srgp_color.c
src/srgp_cursor.o: src/srgp_cursor.c
src/srgp_echo.o: src/srgp_echo.c
src/srgp_error.o: src/srgp_error.c
src/srgp_font.o: src/srgp_font.c
src/srgp_input.o: src/srgp_input.c
src/srgp_inquire.o: src/srgp_inquire.c
src/srgp_marker.o: src/srgp_marker.c
src/srgp_output.o: src/srgp_output.c
src/srgp_pattern.o: src/srgp_pattern.c
src/srgp_raster.o: src/srgp_raster.c
src/srgp_state.o: src/srgp_state.c

show_patterns: tests/show_patterns.c libsrgp.a
	$(CC) $(CFLAGS) -o $@ $< libsrgp.a $(LDLIBS)

testcolor_and_resize:  tests/testcolor_and_resize.c libsrgp.a
	$(CC) $(CFLAGS) -o $@ $< libsrgp.a $(LDLIBS)

live_chart:  tests/live_chart.c libsrgp.a
	$(CC) $(CFLAGS) -o $@ $< libsrgp.a $(LDLIBS)

testeditkeyboard:  tests/testeditkeyboard.c libsrgp.a
	$(CC) $(CFLAGS) -o $@ $< libsrgp.a $(LDLIBS)

test_keyboard:  tests/test_keyboard.c libsrgp.a
	$(CC) $(CFLAGS) -o $@ $< libsrgp.a $(LDLIBS)

test_locator:  tests/test_locator.c libsrgp.a
	$(CC) $(CFLAGS) -o $@ $< libsrgp.a $(LDLIBS)

testmodifiers:  tests/testmodifiers.c  libsrgp.a
	$(CC) $(CFLAGS) -o $@ $< libsrgp.a $(LDLIBS)

testpaint:  tests/testpaint.c libsrgp.a
	$(CC) $(CFLAGS) -o $@ $< libsrgp.a $(LDLIBS)

testpixmap:  tests/testpixmap.c libsrgp.a
	$(CC) $(CFLAGS) -o $@ $< libsrgp.a $(LDLIBS)

testrubber:  tests/testrubber.c libsrgp.a
	$(CC) $(CFLAGS) -o $@ $< libsrgp.a $(LDLIBS)

testtimestamp:  tests/testtimestamp.c libsrgp.a
	$(CC) $(CFLAGS) -o $@ $< libsrgp.a $(LDLIBS)

X_demo_anim:  tests/X_demo_anim.c libsrgp.a
	$(CC) $(CFLAGS) -o $@ $< libsrgp.a $(LDLIBS)

.SUFFIXES: .c .o
.c.o:
	$(CC) $(CFLAGS) -c -o $@ $<


.PHONY: clean
clean:
	rm -f libsrgp.a libsrgp.so $(objects) $(tests)
