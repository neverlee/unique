# set environment variable RM_INCLUDE_DIR to the location of redismodule.h
RM_INCLUDE_DIR=../

CFLAGS = -g -fPIC -O3 -I$(RM_INCLUDE_DIR) -Wall -Wno-unused-function ## -std=gnu99 
LDFLAGS = -g -lc -lm
CC=gcc

OBJS = dict.o siphash.o

all: libutil.a

clean:
	rm -rf *.o *.a

libutil.a: $(OBJS)
	ar rcs $@ $^

