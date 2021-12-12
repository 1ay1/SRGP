# Request POSIX Make behavior and disable any default suffix-based rules
.POSIX:
.SUFFIXES:


# Declare compiler tools and flags
AR      = ar
CC      = cc
CFLAGS  = -std=c99 -D_POSIX_C_SOURCE=200809L
CFLAGS += -fPIC -g -Og
CFLAGS += -Wall -Wextra -Wpedantic
CFLAGS += -Wno-unused-parameter
CFLAGS += -Isrc/
LDFLAGS =
LDLIBS  = -lX11


# Declare which targets should be built by default
default: libsrgp.a
all: libsrgp.a libsrgp.so


# Declare static / shared library sources
libsrgp_sources = $(wildcard src/*.c)
libsrgp_headers = $(wildcard src/*.h)
libsrgp_objects = $(libsrgp_sources:.c=.o)

tests = $(wildcard tests/*.c)
tests_bins = $(tests:.c=)

# Express dependencies between object and source files
$(libsrgp_objects): $(libsrgp_sources) $(libsrgp_headers)
	$(CC) $(CFLAGS) -c $< -o $@

# Build the static library
libsrgp.a: $(libsrgp_objects)
	@echo "BUILDING STATIC  $@"
	@$(AR) rcs $@ $(libsrgp_objects)

# Build the shared library
libsrgp.so: $(libsrgp_objects)
	@echo "BUILDING SHARED  $@"
	@$(CC) $(LDFLAGS) -shared -o $@ $(libsrgp_objects) $(LDLIBS)


# Build the tests binary

# Helper target that cleans up build artifacts
.PHONY: clean tests clean_tests
clean:
	rm -fr *.a *.so src/*.o

clean_tests:
	cd tests && ls | grep -v "\." | xargs rm

tests: $(tests_bins)

$(tests_bins): $(tests)
	$(CC) $(CFLAGS) $< libsrgp.a -o $@

# Default rule for compiling .c files to .o object files
.SUFFIXES: .c .o
.c.o:
	@echo "CC      $@"
	@$(CC) $(CFLAGS) -c -o $@ $<
