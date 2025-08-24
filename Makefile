# Makefile for data project (build artifacts in build/)

# Detect OS for platform-specific settings
UNAME_S := $(shell uname -s)
RPATH_GSDK_REL = ../gsdk/build/lib
ifeq ($(UNAME_S),Darwin)
	SHARED_EXT = dylib
	RPATH_FLAGS = -Wl,-rpath,@loader_path -Wl,-rpath,@loader_path/$(RPATH_GSDK_REL)
else
	SHARED_EXT = so
	RPATH_FLAGS = -Wl,-rpath,'$$ORIGIN' -Wl,-rpath,'$$ORIGIN/$(RPATH_GSDK_REL)'
endif

# Compiler and flags
CC = clang
CFLAGS = -Wall -Wextra -Iinclude -Igsdk/include -Igsdk/include/core -Igsdk/include/data -Igsdk/include/performance -Igsdk/include/reflection -std=c23 -g

# Directories
BUILD_DIR = build
GSDK_LIB_DIR = gsdk/build/lib
KEY_VALUE_DB_SRC_DIR = src

# Sources / objects
KEY_VALUE_DB_SRC = $(wildcard $(KEY_VALUE_DB_SRC_DIR)/*.c)
KEY_VALUE_DB_OBJ = $(patsubst $(KEY_VALUE_DB_SRC_DIR)/%.c,$(BUILD_DIR)/%.o,$(KEY_VALUE_DB_SRC))

# Library / executables (in build/)
KEY_VALUE_DB_LIB_BASENAME = key_value_db
KEY_VALUE_DB_LIB = $(BUILD_DIR)/lib$(KEY_VALUE_DB_LIB_BASENAME).$(SHARED_EXT)
SERVER = $(BUILD_DIR)/key_value_db_server
CLIENT = $(BUILD_DIR)/key_value_db_client

# Locate gsdk shared libraries (full paths)
GSDK_LIBS = $(wildcard $(GSDK_LIB_DIR)/*.$(SHARED_EXT))

# Default target
all: $(KEY_VALUE_DB_LIB) $(SERVER) $(CLIENT)

# Ensure build directory exists
$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)

# Object files
$(BUILD_DIR)/%.o: $(KEY_VALUE_DB_SRC_DIR)/%.c | $(BUILD_DIR)
	$(CC) $(CFLAGS) -fPIC -c -o $@ $<

# Shared library
$(KEY_VALUE_DB_LIB): $(KEY_VALUE_DB_OBJ)
	$(CC) -shared -o $@ $^ $(GSDK_LIBS)

# Executables
$(SERVER): key_value_db_server.c $(KEY_VALUE_DB_LIB)
	$(CC) $(CFLAGS) -o $@ $< $(KEY_VALUE_DB_LIB) $(GSDK_LIBS) $(RPATH_FLAGS)

$(CLIENT): key_value_db_client.c $(KEY_VALUE_DB_LIB)
	$(CC) $(CFLAGS) -o $@ $< $(KEY_VALUE_DB_LIB) $(GSDK_LIBS) $(RPATH_FLAGS)

# Info
info:
	@echo "key_value_db sources : $(KEY_VALUE_DB_SRC)"
	@echo "key_value_db objects : $(KEY_VALUE_DB_OBJ)"
	@echo "key_value_db library : $(KEY_VALUE_DB_LIB)"
	@echo "gsdk libraries : $(GSDK_LIBS)"
	@echo "server executable : $(SERVER)"
	@echo "client exec    : $(CLIENT)"

# Clean
clean:
	rm -rf $(BUILD_DIR)

.PHONY: all clean info
