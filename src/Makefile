# set environment variable RM_INCLUDE_DIR to the location of redismodule.h
RM_INCLUDE_DIR=../

UTIL_LIBDIR=../util/

# find the OS
uname_S := $(shell sh -c 'uname -s 2>/dev/null || echo not')

# Compile flags for linux / osx
ifeq ($(uname_S),Linux)
        SHOBJ_CFLAGS ?=  -fno-common -g -ggdb
        SHOBJ_LDFLAGS ?= -shared
else
        SHOBJ_CFLAGS ?= -dynamic -fno-common -g -ggdb
        SHOBJ_LDFLAGS ?= -bundle -undefined dynamic_lookup
endif

CFLAGS = -I$(RM_INCLUDE_DIR) -I$(UTIL_LIBDIR) -g -fPIC -O3 -Wall  # -std=gnu99 -Wall
LDFLAGS = -g -lc -lm
CC=gcc
.SUFFIXES: .c .so .o

SRCDIR := $(shell pwd)
REDISEX = unique

all: $(REDISEX)

$(REDISEX): %: $(SRCDIR)/%.o
	$(LD) -o $@.so $< $(SHOBJ_LDFLAGS) $(LIBS) -L$(UTIL_LIBDIR) -lutil -lc

clean:
	rm -rf *.so *.o
