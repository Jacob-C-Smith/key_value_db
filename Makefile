# Compiler settings
CC = clang
CFLAGS = -Wall -Wextra -O2 -g -I$(PWD)/gsdk/include

# Get absolute path for GSDK
GSDK_PATH := $(PWD)/gsdk
GSDK_LIB_PATH := $(GSDK_PATH)/build/lib

# Set rpath to find libraries at runtime
RPATH_FLAGS = -Wl,-rpath,$(GSDK_LIB_PATH)

# All GSDK libraries
GSDK_LIBS = log sync hash socket \
            array circular_buffer dict tree tuple priority_queue queue stack \
            json base64 parallel

# Use full path to each .dylib file
LIBS = $(addprefix $(GSDK_LIB_PATH)/, $(addsuffix .so, $(GSDK_LIBS)))

# LDFLAGS: add pthread and all .dylib files
LDFLAGS = $(LIBS) -lpthread

# Source files
SRCS = $(wildcard *.c)
OBJS = $(SRCS:.c=.o)

.PHONY: clean gsdk

key_value_db: gsdk $(OBJS)
	$(CC) $(OBJS) $(RPATH_FLAGS) $(LDFLAGS) -o $@

gsdk:
	$(MAKE) -C gsdk

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f key_value_db $(OBJS)
	$(MAKE) -C gsdk clean