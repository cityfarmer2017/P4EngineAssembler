
APP_NAME := P4eAsm
export APP_NAME

TOP_DIR := $(shell pwd)
export TOP_DIR

BIN_DIR := $(TOP_DIR)/bin
export BIN_DIR

BUILD_DIR := $(TOP_DIR)/build
OBJ_DIR := $(BUILD_DIR)/obj
DEP_DIR := $(BUILD_DIR)/dep
export OBJ_DIR DEP_DIR

SRC_DIR := $(TOP_DIR)/src
INC_DIR := $(TOP_DIR)/inc
export SRC_DIR INC_DIR

CC := g++
CCFLAGS := ${DBG_FLAG}
CCFLAGS += ${TBL_FLAG}
CCFLAGS += ${PRE_FLAG}
CCFLAGS += -g -Wall
export CC CCFLAGS

.PHONY : all clean dist-clean

all :
	@cd $(SRC_DIR); make || exit "$$?";

clean :
	@cd $(SRC_DIR); make clean || exit "$$?";

dist-clean :
	rm -rf $(BUILD_DIR) $(BIN_DIR)