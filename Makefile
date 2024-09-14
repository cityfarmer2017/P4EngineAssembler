APP_NAME := P4eAsm
export APP_NAME

TOP_DIR := $(shell pwd)
BIN_DIR := $(TOP_DIR)/bin
export BIN_DIR

BUILD_DIR := $(TOP_DIR)/build
OBJ_DIR := $(BUILD_DIR)/obj
DEP_DIR := $(BUILD_DIR)/dep
export OBJ_DIR DEP_DIR

TEMP_DIRS := $(BIN_DIR) $(BUILD_DIR) $(OBJ_DIR) $(DEP_DIR)

SRC_DIR := $(TOP_DIR)/src
INC_DIR := $(TOP_DIR)/inc
export SRC_DIR INC_DIR

CC := g++
CCFLAGS := ${DBG_FLAG}
CCFLAGS += ${SUB_FLAG}
CCFLAGS += -g -Wall# -DDEBUG
export CC CCFLAGS

SRCFILES := $(wildcard $(SRC_DIR)/*.cpp)
INCLUDES := $(INC_DIR)
OBJECTS  := $(SRCFILES:$(SRC_DIR)/%.cpp=$(OBJ_DIR)/%.o)
DEPENDS  := $(OBJECTS:$(OBJ_DIR)/%=$(DEP_DIR)/%.d)
export OBJECTS

.PHONY : all
all : $(TEMP_DIRS) $(BIN_DIR)/$(APP_NAME)

$(TEMP_DIRS) :
	mkdir -p $@

SUB1_MODULE := table_proc
# SUB2_MODULE := XXX
SUB_MODULES += $(SUB1_MODULE)
# SUB_MODULES += $(SUB2_MODULE)
SUB1_OBJECTS := $(wildcard $(OBJ_DIR)/$(SUB1_MODULE)/*.o)
# SUB2_OBJECTS := $(wildcard $(OBJ_DIR)/$(SUB2_MODULE)/*.o)

ifeq (,$(SUB_FLAG))

$(BIN_DIR)/$(APP_NAME) : $(OBJECTS) $(SUB1_OBJECTS)
	@for m in $(SUB_MODULES); do cd $(SRC_DIR)/$$m; make SUB_MODULE=$$m || exit "$$?"; done

else  # ifneq (,$(SUB_FLAG))

$(BIN_DIR)/$(APP_NAME) : $(OBJECTS)
	@echo "\e[32m""Linking executable: $(BIN_DIR)/$(APP_NAME) without sub modules included.""\e[0m"
	$(CC) $(CCFLAGS) -o $@ $(OBJECTS)

endif  # ifneq (,$(SUB_FLAG))

$(OBJ_DIR)/%.o : $(SRC_DIR)/%.cpp
	$(CC) $(CCFLAGS) -I$(INCLUDES) -o $@ -c $< -MMD -MF $(patsubst $(OBJ_DIR)/%, $(DEP_DIR)/%.d, $@)

-include $(DEPENDS)

.PHONY : clean
clean :
	@for m in $(SUB_MODULES); do cd $(SRC_DIR)/$$m; make clean SUB_MODULE=$$m || exit "$$?"; done
	rm -rf $(BIN_DIR)/$(APP_NAME) $(OBJECTS) $(DEPENDS)

.PHONY : dist-clean
dist-clean :
	rm -rf $(BUILD_DIR) $(BIN_DIR)